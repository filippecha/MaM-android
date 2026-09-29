#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#include "EntitySnapshots.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Hash.h"

/**
 * @file
 *
 * Snapshots in this header are representations of game binary files, one struct per single file.
 *
 * Struct fields are laid out in the order in which they are laid out in binary files.
 */

class Blob;
class BSPModel;
struct IndoorLocation;
struct OutdoorLocation;
class OutdoorTerrain;
struct SaveGame;
struct SaveGameLite;
struct SpriteFrameTable;

struct IndoorLocation_MM7 {
    BLVHeader_MM7 header;
    std::vector<Vec3s> vertices;
    std::vector<BLVFace_MM7> faces;
    std::vector<int16_t> faceData;
    std::vector<std::array<char, 10>> faceTextures;
    std::vector<BLVFaceExtra_MM7> faceExtras;
    std::vector<std::array<char, 10>> faceExtraTextures;
    std::vector<BLVSector_MM7> sectors;
    std::vector<uint16_t> sectorData;
    std::vector<uint16_t> sectorLightData;
    uint32_t doorCount;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<BLVLight_MM7> lights;
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<SpawnPoint_MM7> spawnPoints;
    std::vector<BLVMapOutline_MM7> mapOutlines;
};

void reconstruct(const IndoorLocation_MM7 &src, IndoorLocation *dst);
void deserialize(InputStream &src, IndoorLocation_MM7 *dst);

/**
 * Reads a MM6 .blv file. MM6 faces lack the float plane, lights, decorations and spawn points are shorter.
 *
 * @param src                           Stream with the decompressed .blv file.
 * @param dst                           MM7 location to fill.
 */
void deserializeMm6(InputStream &src, IndoorLocation_MM7 *dst);


struct IndoorDelta_MM7 {
    LocationHeader_MM7 header;
    std::array<char, 875> visibleOutlines;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    std::vector<BLVDoor_MM7> doors;
    std::vector<int16_t> doorsData;
    PersistentVariables_MM7 eventVariables;
    int64_t lastVisitTime;
    MapWeather_MM7 weather; // Not used, indoor maps have no weather.
};

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst);
void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst);
void serialize(const IndoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx);

/**
 * Reads a .dlv file shipped with MM8, see the outdoor overload.
 */
void deserializeMm8(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx);

/**
 * Reads a MM6 .dlv file, see `deserializeMm6` for .ddm files for the differences.
 *
 * @param src                           Stream with the decompressed .dlv file.
 * @param dst                           MM7 delta to fill.
 * @param ctx                           Location loaded from the matching .blv file.
 */
void deserializeMm6(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx);


struct BSPModelExtras_MM7 {
    std::vector<Vec3i> vertices;
    std::vector<ODMFace_MM7> faces;
    std::vector<uint16_t> facesOrdering; // Seems to be filled either with zeros or garbage, not used in OE.
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<std::array<char, 10>> faceTextures;
};

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst);


struct OutdoorLocation_MM7 {
    std::array<char, 32> name; // Always "blank" in MM7 data.
    std::array<char, 32> fileName; // Always "default.odm" in MM7 data.
    std::array<char, 32> description; // Always "MM6 Outdoor v1.00" or "MM6 Outdoor v7.00" in MM7 data.
    std::array<char, 32> skyTexture;
    std::array<char, 32> groundTilesetUnused;
    std::array<OutdoorTileType_MM7, 4> tileTypes;
    uint32_t mm8TilesetsFile = 0; // Only in MM8 data, which tile table the map uses. 0 (dtile.bin) in all MM8 maps.
    std::array<uint8_t, 128 * 128> heightMap;
    std::array<uint8_t, 128 * 128> tileMap;
    std::array<uint8_t, 128 * 128> attributeMap;
    uint32_t normalCount; // Number of elements in `normals`.
    std::array<uint32_t , 128 * 128 * 2> someOtherMap; // Not used in OE, not even sure what this is.
    std::array<uint16_t, 128 * 128 * 2> normalMap; // Indices into `normals`, unused as we recalculate normals on load.
    std::vector<Vec3f> normals;
    std::vector<BSPModelData_MM7> models;
    std::vector<BSPModelExtras_MM7> modelExtras;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<uint16_t> decorationPidList;
    std::array<uint32_t, 128 * 128> decorationMap;
    std::vector<SpawnPoint_MM7> spawnPoints;
};

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst);
void reconstruct(const OutdoorLocation_MM7 &src, OutdoorLocation *dst);
void deserialize(InputStream &src, OutdoorLocation_MM7 *dst);

