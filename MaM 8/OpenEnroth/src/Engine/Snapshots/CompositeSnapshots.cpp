#include "CompositeSnapshots.h"

#include <cstring>
#include <string>
#include <algorithm>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Mm6Ids.h"
#include "Engine/Objects/Mm8Ids.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Engine/Tables/TileTable.h"


#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Lod/LodWriter.h"
#include "Library/Lod/LodReader.h"
#include "Library/Lod/LodEnums.h"
#include "Library/Image/Pcx.h"

#include "Utility/GameVariant.h"
#include "Utility/Exception.h"
#include "Utility/Memory/MemSet.h"
#include "Utility/Streams/BlobOutputStream.h"

#include "Engine/Graphics/Image.h"

template<class Face>
static void dropDuplicateFaceVertices(Face *face) {
    auto copyVertex = [&](int src, int dst) {
        face->vertexIds[dst] = face->vertexIds[src];
        face->textureUs[dst] = face->textureUs[src];
        face->textureVs[dst] = face->textureVs[src];
    };

    // First pass - collapse everything that doesn't wrap around.
    int writeIdx = 0;
    for (int readIdx = 0; readIdx < face->numVertices; readIdx++) {
        if (writeIdx > 0 && face->vertexIds[readIdx] == face->vertexIds[writeIdx - 1])
            continue; // AA -> A.
        if (writeIdx > 1 && face->vertexIds[readIdx] == face->vertexIds[writeIdx - 2]) {
            writeIdx--;
            continue; // ABA -> A.
        }
        if (readIdx != writeIdx)
            copyVertex(readIdx, writeIdx);
        writeIdx++;
    }

    // Second pass - collapse sequences that wrap around.
    int l = 0;
    int r = writeIdx - 1;
    while (l < r) {
        if (face->vertexIds[l] == face->vertexIds[r]) {
            r--; // A***A -> A***.
        } else if (r - l > 1 && face->vertexIds[l] == face->vertexIds[r - 1]) {
            r -= 2; // A***AB -> A***.
        } else if (r - l > 1 && face->vertexIds[l + 1] == face->vertexIds[r]) {
            l += 2; // BA***A -> ***A.
        } else {
            break; // No new sequences to collapse.
        }
    }

    // Final pass - shift if needed.
    if (l != 0) {
        for (int i = l; i <= r; i++)
            copyVertex(i, i - l);
    }

    face->numVertices = r - l + 1;

    face->vertexIds.resize(face->numVertices);
    face->textureUs.resize(face->numVertices);
    face->textureVs.resize(face->numVertices);
}

/**
 * @param face                          Face to compute the normal of.
 * @param vertices                      Vertex positions, indexed by `face.vertexIds`.
 * @return                              Unit normal of the face, or `std::nullopt` if the face has no area.
 */
template<class Face>
static std::optional<Vec3f> faceNormal(const Face &face, std::span<const Vec3f> vertices) {
    // Compute the normal from the first non-degenerate edge pair.
    Vec3f normal;
    for (int i = 0; i < face.numVertices; i++) {
        Vec3f dir1 = vertices[face.vertexIds[(i + 1) % face.numVertices]] - vertices[face.vertexIds[i]];
        Vec3f dir2 = vertices[face.vertexIds[(i + 2) % face.numVertices]] - vertices[face.vertexIds[(i + 1) % face.numVertices]];
        normal = cross(dir1, dir2);
        if (normal.lengthSqr() > 1e-12f)
            break;
    }

    if (normal.lengthSqr() <= 1e-12f)
        return std::nullopt;

    // TODO(captainurist): just use Newell's method for everything & retrace.
    // For non-planar polygons a single edge pair can give a wrong normal. Check against the Newell's method
    // normal (sum of all cross products) and use it instead if the two disagree.
    Vec3f sumNormal;
    for (int i = 0; i < face.numVertices; i++) {
        Vec3f dir1 = vertices[face.vertexIds[(i + 1) % face.numVertices]] - vertices[face.vertexIds[i]];
        Vec3f dir2 = vertices[face.vertexIds[(i + 2) % face.numVertices]] - vertices[face.vertexIds[(i + 1) % face.numVertices]];
        sumNormal += cross(dir1, dir2);
    }
    if (dot(normal, sumNormal) <= 0)
        normal = sumNormal;

    return normal / normal.length();
}

/**
 * Recomputes the plane of a face from its vertices, and collapses the face to two vertices if it has no area.
 *
 * @param face                          Face to repair.
 * @param vertices                      Vertex positions, indexed by `face->vertexIds`.
 * @param closedVertices                Vertex positions with every door closed, empty for outdoor models, which
 *                                      have no doors. A face with no area in `vertices` but some here is stretched
 *                                      by a door, and takes its normal from here instead of being collapsed.
 */
template<class Face>
static void repairFaceNormal(Face *face, std::span<const Vec3f> vertices, std::span<const Vec3f> closedVertices) {
    if (face->numVertices < 3)
        return;

    std::optional<Vec3f> normal = faceNormal(*face, vertices);
    if (!normal && !closedVertices.empty())
        normal = faceNormal(*face, closedVertices);

    if (!normal) {
        // TODO(captainurist): drop such faces instead, ids are referenced from sectors, doors, the bsp tree and saves.
        face->numVertices = 2;
        return;
    }

    face->facePlane.normal = *normal;
    face->facePlane.dist = -dot(face->facePlane.normal, vertices[face->vertexIds[0]]);
    face->zCalc.init(face->facePlane);
}

