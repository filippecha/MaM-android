#pragma once

#include <string_view>

#include "Utility/Memory/Blob.h"

#include "Library/Lod/LodReader.h"

/**
 * This class provides access to everything in `/data` folder.
 */
class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    void open();

    Blob eventsData(std::string_view filename);

    /**
     * @return                          Table or script of the "Stín nad Krajem" expansion from data/kraj.lod, or an
     *                                  empty blob if the expansion or the file isn't there.
     */
    Blob krajData(std::string_view filename);

 private:
    LodReader _eventsLodReader;
    LodReader _krajLodReader;
    bool _hasKraj = false;
};
