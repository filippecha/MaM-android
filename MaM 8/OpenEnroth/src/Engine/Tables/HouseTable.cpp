#include "HouseTable.h"

#include <array>
#include <map>
#include <string>
#include <utility>

#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Data/HouseEnums.h"
#include "Engine/MapEnumFunctions.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/GameVariant.h"
#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;

template<class T>
static T parseNumberOr(std::string_view s, T fallback) {
    s = trim(s);
    T result = fallback;
    if (s.empty())
        return fallback;
    return tryDeserialize(s, &result) ? result : fallback;
}

/**
 * @param name                          House type from MM6 or MM8 2DEvents.txt, e.g. "General Store".
 * @param houseTypeMap                  Types shared with MM7.
 * @return                              House type. Types that MM7 doesn't have map to the MM7 type that behaves the
 *                                      same, residences to `HOUSE_TYPE_HOUSE`.
 */
static HouseType mm6Or8HouseTypeFromName(std::string_view name, const std::map<std::string, HouseType, ascii::NoCaseLess> &houseTypeMap) {
    static const std::map<std::string, HouseType, ascii::NoCaseLess> mm6Types = {
        {"Elemental Guild", HOUSE_TYPE_ELEMENTAL_GUILD}, // MM8.
        {"The Adventurer's Inn", HOUSE_TYPE_ADVENTURERS_INN}, // MM8.
        {"Seer", HOUSE_TYPE_SEER}, // MM8.
        {"General Store", HOUSE_TYPE_ALCHEMY_SHOP},
        {"Merc Guild", HOUSE_TYPE_MERCENARY_GUILD},
        {"Thieves Guild", HOUSE_TYPE_SHADOW_GUILD},
        {"Town Hall", HOUSE_TYPE_TOWN_HALL},
        {"City Council", HOUSE_TYPE_TOWN_HALL},
        {"Castle Entrance", HOUSE_TYPE_CASTLE},
        {"Throne", HOUSE_TYPE_THRONE_ROOM},
        {"Jail", HOUSE_TYPE_JAIL},
        {"Circus", HOUSE_TYPE_CIRCUS},
        {"Tent", HOUSE_TYPE_CIRCUS},
        {"Wagon", HOUSE_TYPE_CIRCUS},
        {"The Seer", HOUSE_TYPE_SEER},
        {"The Oracle", HOUSE_TYPE_SEER},
    };
    std::string trimmed = std::string(trim(name));
    if (auto it = mm6Types.find(trimmed); it != mm6Types.end())
        return it->second;
    if (auto it = houseTypeMap.find(trimmed); it != houseTypeMap.end())
        return it->second;
    if (trimmed.ends_with(" Ent"))
        return HOUSE_TYPE_DUNGEON;
    return HOUSE_TYPE_HOUSE;
}

/**
 * MM6 2devents.txt has the same leading columns as MM7, but "A"/"B"/"C" hold free-form notes about shop stock,
 * guild spells and stable routes, and exits are given as text ("Dungeon", "2D 154").
 *
 * @param houses                        2devents.txt contents.
 * @param houseTypeMap                  House type name to house type map.
 */
