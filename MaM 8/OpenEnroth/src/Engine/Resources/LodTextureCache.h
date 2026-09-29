#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Library/Lod/LodReader.h"

#include "Utility/Memory/Blob.h"

struct LodImage;
class LodReader;

class LodTextureCache {
 public:
    LodTextureCache();
    ~LodTextureCache();

    void open(Blob blob);

    /**
     * Adds a LOD that is looked into before the one passed to `open`.
     *
     * @param blob                      Contents of the LOD file, e.g. the language LOD of MM8.
     */
    void openOverride(Blob blob);

    void reserveLoadedTextures();
    void releaseUnreserved();

    LodImage *loadTexture(std::string_view pContainer, bool useDummyOnError = true);

    Blob LoadCompressedTexture(std::string_view pContainer); // TODO(captainurist): doesn't belong here.
    Blob read(std::string_view pContainer); // TODO(captainurist): doesn't belong here.

 private:
    bool LoadTextureFromLOD(LodImage *pOutTex, std::string_view pContainer);
    const LodReader &readerFor(std::string_view pContainer) const;

 private:
    LodReader _reader;
    std::unique_ptr<LodReader> _overrideReader;
    int _reservedCount = 0;
    std::unordered_map<std::string, LodImage> _textureByName;
    std::vector<std::string> _texturesInOrder;
};

extern LodTextureCache *pIcons_LOD;
extern LodTextureCache *pIcons_LOD_mm6;
extern LodTextureCache *pIcons_LOD_mm8;

extern LodTextureCache *pBitmaps_LOD;
extern LodTextureCache *pBitmaps_LOD_mm6;
extern LodTextureCache *pBitmaps_LOD_mm8;