void reconstruct(const IndoorLocation_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.vertices, &dst->vertices);
    reconstruct(src.faces, &dst->faces);

    // reconstruct temp face data locally and insert into faces
    std::vector<int16_t> faceData;
    reconstruct(src.faceData, &faceData);

    for (size_t i = 0, j = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];

        pFace->vertexIds = std::vector<int16_t>(faceData.data() + j, faceData.data() + j + pFace->numVertices);
        j += pFace->numVertices + 1; // +1 to skip closing vertex in source data.

        // Skipping pXInterceptDisplacements.
        j += pFace->numVertices + 1;

        // Skipping pYInterceptDisplacements.
        j += pFace->numVertices + 1;

        // Skipping pZInterceptDisplacements.
        j += pFace->numVertices + 1;

        pFace->textureUs = std::vector<int16_t>(faceData.data() + j, faceData.data() + j + pFace->numVertices);
        j += pFace->numVertices + 1;

        pFace->textureVs = std::vector<int16_t>(faceData.data() + j, faceData.data() + j + pFace->numVertices);
        j += pFace->numVertices + 1;

        if (j > faceData.size())
            throw Exception("BLV face data overflow: offset {} exceeds size {}", j, faceData.size());
    }

    for (BLVFace &face : dst->faces)
        dropDuplicateFaceVertices(&face);

    for (size_t i = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];

        std::string texName;
        reconstruct(src.faceTextures[i], &texName);
        pFace->SetTexture(texName);
    }

    // Reconstruct temp face extras locally and insert into faces
    std::vector<BLVFaceExtra> faceExtras;
    reconstruct(src.faceExtras, &faceExtras);

    std::string textureName;
    for (unsigned i = 0; i < faceExtras.size(); ++i) {
        reconstruct(src.faceExtraTextures[i], &textureName);

        if (textureName.empty())
            faceExtras[i].additionalBitmapId = -1;
        else
            faceExtras[i].additionalBitmapId = -1; //pBitmaps_LOD->loadTexture(textureName); // TODO(captainurist): unused for some reason.
    }

    for (size_t i = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];
        BLVFaceExtra *pFaceExtra = &faceExtras[src.faces[i].faceExtraId];
        pFace->faceId = pFaceExtra->faceId;
        pFace->additionalBitmapId = pFaceExtra->additionalBitmapId;
        pFace->textureDeltaU = pFaceExtra->textureDeltaU;
        pFace->textureDeltaV = pFaceExtra->textureDeltaV;
        pFace->cogNumber = pFaceExtra->cogNumber;
        pFace->eventId = pFaceExtra->eventId;

        if (pFace->eventId) {
            if (pFace->HasEventHint())
                pFace->attributes |= FACE_EVENT_IS_HINT;
            else
                pFace->attributes &= ~FACE_EVENT_IS_HINT;
        }
    }

    reconstruct(src.sectors, &dst->sectors);
    reconstruct(src.sectorData, &dst->sectorData);

    for (size_t i = 0, j = 0; i < dst->sectors.size(); ++i) {
        BLVSector *dstSector = &dst->sectors[i];
        const BLVSector_MM7 &srcSector = src.sectors[i];

        dstSector->floorIds = std::span(dst->sectorData.data() + j, srcSector.numFloors);
        j += srcSector.numFloors;

        dstSector->wallIds = std::span(dst->sectorData.data() + j, srcSector.numWalls);
        j += srcSector.numWalls;

        dstSector->ceilingIds = std::span(dst->sectorData.data() + j, srcSector.numCeilings);
        j += srcSector.numCeilings;

        j += srcSector.numFluids; // Fluids not used in OE, skip.

        dstSector->portalIds = std::span(dst->sectorData.data() + j, srcSector.numPortals);
        j += srcSector.numPortals;

        dstSector->faceIds = std::span(dst->sectorData.data() + j, srcSector.numFaces);
        dstSector->nonBspFaceIds = dstSector->faceIds.subspan(0, srcSector.numNonBspFaces);
        j += srcSector.numFaces;

        j += srcSector.numCogs; // Cogs not used in OE, skip.

        dstSector->decorationIds = std::span(dst->sectorData.data() + j, srcSector.numDecorations);
        j += srcSector.numDecorations;

        j += srcSector.numMarkers; // Markers not used in OE, skip.

        if (j > dst->sectorData.size())
            throw Exception("BLV sector data overflow: offset {} exceeds size {}", j, dst->sectorData.size());
    }

    reconstruct(src.sectorLightData, &dst->sectorLightData);

    for (size_t i = 0, j = 0; i < dst->sectors.size(); ++i) {
        BLVSector *dstSector = &dst->sectors[i];
        const BLVSector_MM7 &srcSector = src.sectors[i];

        dstSector->lightIds = std::span(dst->sectorLightData.data() + j, srcSector.numLights);
        j += srcSector.numLights;

        if (j > dst->sectorLightData.size())
            throw Exception("BLV sector light data overflow: offset {} exceeds size {}", j, dst->sectorLightData.size());
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationTable->decorationId(decorationName);
    }

    reconstruct(src.lights, &dst->lights);
    reconstruct(src.bspNodes, &dst->nodes);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
    reconstruct(src.mapOutlines, &dst->mapOutlines);
}

void deserialize(InputStream &src, IndoorLocation_MM7 *dst) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->vertices);
    deserialize(src, &dst->faces);
    deserialize(src, &dst->faceData, tags::presized(dst->header.faceDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->faceTextures, tags::presized(dst->faces.size()));
    deserialize(src, &dst->faceExtras);
    deserialize(src, &dst->faceExtraTextures, tags::presized(dst->faceExtras.size()));
    if (isMm8()) {
        std::vector<BLVSector_MM8> sectors;
        deserialize(src, &sectors);
        dst->sectors.clear();
        for (const BLVSector_MM8 &sector : sectors)
            dst->sectors.push_back(sectorFromMm8(sector));
    } else {
        deserialize(src, &dst->sectors);
    }
    deserialize(src, &dst->sectorData, tags::presized(dst->header.sectorDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->sectorLightData, tags::presized(dst->header.sectorLightDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->doorCount);
    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    if (isMm8()) {
        std::vector<BLVLight_MM8> lights;
        deserialize(src, &lights);
        dst->lights.clear();
        for (const BLVLight_MM8 &light : lights)
            dst->lights.push_back(light.light);
    } else {
        deserialize(src, &dst->lights);
    }
    deserialize(src, &dst->bspNodes);
    deserialize(src, &dst->spawnPoints);
    deserialize(src, &dst->mapOutlines);
}

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst) {
    snapshot(src.dlv, &dst->header.info);
    dst->header.totalFacesCount = src.faces.size();
    dst->header.bmodelCount = 0;
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src._visible_outlines, &dst->visibleOutlines);

    // Symmetric to what's happening in reconstruct - not all of the attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BLVFace &pFace : pIndoor->faces)
        dst->faceAttributes.push_back(std::to_underlying(pFace.attributes & ~(FACE_EVENT_IS_HINT | FACE_ANIMATED)));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    // TODO(captainurist): vanilla MM7 only allocated memory for 500 actors, 1000 sprite objects, and 20 chests at
    //                     runtime. We should either cap these or fail gracefully on save to maintain compatibility.
    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(src.doors, &dst->doors);
    snapshot(src.doorsData, &dst->doorsData);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    memzero(&dst->weather); // Indoor maps have no weather.
    dst->lastVisitTime = src.lastVisitTime.ticks();
}

