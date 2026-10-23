#include "ResourceManager.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/String/Ascii.h"

#include "EngineFileSystem.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    _hasKraj = dfs->exists("data/kraj.lod");
    if (_hasKraj)
        _krajLodReader.open(dfs->read("data/kraj.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::krajData(std::string_view filename) {
    if (!_hasKraj || !_krajLodReader.exists(filename))
        return Blob();
    return lod::decodeMaybeCompressed(_krajLodReader.read(filename));
}

Blob ResourceManager::eventsData(std::string_view filename) {
    const LodReader &reader = _hasKraj && _krajLodReader.exists(filename) ? _krajLodReader : _eventsLodReader;
    Blob result = lod::decodeMaybeCompressed(reader.read(filename));

    bool isText = filename.size() >= 4 && ascii::noCaseEquals(filename.substr(filename.size() - 4), ".txt");
    if (isText && result.size() > 0 && result.str().back() == '\x1a')
        return result.subBlob(0, result.size() - 1); // Some localized tables end with a DOS EOF marker.

    return result;
}
