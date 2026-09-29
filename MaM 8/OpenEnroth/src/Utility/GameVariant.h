#pragma once

/**
 * Which game the loaded data belongs to. MM6, MM7 and MM8 share most of the engine and the data formats,
 * the places that differ check this. Set once at startup from the files found in the data folder.
 */
enum class GameVariant {
    MM6,
    MM7,
    MM8,
};

extern GameVariant gameVariant;

inline bool isMm6() {
    return gameVariant == GameVariant::MM6;
}

inline bool isMm8() {
    return gameVariant == GameVariant::MM8;
}