void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.header.info, &dst->dlv); // XXX
    reconstruct(src.visibleOutlines, &dst->_visible_outlines);

    for (size_t i = 0; i < dst->mapOutlines.size(); ++i) {
        BLVMapOutline *pVertex = &dst->mapOutlines[i];
        if ((uint8_t)(1 << (7 - i % 8)) & dst->_visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BLVFace &face : dst->faces) {
        face.attributes &= FACE_ANIMATED | FACE_EVENT_IS_HINT;
        face.attributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~(FACE_EVENT_IS_HINT | FACE_ANIMATED);
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    nextActorReuseScanStart = 0;
    for (size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.itemId != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].spriteId = static_cast<SpriteId>(pItemTable->items[pSpriteObjects[i].containing_item.itemId].spriteId);
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].spriteId);
        }
    }

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.doors, &dst->doors);
    reconstruct(src.doorsData, &dst->doorsData);

    for (unsigned i = 0, j = 0; i < dst->doors.size(); ++i) {
        BLVDoor *pDoor = &dst->doors[i];

        pDoor->pVertexIDs = dst->doorsData.data() + j;
        j += pDoor->numVertices;

        pDoor->pFaceIDs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pSectorIDs = dst->doorsData.data() + j;
        j += pDoor->numSectors;

        pDoor->pDeltaUs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pDeltaVs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pXOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        pDoor->pYOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        pDoor->pZOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        if (j > dst->doorsData.size())
            throw Exception("BLV door data overflow: offset {} exceeds size {}", j, dst->doorsData.size());
    }

    for (size_t i = 0; i < dst->doors.size(); ++i) {
        BLVDoor *pDoor = &dst->doors[i];

        for (unsigned j = 0; j < pDoor->numFaces; ++j) {
            BLVFace *pFace = &dst->faces[pDoor->pFaceIDs[j]];
            pDoor->pDeltaUs[j] = pFace->textureDeltaU;
            pDoor->pDeltaVs[j] = pFace->textureDeltaV;
        }
    }

    std::vector<Vec3f> closedVertices;
    if (!dst->doors.empty()) {
        closedVertices = dst->vertices;
        for (const BLVDoor &door : dst->doors)
            for (int i = 0; i < door.numVertices; ++i)
                closedVertices[door.pVertexIDs[i]] = door.direction * door.moveLength + Vec3f(door.pXOffsets[i], door.pYOffsets[i], door.pZOffsets[i]);
    }

    for (BLVFace &face : dst->faces)
        repairFaceNormal(&face, dst->vertices, closedVertices);

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    dst->lastVisitTime = Time::fromTicks(src.lastVisitTime);
}

void serialize(const IndoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.visibleOutlines, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.doors, dst, tags::unsized);
    serialize(src.doorsData, dst, tags::unsized);
    serialize(src.eventVariables, dst);
    serialize(src.lastVisitTime, dst);
    serialize(src.weather, dst);
}

void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->doors, tags::presized(ctx->doorCount));
    deserialize(src, &dst->doorsData, tags::presized(ctx->header.doorsDataSizeBytes / sizeof(int16_t)));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst) {
    const auto &[srcData, srcExtras] = src;

    // dst->index is set externally.
    dst->position = srcData.position.toFloat();
    dst->boundingBox.x1 = srcData.minX;
    dst->boundingBox.y1 = srcData.minY;
    dst->boundingBox.z1 = srcData.minZ;
    dst->boundingBox.x2 = srcData.maxX;
    dst->boundingBox.y2 = srcData.maxY;
    dst->boundingBox.z2 = srcData.maxZ;
    dst->boundingCenter = srcData.boundingCenter.toFloat();
    dst->boundingRadius = srcData.boundingRadius;

    reconstruct(srcExtras.vertices, &dst->vertices);
    dst->faces.clear();
    dst->faces.resize(srcExtras.faces.size());
    for (int i = 0; i < srcExtras.faces.size(); i++) {
        reconstruct(srcExtras.faces[i], &dst->faces[i], tags::context(i)); // tag to set indexes in dst->faces
    }

    for (BLVFace &face : dst->faces) {
        dropDuplicateFaceVertices(&face);
        repairFaceNormal(&face, dst->vertices, {});
    }

    reconstruct(srcExtras.bspNodes, &dst->nodes);

    std::string textureName;
    for (size_t i = 0; i < dst->faces.size(); ++i) {
        reconstruct(srcExtras.faceTextures[i], &textureName);
        dst->faces[i].SetTexture(textureName);

        if (dst->faces[i].eventId) {
            if (dst->faces[i].HasEventHint())
                dst->faces[i].attributes |= FACE_EVENT_IS_HINT;
            else
                dst->faces[i].attributes &= ~FACE_EVENT_IS_HINT;
        }
    }
}