/**
 * Reads a MM6 .odm file. MM6 has no terrain normals, and shorter decoration and spawn point records.
 *
 * @param src                           Stream with the decompressed .odm file.
 * @param dst                           MM7 location to fill, normals are left empty.
 */
void deserializeMm6(InputStream &src, OutdoorLocation_MM7 *dst);

struct OutdoorDelta_MM7 {
    LocationHeader_MM7 header;
    std::array<std::array<uint8_t, 11>, 88> fullyRevealedCells;
    std::array<std::array<uint8_t, 11>, 88> partiallyRevealedCells;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    PersistentVariables_MM7 eventVariables;
    int64_t lastVisitTime;
    MapWeather_MM7 weather;
};

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst);
void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst);
void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx);

/**
 * Reads a .ddm file shipped with MM8. Its monsters have a longer record, and its items have MM8 ids.
 *
 * @param src                           Stream with the decompressed .ddm file.
 * @param dst                           MM7 delta to fill.
 * @param ctx                           The location of the delta.
 */
void deserializeMm8(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx);

/**
 * Reads a MM6 .ddm file. MM6 stores neither face attributes nor decoration flags there, those are taken from the
 * .odm file. Actors, sprite objects and chests are converted to their MM7 layouts. Actor monster info is left empty
 * and has to be filled in from the monster table by the caller.
 *
 * @param src                           Stream with the decompressed .ddm file.
 * @param dst                           MM7 delta to fill.
 * @param ctx                           Location loaded from the matching .odm file.
 */
void deserializeMm6(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx);


struct SaveGame_MM7 {
    SaveGameHeader_MM7 header; // In header.bin.
    Party_MM7 party; // In party.bin.
    Timer_MM7 eventTimer; // In clock.bin.
    ActiveOverlayList_MM7 overlays; // In overlay.bin.
    std::array<NPCData_MM7, 501> npcData; // In npcdata.bin.
    std::vector<NPCData_MM7> mm8NpcData; // In npcdata8.bin, only in MM8 saves, the NPCs past the MM7 ones.
    std::vector<Character_MM7> mm8FifthCharacter; // In player5.bin, only in MM8 saves with a party of five.
    std::vector<Character_MM7> mm8InnCharacters; // In inn.bin, only in MM8 saves.
    std::vector<int32_t> mm8RosterIds; // In rosterid.bin, only in MM8 saves. Party members first, then mm8InnCharacters.
    std::vector<SpellBuff_MM7> mm8RacialBuffs; // In buff8.bin, only in MM8 saves. Three per character in mm8RosterIds order.
    std::array<uint16_t, 51> npcGroups; // In npcgroup.bin.
    std::unordered_map<std::string, Blob> mapDeltas; // Map deltas by name (e.g. "out01.ddm", "d29.dlv").
    std::unordered_map<std::pair<int, int>, Blob> lloydImages; // Lloyd's Beacon images as PCX blobs, by {playerIndex, beaconIndex}.
    Blob thumbnail; // In image.pcx - save thumbnail.
};

void snapshot(const SaveGame &src, SaveGame_MM7 *dst);
void reconstruct(const SaveGame_MM7 &src, SaveGame *dst);
void serialize(const SaveGame_MM7 &src, Blob *dst);
void deserialize(const Blob &src, SaveGame_MM7 *dst);


struct SaveGameLite_MM7 {
    SaveGameHeader_MM7 header;
    Blob thumbnail;
};

void reconstruct(const SaveGameLite_MM7 &src, SaveGameLite *dst);
void deserialize(const Blob &src, SaveGameLite_MM7 *dst);


struct SpriteFrameTable_MM7 {
    uint32_t frameCount;
    uint32_t eframeCount;
    std::vector<SpriteFrame_MM7> frames;
    std::vector<uint16_t> eframes;
};

void reconstruct(const SpriteFrameTable_MM7 &src, SpriteFrameTable *dst);
void deserialize(InputStream &src, SpriteFrameTable_MM7 *dst);

struct SpriteFrameTable_MM6 {
    uint32_t frameCount;
    uint32_t eframeCount;
    std::vector<SpriteFrame_MM6> frames;
    std::vector<uint16_t> eframes;
};

void reconstruct(const SpriteFrameTable_MM6 &src, SpriteFrameTable *dst);
void deserialize(InputStream &src, SpriteFrameTable_MM6 *dst);
