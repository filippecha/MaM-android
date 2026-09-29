#include "ResourceManager.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "Library/Logger/Logger.h"

#include "Utility/GameVariant.h"
#include "Utility/String/Ascii.h"

#include "EngineFileSystem.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    // MM6 keeps event scripts and text tables in icons.lod, MM8 in EnglishT.lod.
    _eventsLodReader.open(dfs->read(isMm6() ? "data/icons.lod" : isMm8() ? "data/englisht.lod" : "data/events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::eventsData(std::string_view filename) {
    if ((isMm6() || isMm8()) && !_eventsLodReader.exists(filename)) {
        MM_WARNING("{} data has no '{}', using an empty table", isMm6() ? "MM6" : "MM8", filename); // E.g. MM6 placemon.txt, MM8 npcdist.txt.
        return Blob::fromString("");
    }

    Blob result = lod::decodeMaybeCompressed(_eventsLodReader.read(filename));

    bool isText = filename.size() >= 4 && ascii::noCaseEquals(filename.substr(filename.size() - 4), ".txt");
    if (isText && result.size() > 0 && result.str().back() == '\x1a')
        return result.subBlob(0, result.size() - 1); // Some localized tables end with a DOS EOF marker.

    return result;
}