static int mapToGlobalTileId(const std::array<int, 4> &baseIds, int localTileId) {
    // Tiles in tilemap:
    // [0..90) are mapped as-is, but seem to be mostly invalid. Only global tile ids [1..12] are valid (all are dirt),
    //         the rest are "pending", effectively invalid.
    // [90..126) map to tileset #1.
    // [126..162) map to tileset #2.
    // [162..198) map to tileset #3.
    // [198..234) map to tileset #4 (road).
    // [234..255) are invalid.
    if (localTileId < 90)
        return localTileId;

    if (localTileId >= 234)
        return 0;

    int tilesetIndex = (localTileId - 90) / 36;
    int tilesetOffset = (localTileId - 90) % 36;
    return baseIds[tilesetIndex] + tilesetOffset;
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst) {
    std::array<int, 4> baseTileIds;
    for (int i = 0; i < 4; i++) {
        reconstruct(src.tileTypes[i].tileset, &dst->_tilesets[i]);
        baseTileIds[i] = pTileTable->tileId(dst->_tilesets[i], isRoad(dst->_tilesets[i]) ? TILE_VARIANT_ROAD_N_S_E_W : TILE_VARIANT_BASE1);
    }

    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
            dst->_heightMap[y][x] = src.heightMap[y * 128 + x];

    for (int y = 0; y < 127; y++)
        for (int x = 0; x < 127; x++)
            dst->_originalTileMap[y][x] = mapToGlobalTileId(baseTileIds, src.tileMap[y * 128 + x]);

    dst->recalculateNormals();
    dst->recalculateTransitions(&dst->_tileMap);

    dst->_tileMap = Image<int16_t>::copy(dst->_originalTileMap);
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.skyTexture, &dst->sky_texture_filename);
    reconstruct(src, &dst->pTerrain);

    dst->pBModels.clear();
    for (size_t i = 0; i < src.models.size(); i++) {
        BSPModel &model = dst->pBModels.emplace_back();
        model.index = i;
        reconstruct(std::forward_as_tuple(src.models[i], src.modelExtras[i]), &model);

        // Recalculate bounding spheres, the ones stored in data files are borked.
        model.boundingCenter = model.boundingBox.center().toFloat();
        model.boundingRadius = model.boundingBox.size().toFloat().length() / 2.0f;
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationTable->decorationId(decorationName);
    }

    reconstruct(src.decorationPidList, &dst->pFaceIDLIST);
    reconstruct(src.decorationMap, &dst->pOMAP);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
}

void deserialize(InputStream &src, OutdoorLocation_MM7 *dst) {
    deserialize(src, &dst->name);
    deserialize(src, &dst->fileName);
    deserialize(src, &dst->description);
    deserialize(src, &dst->skyTexture);
    deserialize(src, &dst->groundTilesetUnused);
    deserialize(src, &dst->tileTypes);
    if (isMm8())
        deserialize(src, &dst->mm8TilesetsFile);
    deserialize(src, &dst->heightMap);
    deserialize(src, &dst->tileMap);
    deserialize(src, &dst->attributeMap);
    deserialize(src, &dst->normalCount);
    deserialize(src, &dst->someOtherMap);
    deserialize(src, &dst->normalMap);
    deserialize(src, &dst->normals, tags::presized(dst->normalCount));
    deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        deserialize(src, &extra.vertices, tags::presized(model.numVertices));
        deserialize(src, &extra.faces, tags::presized(model.numFaces));
        deserialize(src, &extra.facesOrdering, tags::presized(model.numFaces));
        deserialize(src, &extra.bspNodes, tags::presized(model.numNodes));
        deserialize(src, &extra.faceTextures, tags::presized(model.numFaces));
    }

    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->decorationPidList);
    deserialize(src, &dst->decorationMap);
    deserialize(src, &dst->spawnPoints);
}

static void deserializeMm6Decorations(InputStream &src, std::vector<LevelDecoration_MM7> *dst) {
    // MM6 decorations are 28 bytes, the event variable sits where MM7 has the cog number.
    uint32_t count = 0;
    deserialize(src, &count);
    dst->clear();
    for (uint32_t i = 0; i < count; i++) {
        std::array<uint8_t, 0x1C> raw;
        deserialize(src, &raw);
        LevelDecoration_MM7 &decoration = dst->emplace_back();
        memset(&decoration, 0, sizeof(decoration));
        memcpy(&decoration, raw.data(), 0x14);
        memcpy(&decoration.uEventID, raw.data() + 0x16, 6); // Event, trigger radius, yaw in degrees.
        int16_t eventVariable;
        memcpy(&eventVariable, raw.data() + 0x14, 2);
        decoration.eventVarId = eventVariable ? eventVariable + 75 : 0; // Reconstruct subtracts 75, see LevelDecoration_MM7.
    }
}

static void deserializeMm6SpawnPoints(InputStream &src, std::vector<SpawnPoint_MM7> *dst) {
    uint32_t count = 0;
    deserialize(src, &count);
    dst->clear();
    for (uint32_t i = 0; i < count; i++) {
        SpawnPoint_MM6 spawn;
        deserialize(src, &spawn);
        SpawnPoint_MM7 &result = dst->emplace_back();
        memset(&result, 0, sizeof(result));
        memcpy(&result, &spawn, sizeof(spawn));
    }
}

