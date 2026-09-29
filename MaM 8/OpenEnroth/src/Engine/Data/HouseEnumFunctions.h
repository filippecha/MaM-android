#pragma once

#include <utility>

#include "AwardEnums.h"
#include "HouseEnums.h"

#include "Utility/GameVariant.h"
#include "Utility/Segment.h"

/**
 * MM6 numbers its houses differently from MM7, so in MM6 the house checks below look at the house type instead of
 * the house id. The types are set when the house table is loaded.
 *
 * @param houseId                       House.
 * @param type                          Type of the house as given by MM6 2DEvents.txt.
 */
void setMm6HouseType(HouseId houseId, HouseType type);

HouseType mm6HouseType(HouseId houseId);

/**
 * @param houseId                       MM6 house.
 * @param award                         Award that MM6 requires to enter the house, `AWARD_INVALID` for none.
 */
void setMm6GuildAward(HouseId houseId, AwardId award);

AwardId mm6GuildAward(HouseId houseId);

inline Segment<HouseId> allTownhallHouses() {
    return {HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL};
}

inline Segment<HouseId> allHouses() {
    return {HOUSE_FIRST, HOUSE_LAST};
}

inline Segment<HouseId> allArcomageTaverns() {
    return {HOUSE_FIRST_ARCOMAGE_TAVERN, HOUSE_LAST_ARCOMAGE_TAVERN};
}

inline bool isShop(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) >= HOUSE_TYPE_WEAPON_SHOP && mm6HouseType(houseId) <= HOUSE_TYPE_ALCHEMY_SHOP;
    return houseId >= HOUSE_FIRST_SHOP && houseId <= HOUSE_LAST_SHOP;
}

inline bool isWeaponShop(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_WEAPON_SHOP;
    return houseId >= HOUSE_FIRST_WEAPON_SHOP && houseId <= HOUSE_LAST_WEAPON_SHOP;
}

inline bool isArmorShop(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_ARMOR_SHOP;
    return houseId >= HOUSE_FIRST_ARMOR_SHOP && houseId <= HOUSE_LAST_ARMOR_SHOP;
}

inline bool isMagicShop(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_MAGIC_SHOP;
    return houseId >= HOUSE_FIRST_MAGIC_SHOP && houseId <= HOUSE_LAST_MAGIC_SHOP;
}

inline bool isAlchemyShop(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_ALCHEMY_SHOP;
    return houseId >= HOUSE_FIRST_ALCHEMY_SHOP && houseId <= HOUSE_LAST_ALCHEMY_SHOP;
}

inline bool isMagicGuild(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) >= HOUSE_TYPE_FIRE_GUILD && mm6HouseType(houseId) <= HOUSE_TYPE_SELF_GUILD;
    return houseId >= HOUSE_FIRST_MAGIC_GUILD && houseId <= HOUSE_LAST_MAGIC_GUILD;
}

inline bool isStable(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_STABLE;
    return houseId >= HOUSE_FIRST_STABLE && houseId <= HOUSE_LAST_STABLE;
}

inline bool isBoat(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_BOAT;
    return houseId >= HOUSE_FIRST_BOAT && houseId <= HOUSE_LAST_BOAT;
}

inline bool isTavern(HouseId houseId) {
    if (isMm6())
        return mm6HouseType(houseId) == HOUSE_TYPE_TAVERN;
    return houseId >= HOUSE_FIRST_TAVERN && houseId <= HOUSE_LAST_TAVERN;
}

inline constexpr int MM8_FIRST_TAVERN = 107;
inline constexpr int MM8_TAVERN_COUNT = 11;

inline bool isArcomageTavern(HouseId houseId) {
    if (isMm6())
        return false;
    if (isMm8())
        return std::to_underlying(houseId) >= MM8_FIRST_TAVERN && std::to_underlying(houseId) < MM8_FIRST_TAVERN + MM8_TAVERN_COUNT;
    return houseId >= HOUSE_FIRST_ARCOMAGE_TAVERN && houseId <= HOUSE_LAST_ARCOMAGE_TAVERN;
}

/**
 * MM8.exe 0x4F4D60 has the Arcomage rules of the first eleven MM7 taverns, in the same order from its tavern 107 on.
 *
 * @param houseId                       Arcomage tavern of the loaded game.
 * @return                              MM7 tavern whose rules and win flag the tavern uses.
 */
inline HouseId arcomageRulesTavern(HouseId houseId) {
    assert(isArcomageTavern(houseId));
    if (isMm8())
        return static_cast<HouseId>(std::to_underlying(HOUSE_FIRST_ARCOMAGE_TAVERN) + std::to_underlying(houseId) - MM8_FIRST_TAVERN);
    return houseId;
}

inline int arcomageTopicForTavern(HouseId houseId) {
    assert(isArcomageTavern(houseId));
    if (isMm8())
        return std::to_underlying(houseId) - MM8_FIRST_TAVERN + 136;
    return std::to_underlying(houseId) - std::to_underlying(HOUSE_FIRST_ARCOMAGE_TAVERN) + 355;
}

inline int arcomageRulesTopic() {
    return isMm8() ? 135 : 354;
}

AwardId membershipAwardForGuild(HouseId houseId);

AwardId membershipAwardForGuild(GuildId guildId);
