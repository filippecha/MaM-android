#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Library/Lod/LodReader.h"

class FileSystem;

bool Initialize_GamesLOD_NewLOD();

/**
 * Maps from data/games.lod, with the maps of the MaM 7 expansion "Stín nad Krajem" from data/kraj.lod (if present)
 * taking precedence.
 */
class GamesLod {
 public:
    [[nodiscard]] bool exists(std::string_view filename) const;
    [[nodiscard]] Blob read(std::string_view filename) const;
    [[nodiscard]] std::vector<std::string> ls() const;

    LodReader base;
    std::unique_ptr<LodReader> kraj;
};

extern std::unique_ptr<GamesLod> pGames_LOD;