void deserializeMm6(InputStream &src, IndoorLocation_MM7 *dst) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->vertices);

    // MM6 faces are MM7 faces without the leading float plane.
    uint32_t faceCount = 0;
    deserialize(src, &faceCount);
    dst->faces.clear();
    for (uint32_t i = 0; i < faceCount; i++) {
        std::array<uint8_t, 0x50> raw;
        deserialize(src, &raw);
        BLVFace_MM7 &face = dst->faces.emplace_back();
        memset(&face, 0, sizeof(face));
        memcpy(&face.facePlaneOld, raw.data(), raw.size());
        face.facePlane.normal = Vec3f(face.facePlaneOld.normal.x, face.facePlaneOld.normal.y, face.facePlaneOld.normal.z) / 65536.0f;
        face.facePlane.dist = face.facePlaneOld.dist / 65536.0f;
    }

    deserialize(src, &dst->faceData, tags::presized(dst->header.faceDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->faceTextures, tags::presized(dst->faces.size()));
    deserialize(src, &dst->faceExtras);
    deserialize(src, &dst->faceExtraTextures, tags::presized(dst->faceExtras.size()));
    deserialize(src, &dst->sectors);
    deserialize(src, &dst->sectorData, tags::presized(dst->header.sectorDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->sectorLightData, tags::presized(dst->header.sectorLightDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->doorCount);
    deserializeMm6Decorations(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));

    // MM6 lights: position, attributes, brightness, radius. MM6 lights are white.
    uint32_t lightCount = 0;
    deserialize(src, &lightCount);
    dst->lights.clear();
    for (uint32_t i = 0; i < lightCount; i++) {
        std::array<int16_t, 6> raw;
        deserialize(src, &raw);
        BLVLight_MM7 &light = dst->lights.emplace_back();
        memset(&light, 0, sizeof(light));
        light.vPosition = Vec3s(raw[0], raw[1], raw[2]);
        light.uAtributes = raw[3];
        light.uBrightness = raw[4];
        light.uRadius = raw[5];
        light.uRed = light.uGreen = light.uBlue = static_cast<char>(255);
    }

    deserialize(src, &dst->bspNodes);
    deserializeMm6SpawnPoints(src, &dst->spawnPoints);
    deserialize(src, &dst->mapOutlines);
}

void deserializeMm6(InputStream &src, OutdoorLocation_MM7 *dst) {
    deserialize(src, &dst->name);
    deserialize(src, &dst->fileName);
    deserialize(src, &dst->description);
    deserialize(src, &dst->skyTexture);
    deserialize(src, &dst->groundTilesetUnused);
    deserialize(src, &dst->tileTypes);
    deserialize(src, &dst->heightMap);
    deserialize(src, &dst->tileMap);
    deserialize(src, &dst->attributeMap);
    dst->normalCount = 0;
    dst->someOtherMap.fill(0);
    dst->normalMap.fill(0);
    dst->normals.clear();
    deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        deserialize(src, &extra.vertices, tags::presized(model.numVertices));
        deserialize(src, &extra.faces, tags::presized(model.numFaces));
        deserialize(src, &extra.facesOrdering, tags::presized(model.numFaces));
        deserialize(src, &extra.bspNodes, tags::presized(model.numNodes));
        deserialize(src, &extra.faceTextures, tags::presized(model.numFaces));
    }

    deserializeMm6Decorations(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->decorationPidList);
    deserialize(src, &dst->decorationMap);

    deserializeMm6SpawnPoints(src, &dst->spawnPoints);
}

static Item_MM7 itemFromMm6(const uint8_t *raw) {
    Item_MM7 result;
    memset(&result, 0, sizeof(result));
    memcpy(&result, raw, 0x1C);
    if (result.itemId > 0)
        result.itemId = std::to_underlying(itemIdFromMm6(result.itemId));
    return result;
}

static Actor_MM7 actorFromMm6(const std::array<uint8_t, 0x224> &raw) {
    Actor_MM7 result;
    memset(&result, 0, sizeof(result));
    memcpy(&result, raw.data(), 0x2C); // Name, npc id, attributes, hp.
    result.monsterInfo.id = raw[0x2C + 8];
    memcpy(&result.field_84, raw.data() + 0x74, 0x50); // Monster id, size, position... up to the sounds.
    if (result.monsterId == 0)
        result.monsterId = result.monsterInfo.id; // Not yet spawned actors in MM6 .ddm files only have the id in the stats copy.
    memcpy(&result.group, raw.data() + 0x1A4, 0x70); // Group, ally, schedules, summoner, last attacker.
    return result;
}

static SpriteObject_MM7 spriteObjectFromMm6(const std::array<uint8_t, 0x64> &raw) {
    SpriteObject_MM7 result;
    memset(&result, 0, sizeof(result));
    memcpy(&result, raw.data(), 0x24);
    result.containing_item = itemFromMm6(raw.data() + 0x24);
    memcpy(&result.uSpellID, raw.data() + 0x40, 12); // Spell, skill, mastery.
    memcpy(&result.spell_caster_pid, raw.data() + 0x4C, 0x18); // Owner, target, range, attack type, start position.
    return result;
}

static void deserializeMm6Entities(InputStream &src, std::vector<Actor_MM7> *actors, std::vector<SpriteObject_MM7> *spriteObjects, std::vector<Chest_MM7> *chests) {
    uint32_t count = 0;
    deserialize(src, &count);
    actors->clear();
    for (uint32_t i = 0; i < count; i++) {
        std::array<uint8_t, 0x224> raw;
        deserialize(src, &raw);
        actors->push_back(actorFromMm6(raw));
    }

    deserialize(src, &count);
    spriteObjects->clear();
    for (uint32_t i = 0; i < count; i++) {
        std::array<uint8_t, 0x64> raw;
        deserialize(src, &raw);
        spriteObjects->push_back(spriteObjectFromMm6(raw));
    }

    deserialize(src, &count);
    chests->clear();
    for (uint32_t i = 0; i < count; i++) {
        std::array<uint8_t, 4 + 140 * 0x1C> raw;
        Chest_MM7 &chest = chests->emplace_back();
        deserialize(src, &raw);
        memcpy(&chest, raw.data(), 4);
        for (int j = 0; j < 140; j++)
            chest.items[j] = itemFromMm6(raw.data() + 4 + j * 0x1C);
        deserialize(src, &chest.inventoryMatrix);
    }
}

/**
 * @param raw                           MM8 map monster, laid out as MMExtension's MapMonster.
 * @return                              The same monster in the MM7 layout. MM8 has two byte resistances and eight more
 *                                      buffs, the buffs past the MM7 ones are dropped.
 */