static void initializeHousesMm6(const Blob &houses, const std::map<std::string, HouseType, ascii::NoCaseLess> &houseTypeMap) {
    for (std::string_view line : split(houses.str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 25> tokens = split(line).by('\t').resize(25, "");
        int id = parseNumberOr<int>(tokens[0], 0);
        if (id < std::to_underlying(HOUSE_FIRST) || id > std::to_underlying(HOUSE_LAST))
            continue;

        HouseData &house = houseTable[static_cast<HouseId>(id)];
        house.uType = mm6Or8HouseTypeFromName(tokens[2], houseTypeMap);
        setMm6HouseType(static_cast<HouseId>(id), house.uType);
        house.uAnimationID = parseNumberOr<int>(tokens[4], 0);
        house.name = unquote(tokens[5]);
        house.pProprieterName = trim(unquote(tokens[6]));
        house.pProprieterTitle = unquote(tokens[7]);
        house.fPriceMultiplier = parseNumberOr<float>(tokens[12], 1.0f);
        house.flt_24 = house.fPriceMultiplier; // MM6 uses the same multiplier for goods and for skills & spells.
        house.generation_interval_days = parseNumberOr<int>(tokens[15], 0);
        house.uOpenTime = parseNumberOr<int>(tokens[18], 0);
        house.uCloseTime = parseNumberOr<int>(tokens[19], 0);
        house.uExitPicID = parseNumberOr<int>(tokens[20], 0);
        house.uExitMapID = mapIdFromMm6(parseNumberOr<int>(tokens[21], 0));
        house._quest_bit = static_cast<QuestBit>(parseNumberOr<int>(tokens[22], 0));
        house.enterText = unquote(tokens[23]);
        house.exitDeniedText = unquote(tokens[24]);
    }
}

void initializeHouses(const Blob &houses) {
    // 2devents.txt table structure (column names are the headers from the data file):
    //  0: "#"                  - house id
    //  1: "#"                  - per-type sequence number, resets at each new Type         (not used)
    //  2: "Type"               - house type                                                (not localized)
    //  3: "Map"                - map id this building lives on                             (not used)
    //  4: "Picture"            - index into `pAnimatedRooms`, for npc id, video & sound
    //  5: "Name"               - house name                                                (localized)
    //  6: "Proprietor Name"                                                                (localized)
    //  7: "Proprietor Title"                                                               (localized)
    //  8: "Picture"            - always 0                                                  (not used in MM7)
    //  9: "State"              - always 0                                                  (not used in MM7)
    // 10: "Rep"                - always 0, reputation?                                     (not used in MM7)
    // 11: "Per"                - always 0                                                  (not used in MM7)
    // 12: "Val"                - shop price multiplier, float
    // 13: "A"                  - skill/spell price multiplier, float
    // 14: "B"                  - always empty
    // 15: "C"                  - item-generation interval, days
    // 16: "Notes:"             - mostly empty, an alternative index into `pAnimatedRooms`,
    //                            points at the base entry of a race-tier triplet           (not used)
    // 17: "Notes(2):"          - max trainable level for Training houses                   (not used)
    // 18: "Open"               - opening hour, 0-24
    // 19: "Closed"             - closing hour, 0-24
    // 20: "Pic"                - exit picture id                                           (not used in MM7)
    // 21: "Map"                - exit map id                                               (not used in MM7)
    // 22: "Restrictions"       - exit gating quest bit                                     (not used in MM7)
    // 23: "Text"               - exit text                                                 (not used in MM7)
    static const std::map<std::string, HouseType, ascii::NoCaseLess> houseTypeMap = {
        {"Weapon Shop", HOUSE_TYPE_WEAPON_SHOP},
        {"Armor Shop", HOUSE_TYPE_ARMOR_SHOP},
        {"Magic Shop", HOUSE_TYPE_MAGIC_SHOP},
        {"Alchemist", HOUSE_TYPE_ALCHEMY_SHOP},
        {"Stables", HOUSE_TYPE_STABLE},
        {"Boats", HOUSE_TYPE_BOAT},
        {"Temple", HOUSE_TYPE_TEMPLE},
        {"Training", HOUSE_TYPE_TRAINING_GROUND},
        {"Town Hall", HOUSE_TYPE_TOWN_HALL},
        {"Tavern", HOUSE_TYPE_TAVERN},
        {"Bank", HOUSE_TYPE_BANK},
        {"Fire Guild", HOUSE_TYPE_FIRE_GUILD},
        {"Air Guild", HOUSE_TYPE_AIR_GUILD},
        {"Water Guild", HOUSE_TYPE_WATER_GUILD},
        {"Earth Guild", HOUSE_TYPE_EARTH_GUILD},
        {"Spirit Guild", HOUSE_TYPE_SPIRIT_GUILD},
        {"Mind Guild", HOUSE_TYPE_MIND_GUILD},
        {"Body Guild", HOUSE_TYPE_BODY_GUILD},
        {"Light Guild", HOUSE_TYPE_LIGHT_GUILD},
        {"Dark Guild", HOUSE_TYPE_DARK_GUILD},
        {"Element Guild", HOUSE_TYPE_ELEMENTAL_GUILD}, // This is MM6 only.
        {"Self Guild", HOUSE_TYPE_SELF_GUILD},
        {"Mirrored Path Guild", HOUSE_TYPE_MIRRORED_PATH_GUILD},
        {"Mercenary Guild", HOUSE_TYPE_TOWN_HALL}, // This is MM6 only. TODO(captainurist): Is this right and not Merc Guild (18)?
    };

    if (isMm6()) {
        initializeHousesMm6(houses, houseTypeMap);
        return;
    }

    for (std::string_view line : split(houses.str()).by("\r\n").drop(2).skip("")) {
        // Some lines have only ~12 cols, and some cols are empty, so need both resize & replace.
        std::array<std::string_view, 24> tokens = split(line).by('\t').replace("", "0").resize(24, "0");
        if (isMm8())
            for (std::string_view &token : tokens)
                if (trim(token).empty())
                    token = "0"; // MM8 has cells with just spaces.

        // TODO(captainurist): We don't check if int is in range. A better way would be to deal away with enums
        //                     entirely, and just use typed ids. Do this once we iron out the details of how #mm6
        //                     enums will be handled by the engine. Also apply to other table parsers.
        HouseId houseId = static_cast<HouseId>(fromString<int>(tokens[0]));
        houseTable[houseId].uType = isMm8() ? mm6Or8HouseTypeFromName(tokens[2], houseTypeMap) : valueOr(houseTypeMap, tokens[2], HOUSE_TYPE_MERCENARY_GUILD);
        houseTable[houseId].uAnimationID = fromString<int>(tokens[4]);
        houseTable[houseId].name = unquote(tokens[5]);
        houseTable[houseId].pProprieterName = unquote(tokens[6]);
        houseTable[houseId].pProprieterTitle = unquote(tokens[7]);
        houseTable[houseId].fPriceMultiplier = fromString<float>(tokens[12]);
        houseTable[houseId].flt_24 = fromString<float>(tokens[13]);
        houseTable[houseId].generation_interval_days = fromString<int>(tokens[15]);
        houseTable[houseId].uOpenTime = fromString<int>(tokens[18]);
        houseTable[houseId].uCloseTime = fromString<int>(tokens[19]);
        houseTable[houseId].uExitPicID = fromString<int>(tokens[20]);
        houseTable[houseId].uExitMapID = static_cast<MapId>(fromString<int>(tokens[21]));
        houseTable[houseId]._quest_bit = static_cast<QuestBit>(fromString<int>(tokens[22]));
    }
}
