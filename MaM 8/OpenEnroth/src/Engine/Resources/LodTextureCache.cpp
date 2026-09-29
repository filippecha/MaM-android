#include "LodTextureCache.h"

#include <memory>
#include <utility>
#include <string>

#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

LodTextureCache *pIcons_LOD = nullptr;
LodTextureCache *pIcons_LOD_mm6 = nullptr;
LodTextureCache *pIcons_LOD_mm8 = nullptr;

LodTextureCache *pBitmaps_LOD = nullptr;
LodTextureCache *pBitmaps_LOD_mm6 = nullptr;
LodTextureCache *pBitmaps_LOD_mm8 = nullptr;

LodTextureCache::LodTextureCache() = default;
LodTextureCache::~LodTextureCache() = default;

void LodTextureCache::open(Blob blob) {
    _reader.open(std::move(blob));
}

void LodTextureCache::openOverride(Blob blob) {
    _overrideReader = std::make_unique<LodReader>();
    _overrideReader->open(std::move(blob));
}

const LodReader &LodTextureCache::readerFor(std::string_view pContainer) const {
    return _overrideReader && _overrideReader->exists(pContainer) ? *_overrideReader : _reader;
}

void LodTextureCache::reserveLoadedTextures() {
    _reservedCount = _texturesInOrder.size();
}

void LodTextureCache::releaseUnreserved() {
    while (_texturesInOrder.size() > _reservedCount) {
        const std::string &name = _texturesInOrder.back();
        _textureByName.erase(name);
        _texturesInOrder.pop_back();
    }
}

LodImage *LodTextureCache::loadTexture(std::string_view pContainer, bool useDummyOnError) {
    std::string name = ascii::toLower(pContainer);

    LodImage *result = valuePtr(_textureByName, name);
    if (result)
        return result;

    result = &_textureByName[name];
    if (LoadTextureFromLOD(result, name)) {
        _texturesInOrder.push_back(name);
        return result;
    }
    _textureByName.erase(name);

    if (useDummyOnError) {
        return loadTexture("pending", false);
    } else {
        return nullptr;
    }
}

Blob LodTextureCache::LoadCompressedTexture(std::string_view pContainer) {
    return lod::decodeMaybeCompressed(readerFor(pContainer).read(pContainer));
}

Blob LodTextureCache::read(std::string_view pContainer) {
    return readerFor(pContainer).read(pContainer);
}

bool LodTextureCache::LoadTextureFromLOD(LodImage *pOutTex, std::string_view pContainer) {
    const LodReader &reader = readerFor(pContainer);
    if (!reader.exists(pContainer))
        return false;

    *pOutTex = lod::decodeImage(reader.read(pContainer));
    return true;
}