static Actor_MM7 actorFromMm8(const std::array<uint8_t, 0x3CC> &raw) {
    std::array<uint8_t, sizeof(Actor_MM7)> mm7 = {};
    constexpr int STATS = 0x2C; // Where the copy of the monsters.txt line starts.
    memcpy(mm7.data(), raw.data(), STATS + 0x24); // Up to the resistances.
    for (int i = 0; i < 10; i++) {
        uint16_t resistance;
        memcpy(&resistance, raw.data() + STATS + 0x24 + 2 * i, 2);
        mm7[STATS + 0x24 + i] = std::min<int>(resistance, 255);
    }
    memcpy(mm7.data() + STATS + 0x2E, raw.data() + STATS + 0x38, 0x10); // Special attack up to the summoned monster.
    memcpy(mm7.data() + STATS + 0x40, raw.data() + STATS + 0x48, 0x18); // Hit points up to the preferred class.
    memcpy(mm7.data() + 0x84, raw.data() + 0x8C, 0x50 + 22 * 0x10); // Position up to the first 22 buffs.
    memcpy(mm7.data() + 0x234, raw.data() + 0x2BC, 0x110); // Items, group, ally, schedules and the rest.

    Actor_MM7 result;
    memcpy(&result, mm7.data(), sizeof(result));
    for (Item_MM7 &item : result.items)
        item.itemId = std::to_underlying(itemIdFromData(item.itemId));
    return result;
}

/**
 * Reads the actors, sprite objects and chests of an MM8 .ddm or .dlv file, with the item ids moved into the ranges
 * of the engine.
 */
static void deserializeMm8Entities(InputStream &src, std::vector<Actor_MM7> *actors, std::vector<SpriteObject_MM7> *spriteObjects, std::vector<Chest_MM7> *chests) {
    uint32_t count = 0;
    deserialize(src, &count);
    actors->clear();
    for (uint32_t i = 0; i < count; i++) {
        std::array<uint8_t, 0x3CC> raw;
        deserialize(src, &raw);
        actors->push_back(actorFromMm8(raw));
    }

    deserialize(src, spriteObjects);
    for (SpriteObject_MM7 &object : *spriteObjects)
        object.containing_item.itemId = std::to_underlying(itemIdFromData(object.containing_item.itemId));

    deserialize(src, chests);
    for (Chest_MM7 &chest : *chests)
        for (Item_MM7 &item : chest.items)
            item.itemId = std::to_underlying(itemIdFromData(item.itemId));
}

void deserializeMm8(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx->models)
        totalFaces += model.numFaces;

    deserialize(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);
    deserialize(src, &dst->faceAttributes, tags::presized(totalFaces));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserializeMm8Entities(src, &dst->actors, &dst->spriteObjects, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

void deserializeMm8(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserializeMm8Entities(src, &dst->actors, &dst->spriteObjects, &dst->chests);
    deserialize(src, &dst->doors, tags::presized(ctx->doorCount));
    deserialize(src, &dst->doorsData, tags::presized(ctx->header.doorsDataSizeBytes / sizeof(int16_t)));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

static void deserializeMm6LocationHeader(InputStream &src, LocationHeader_MM7 *dst) {
    memset(dst, 0, sizeof(*dst));
    std::array<int32_t, 2> info;
    deserialize(src, &info);
    dst->info.respawnCount = info[0];
    dst->info.lastRespawnDay = info[1];
}

void deserializeMm6(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx) {
    deserializeMm6LocationHeader(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);

    dst->faceAttributes.clear();
    for (const BSPModelExtras_MM7 &extra : ctx->modelExtras)
        for (const ODMFace_MM7 &face : extra.faces)
            dst->faceAttributes.push_back(face.attributes);
    dst->decorationFlags.clear();
    for (const LevelDecoration_MM7 &decoration : ctx->decorations)
        dst->decorationFlags.push_back(decoration.uFlags);

    deserializeMm6Entities(src, &dst->actors, &dst->spriteObjects, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

void deserializeMm6(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserializeMm6LocationHeader(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);

    dst->faceAttributes.clear();
    for (const BLVFace_MM7 &face : ctx->faces)
        dst->faceAttributes.push_back(face.attributes);
    dst->decorationFlags.clear();
    for (const LevelDecoration_MM7 &decoration : ctx->decorations)
        dst->decorationFlags.push_back(decoration.uFlags);

    deserializeMm6Entities(src, &dst->actors, &dst->spriteObjects, &dst->chests);
    deserialize(src, &dst->doors, tags::presized(ctx->doorCount));
    deserialize(src, &dst->doorsData, tags::presized(ctx->header.doorsDataSizeBytes / sizeof(int16_t)));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst) {
    snapshot(src.ddm, &dst->header.info);
    dst->header.totalFacesCount = 0;
    for (const BSPModel &model : src.pBModels)
        dst->header.totalFacesCount += model.faces.size();
    dst->header.bmodelCount = src.pBModels.size();
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src.uFullyRevealedCellOnMap, &dst->fullyRevealedCells);
    snapshot(src.uPartiallyRevealedCellOnMap, &dst->partiallyRevealedCells);

    // Symmetric to what's happening in reconstruct - no all attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BSPModel &model : src.pBModels)
        for (const BLVFace &face : model.faces)
            dst->faceAttributes.push_back(std::to_underlying(face.attributes & ~(FACE_EVENT_IS_HINT | FACE_ANIMATED)));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    // TODO(captainurist): vanilla MM7 only allocated memory for 500 actors, 1000 sprite objects, and 20 chests at
    //                     runtime. We should either cap these or fail gracefully on save to maintain compatibility.
    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    snapshot(src.weather, &dst->weather);
    dst->lastVisitTime = src.lastVisitTime.ticks();
}

void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.header.info, &dst->ddm);
    reconstruct(src.fullyRevealedCells, &dst->uFullyRevealedCellOnMap);
    reconstruct(src.partiallyRevealedCells, &dst->uPartiallyRevealedCellOnMap);

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BSPModel &model : dst->pBModels) {
        for (BLVFace &face : model.faces) {
            face.attributes &= FACE_ANIMATED | FACE_EVENT_IS_HINT;
            face.attributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~(FACE_EVENT_IS_HINT | FACE_ANIMATED);
        }
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    nextActorReuseScanStart = 0;
    for (size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    reconstruct(src.weather, &dst->weather);
    dst->lastVisitTime = Time::fromTicks(src.lastVisitTime);
}

void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.fullyRevealedCells, dst);
    serialize(src.partiallyRevealedCells, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.eventVariables, dst);
    serialize(src.lastVisitTime, dst);
    serialize(src.weather, dst);
}

