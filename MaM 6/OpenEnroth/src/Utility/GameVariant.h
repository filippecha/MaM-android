#pragma once

/**
 * Which game the loaded data belongs to. MM6 and MM7 share most of the engine and the data formats,
 * the places that differ check this. Set once at startup from the files found in the data folder.
 */
enum class GameVariant {
    MM6,
    MM7,
};

extern GameVariant gameVariant;

inline bool isMm6() {
    return gameVariant == GameVariant::MM6;
}
