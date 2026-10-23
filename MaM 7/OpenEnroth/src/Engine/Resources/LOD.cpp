#include "LOD.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "EngineFileSystem.h"

std::unique_ptr<GamesLod> pGames_LOD;

bool GamesLod::exists(std::string_view filename) const {
    return (kraj && kraj->exists(filename)) || base.exists(filename);
}

Blob GamesLod::read(std::string_view filename) const {
    if (kraj && kraj->exists(filename))
        return kraj->read(filename);
    return base.read(filename);
}

std::vector<std::string> GamesLod::ls() const {
    std::vector<std::string> result = base.ls();
    if (kraj)
        for (const std::string &name : kraj->ls())
            if (std::ranges::find(result, name) == result.end())
                result.push_back(name);
    return result;
}

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = std::make_unique<GamesLod>();
    pGames_LOD->base.open(dfs->read("data/games.lod"));
    if (dfs->exists("data/kraj.lod"))
        pGames_LOD->kraj = std::make_unique<LodReader>(dfs->read("data/kraj.lod"));
    return true;
}