void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx->models)
        totalFaces += model.numFaces;

    deserialize(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);
    deserialize(src, &dst->faceAttributes, tags::presized(totalFaces));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->lastVisitTime);
    deserialize(src, &dst->weather);
}

void snapshot(const SaveGame &src, SaveGame_MM7 *dst) {
    snapshot(src.header, &dst->header);
    snapshot(src.party, &dst->party);
    snapshot(src.eventTimer, &dst->eventTimer);
    snapshot(src.overlays, &dst->overlays);
    for (int i = 0; i < dst->npcData.size(); i++)
        snapshot(src.npcData[i], &dst->npcData[i]);
    dst->mm8NpcData.clear();
    dst->mm8FifthCharacter.clear();
    dst->mm8InnCharacters.clear();
    dst->mm8RosterIds.clear();
    dst->mm8RacialBuffs.clear();
    if (isMm8()) {
        auto saveRacialBuffs = [&](const Character &character) {
            for (CharacterBuff buff : {CHARACTER_BUFF_MM8_GLAMOUR, CHARACTER_BUFF_MM8_LEVITATE, CHARACTER_BUFF_MM8_MISTFORM})
                snapshot(character.pCharacterBuffs[buff], &dst->mm8RacialBuffs.emplace_back());
        };
        for (const Character &character : src.party.pCharacters)
            saveRacialBuffs(character);
        for (const Character &character : src.party.mm8InnCharacters)
            saveRacialBuffs(character);
        for (int i = dst->npcData.size(); i < src.npcData.size(); i++)
            snapshot(src.npcData[i], &dst->mm8NpcData.emplace_back());
        if (src.party.pCharacters.size() == 5)
            snapshot(src.party.pCharacters[4], &dst->mm8FifthCharacter.emplace_back());
        for (const Character &character : src.party.pCharacters)
            dst->mm8RosterIds.push_back(character.mm8RosterId);
        for (const Character &character : src.party.mm8InnCharacters) {
            snapshot(character, &dst->mm8InnCharacters.emplace_back());
            dst->mm8RosterIds.push_back(character.mm8RosterId);
        }
    }
    snapshot(src.npcGroups, &dst->npcGroups);

    // Share map deltas.
    dst->mapDeltas.clear();
    for (const auto &[key, value] : src.mapDeltas)
        dst->mapDeltas[key] = Blob::share(value);

    // Encode Lloyd's Beacon images from party.
    dst->lloydImages.clear();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (!src.party.pCharacters.firstFour()[i].vBeacons[j])
                continue;
            const LloydBeacon &beacon = *src.party.pCharacters.firstFour()[i].vBeacons[j];
            if (beacon.uBeaconTime.isValid() && beacon.image != nullptr)
                dst->lloydImages[{i, j}] = pcx::encode(beacon.image->rgba());
        }
    }

    dst->thumbnail = Blob::share(src.thumbnail);
}

void reconstruct(const SaveGame_MM7 &src, SaveGame *dst) {
    reconstruct(src.header, &dst->header);
    reconstruct(src.party, &dst->party);
    reconstruct(src.eventTimer, &dst->eventTimer);
    reconstruct(src.overlays, &dst->overlays);
    for (int i = 0; i < src.npcData.size(); i++)
        reconstruct(src.npcData[i], &dst->npcData[i]);
    for (int i = 0; i < src.mm8NpcData.size() && src.npcData.size() + i < dst->npcData.size(); i++)
        reconstruct(src.mm8NpcData[i], &dst->npcData[src.npcData.size() + i]);
    if (!src.mm8FifthCharacter.empty()) {
        dst->party.pCharacters.resize(5);
        reconstruct(src.mm8FifthCharacter[0], &dst->party.pCharacters.slots()[4], tags::context(4));
    }
    for (int i = 0; i < src.mm8InnCharacters.size(); i++)
        reconstruct(src.mm8InnCharacters[i], &dst->party.mm8InnCharacters.emplace_back(), tags::context(5 + i));
    if (src.mm8RacialBuffs.size() == 3 * (dst->party.pCharacters.size() + dst->party.mm8InnCharacters.size())) {
        int index = 0;
        auto loadRacialBuffs = [&](Character &character) {
            for (CharacterBuff buff : {CHARACTER_BUFF_MM8_GLAMOUR, CHARACTER_BUFF_MM8_LEVITATE, CHARACTER_BUFF_MM8_MISTFORM})
                reconstruct(src.mm8RacialBuffs[index++], &character.pCharacterBuffs[buff]);
        };
        for (Character &character : dst->party.pCharacters)
            loadRacialBuffs(character);
        for (Character &character : dst->party.mm8InnCharacters)
            loadRacialBuffs(character);
    }
    if (src.mm8RosterIds.size() == dst->party.pCharacters.size() + dst->party.mm8InnCharacters.size()) {
        for (int i = 0; i < dst->party.pCharacters.size(); i++)
            dst->party.pCharacters[i].mm8RosterId = src.mm8RosterIds[i];
        for (int i = 0; i < dst->party.mm8InnCharacters.size(); i++)
            dst->party.mm8InnCharacters[i].mm8RosterId = src.mm8RosterIds[dst->party.pCharacters.size() + i];
    }
    reconstruct(src.npcGroups, &dst->npcGroups);

    // Share map deltas.
    dst->mapDeltas.clear();
    for (const auto &[key, value] : src.mapDeltas)
        dst->mapDeltas[key] = Blob::share(value);

    // Decode Lloyd's Beacon images into party.
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (!dst->party.pCharacters.firstFour()[i].vBeacons[j])
                continue;
            LloydBeacon &beacon = *dst->party.pCharacters.firstFour()[i].vBeacons[j];
            beacon.image = GraphicsImage::Create(pcx::decode(src.lloydImages.at({i, j})));
        }
    }

    dst->thumbnail = Blob::share(src.thumbnail);
}

