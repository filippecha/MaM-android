#pragma once

#include <utility>

#include "MapEnums.h"

#include "Utility/GameVariant.h"
#include "Utility/Segment.h"

inline constexpr MapId MAP_LAST_OUTDOOR_MM6 = static_cast<MapId>(115); // OutA1-OutE3 come first in MM6 mapstats.txt.
inline constexpr MapId MAP_LAST_OUTDOOR_MM8 = static_cast<MapId>(215); // Out01-Out15 come first in MM8 mapstats.txt...
inline constexpr MapId MAP_ELEMENTAL_EARTH_MM8 = static_cast<MapId>(210); // ...except ElemE.blv, the plane of earth.

/**
 * Is map an outdoor map?
 */
inline bool isMapOutdoor(MapId mapid) {
    if (isMm6())
        return mapid >= MAP_FIRST_MM6 && mapid <= MAP_LAST_OUTDOOR_MM6;
    if (isMm8())
        return mapid >= MAP_FIRST_MM8 && mapid <= MAP_LAST_OUTDOOR_MM8 && mapid != MAP_ELEMENTAL_EARTH_MM8;
    return mapid >= MAP_EMERALD_ISLAND && mapid <= MAP_SHOALS && mapid != MAP_PIT && mapid != MAP_CELESTE;
}

/**
 * Is map an indoor map?
 */
inline bool isMapIndoor(MapId mapid) {
    if (isMm6())
        return mapid > MAP_LAST_OUTDOOR_MM6 && mapid <= MAP_LAST_MM6;
    if (isMm8())
        return mapid == MAP_ELEMENTAL_EARTH_MM8 || (mapid > MAP_LAST_OUTDOOR_MM8 && mapid <= MAP_LAST_MM8);
    return (mapid >= MAP_DRAGON_CAVES && mapid <= MAP_ARENA) || mapid == MAP_PIT || mapid == MAP_CELESTE;
}

/**
 * Is map an outdoor underwater map (requires wetsuit etc.)?
 */
inline bool isMapUnderwater(MapId mapid) {
    return mapid == MAP_SHOALS;
}

/**
 * Is hirelings interactions are forbidden on this map?
 */
inline bool isHirelingsBlockedOnMap(MapId mapid) {
    return (mapid == MAP_SHOALS) || (mapid == MAP_LINCOLN);
}

inline Segment<MapId> allMaps() {
    if (isMm6())
        return {MAP_FIRST_MM6, MAP_LAST_MM6};
    if (isMm8())
        return {MAP_FIRST_MM8, MAP_LAST_MM8};
    return {MAP_FIRST, MAP_LAST};
}

/**
 * @param mm6Id                         Map id from MM6 mapstats.txt, 1-based.
 * @return                              Engine map id, `MAP_INVALID` for ids outside of the MM6 range.
 */
inline MapId mapIdFromMm6(int mm6Id) {
    if (mm6Id <= 0 || mm6Id > std::to_underlying(MAP_LAST_MM6) - std::to_underlying(MAP_FIRST_MM6) + 1)
        return MAP_INVALID;
    return static_cast<MapId>(std::to_underlying(MAP_FIRST_MM6) + mm6Id - 1);
}

/**
 * @param mm8Id                         Map id from MM8 mapstats.txt, 1-based.
 * @return                              Engine map id, `MAP_INVALID` for ids outside of the MM8 range.
 */
inline MapId mapIdFromMm8(int mm8Id) {
    if (mm8Id <= 0 || mm8Id > std::to_underlying(MAP_LAST_MM8) - std::to_underlying(MAP_FIRST_MM8) + 1)
        return MAP_INVALID;
    return static_cast<MapId>(std::to_underlying(MAP_FIRST_MM8) + mm8Id - 1);
}

inline bool isArenaMap(MapId map) {
    if (isMm8())
        return map == mapIdFromMm8(53); // MM8 d42.blv.
    return isMm6() ? map == mapIdFromMm6(52) : map == MAP_ARENA; // MM6 zarena.blv.
}