void serialize(const SaveGame_MM7 &src, Blob *dst) {
    LodInfo lodInfo;
    lodInfo.version = LOD_VERSION_MM7;
    lodInfo.rootName = "chapter";
    lodInfo.description = "newmaps for MMVII";

    BlobOutputStream stream(dst);
    LodWriter lodWriter(&stream, std::move(lodInfo));

    lodWriter.write("header.bin", toBlob(src.header));
    lodWriter.write("party.bin", toBlob(src.party));
    lodWriter.write("clock.bin", toBlob(src.eventTimer));
    lodWriter.write("overlay.bin", toBlob(src.overlays));
    lodWriter.write("npcdata.bin", toBlob(src.npcData));
    lodWriter.write("npcgroup.bin", toBlob(src.npcGroups));
    if (!src.mm8NpcData.empty())
        lodWriter.write("npcdata8.bin", toBlob(src.mm8NpcData));
    if (!src.mm8FifthCharacter.empty())
        lodWriter.write("player5.bin", toBlob(src.mm8FifthCharacter));
    if (!src.mm8InnCharacters.empty())
        lodWriter.write("inn.bin", toBlob(src.mm8InnCharacters));
    if (!src.mm8RosterIds.empty())
        lodWriter.write("rosterid.bin", toBlob(src.mm8RosterIds));
    if (!src.mm8RacialBuffs.empty())
        lodWriter.write("buff8.bin", toBlob(src.mm8RacialBuffs));

    for (const auto &[name, blob] : src.mapDeltas)
        lodWriter.write(name, blob);

    for (const auto &[key, blob] : src.lloydImages)
        lodWriter.write(fmt::format("lloyd{}{}.pcx", key.first + 1, key.second + 1), blob);

    lodWriter.write("image.pcx", src.thumbnail);

    // Apparently vanilla had two bugs canceling each other out:
    // 1. Broken binary search implementation when looking up LOD entries.
    // 2. Writing additional duplicate entry at the end of a saves LOD file.
    // Our code doesn't support duplicate entries, so we just add a dummy entry.
    lodWriter.write("z.bin", Blob::fromString("dummy"));

    lodWriter.close();
    stream.close();
}

void deserialize(const Blob &src, SaveGame_MM7 *dst) {
    LodReader lodReader(Blob::share(src), LOD_ALLOW_DUPLICATES);

    deserialize(lodReader.read("header.bin"), &dst->header);
    deserialize(lodReader.read("party.bin"), &dst->party);
    deserialize(lodReader.read("clock.bin"), &dst->eventTimer);
    deserialize(lodReader.read("overlay.bin"), &dst->overlays);
    deserialize(lodReader.read("npcdata.bin"), &dst->npcData);
    deserialize(lodReader.read("npcgroup.bin"), &dst->npcGroups);
    dst->mm8NpcData.clear();
    if (lodReader.exists("npcdata8.bin"))
        deserialize(lodReader.read("npcdata8.bin"), &dst->mm8NpcData);
    dst->mm8FifthCharacter.clear();
    if (lodReader.exists("player5.bin"))
        deserialize(lodReader.read("player5.bin"), &dst->mm8FifthCharacter);
    dst->mm8InnCharacters.clear();
    if (lodReader.exists("inn.bin"))
        deserialize(lodReader.read("inn.bin"), &dst->mm8InnCharacters);
    dst->mm8RosterIds.clear();
    if (lodReader.exists("rosterid.bin"))
        deserialize(lodReader.read("rosterid.bin"), &dst->mm8RosterIds);
    dst->mm8RacialBuffs.clear();
    if (lodReader.exists("buff8.bin"))
        deserialize(lodReader.read("buff8.bin"), &dst->mm8RacialBuffs);

    dst->mapDeltas.clear();
    for (const std::string &name : lodReader.ls())
        if (name.ends_with(".ddm") || name.ends_with(".dlv"))
            dst->mapDeltas[name] = lodReader.read(name);

    dst->lloydImages.clear();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            std::string name = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
            if (lodReader.exists(name))
                dst->lloydImages[{i, j}] = lodReader.read(name);
        }
    }

    dst->thumbnail = lodReader.read("image.pcx");
}

void reconstruct(const SaveGameLite_MM7 &src, SaveGameLite *dst) {
    reconstruct(src.header, &dst->header);
    dst->thumbnail = Blob::share(src.thumbnail);
}

void deserialize(const Blob &src, SaveGameLite_MM7 *dst) {
    LodReader lodReader(Blob::share(src), LOD_ALLOW_DUPLICATES);
    deserialize(lodReader.read("header.bin"), &dst->header);
    dst->thumbnail = lodReader.read("image.pcx");
}

void reconstruct(const SpriteFrameTable_MM7 &src, SpriteFrameTable *dst) {
    reconstruct(src.frames, &dst->pSpriteSFrames);
    reconstruct(src.eframes, &dst->pSpriteEFrames);
}

void deserialize(InputStream &src, SpriteFrameTable_MM7 *dst) {
    deserialize(src, &dst->frameCount);
    deserialize(src, &dst->eframeCount);
    deserialize(src, &dst->frames, tags::presized(dst->frameCount));
    deserialize(src, &dst->eframes, tags::presized(dst->eframeCount));
}

void reconstruct(const SpriteFrameTable_MM6 &src, SpriteFrameTable *dst) {
    reconstruct(src.frames, &dst->pSpriteSFrames);
    reconstruct(src.eframes, &dst->pSpriteEFrames);
}

void deserialize(InputStream &src, SpriteFrameTable_MM6 *dst) {
    deserialize(src, &dst->frameCount);
    deserialize(src, &dst->eframeCount);
    deserialize(src, &dst->frames, tags::presized(dst->frameCount));
    deserialize(src, &dst->eframes, tags::presized(dst->eframeCount));
}
