#include "Mm6Potions.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

#include "Engine/Objects/Character.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Party.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

namespace mm6_potions {

namespace {

constexpr int ITEM_COUNT = LAST_ITEM - FIRST_ITEM + 1;
constexpr int FIRST_SPARE_BOTTLE = 189; // Items 189-196 are bottles too.
constexpr int LAST_SPARE_BOTTLE = 196;
const Duration buffDuration = Duration::fromHours(6);

std::array<std::array<int, ITEM_COUNT>, ITEM_COUNT> mixTable = {};

int normalized(int mm6Id) {
    return mm6Id >= FIRST_SPARE_BOTTLE && mm6Id <= LAST_SPARE_BOTTLE ? BOTTLE : mm6Id;
}

void setStatBonuses(Character *character, int value) {
    for (Attribute stat : character->_statBonuses.indices())
        character->_statBonuses[stat] = value;
}

void setResistanceBonuses(Character *character, int value) {
    // MM6 fire, electricity, cold, poison and magic.
    character->sResFireBonus = character->sResAirBonus = character->sResWaterBonus = character->sResBodyBonus = character->sResMindBonus = value;
}

/**
 * @param character                     Character.
 * @param stat                          Stat to lower, it doesn't go below 1.
 * @param amount                        How much to lower it by.
 */
void lowerStat(Character *character, Attribute stat, int amount) {
    character->_stats[stat] = std::max(1, character->_stats[stat] - amount);
}

/**
 * @param character                     Character that drinks a pure stat potion.
 * @param stat                          Stat that the potion raises by 15, once per character.
 * @param loweredStat                   Stat that it lowers by 5.
 */
void drinkPureStat(Character *character, Attribute stat, Attribute loweredStat) {
    if (character->_pureStatPotionUsed[stat])
        return;
    character->_pureStatPotionUsed[stat] = true;
    character->_stats[stat] += 15;
    lowerStat(character, loweredStat, 5);
}

void applyBuff(Character *character, CharacterBuff buff) {
    character->pCharacterBuffs[buff].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, -1);
}

} // namespace

void loadMixTable(const Blob &useItems) {
    // useitems.txt: item id | name | english name | kind | effect | change item | a column for every item 160-188.
    for (std::string_view line : split(useItems.str()).by("\r\n")) {
        std::array<std::string_view, 6 + ITEM_COUNT> tokens = split(line).by('\t').resize(6 + ITEM_COUNT, "");
        std::string_view key = trim(tokens[0]);
        if (key.empty() || key[0] < '0' || key[0] > '9')
            continue;
        int row = fromString<int>(key);
        if (row < FIRST_ITEM || row > LAST_ITEM)
            continue;
        for (int column = 0; column < ITEM_COUNT; column++) {
            std::string_view cell = trim(tokens[6 + column]);
            int value = 0;
            if (!cell.empty() && (cell[0] == 'E' || cell[0] == 'e'))
                value = fromString<int>(cell.substr(1));
            else if (!cell.empty() && cell[0] >= '0' && cell[0] <= '9')
                value = fromString<int>(cell);
            mixTable[row - FIRST_ITEM][column] = value;
        }
    }
}

int mix(int held, int target) {
    held = normalized(held);
    target = normalized(target);
    if (held == BOTTLE || held < FIRST_ITEM || held > LAST_ITEM || target < FIRST_ITEM || target > LAST_ITEM)
        return 0;
    return mixTable[held - FIRST_ITEM][target - FIRST_ITEM];
}

bool drink(Character *character, int mm6Id) {
    switch (mm6Id) {
    case 160: // Poppysnaps.
        if (!character->conditions.has(CONDITION_POISON_WEAK))
            character->conditions.set(CONDITION_POISON_WEAK, pParty->GetPlayingTime());
        return true;
    case 161: // Phirna root.
        character->mana = std::min(character->mana + 2, character->GetMaxMana());
        return true;
    case 162: // Widoweeps berries.
        character->Heal(2);
        return true;
    case 164: // Cure wounds.
        character->Heal(10);
        return true;
    case 165: // Magic potion.
        character->mana = std::min(character->mana + 10, character->GetMaxMana());
        return true;
    case 166: // Energy.
        setStatBonuses(character, 10);
        return true;
    case 167: // Protection.
        character->sACModifier = 10;
        return true;
    case 168: // Resistance.
        setResistanceBonuses(character, 10);
        return true;
    case 169: // Cure poison.
        character->conditions.reset(CONDITION_POISON_WEAK);
        character->conditions.reset(CONDITION_POISON_MEDIUM);
        character->conditions.reset(CONDITION_POISON_SEVERE);
        return true;
    case 170: // Supreme protection.
        character->sACModifier = 20;
        return true;
    case 171: // Restoration, cures everything but death, stone and eradication.
        for (Condition condition : allConditions())
            if (condition != CONDITION_DEAD && condition != CONDITION_PETRIFIED && condition != CONDITION_ERADICATED)
                character->conditions.reset(condition);
        return true;
    case 172: // Extreme energy.
        setStatBonuses(character, 20);
        return true;
    case 173: // Super resistance.
        setResistanceBonuses(character, 20);
        return true;
    case 174:
        applyBuff(character, CHARACTER_BUFF_HEROISM);
        return true;
    case 175:
        applyBuff(character, CHARACTER_BUFF_HASTE);
        return true;
    case 176:
        applyBuff(character, CHARACTER_BUFF_STONESKIN);
        return true;
    case 177:
        applyBuff(character, CHARACTER_BUFF_BLESS);
        return true;
    case 178: // Divine power.
        character->sAgeModifier++;
        character->sLevelModifier = 20;
        return true;
    case 179: // Divine cure, can go over the maximum once.
        if (character->health <= character->GetMaxHealth()) {
            character->health += 100;
            if (character->health > 0)
                character->conditions.reset(CONDITION_UNCONSCIOUS);
            character->sAgeModifier++;
        }
        return true;
    case 180: // Divine magic, can go over the maximum once.
        if (character->mana <= character->GetMaxMana()) {
            character->mana += 100;
            character->sAgeModifier++;
        }
        return true;
    case 181:
        drinkPureStat(character, ATTRIBUTE_MIGHT, ATTRIBUTE_INTELLIGENCE);
        return true;
    case 182:
        drinkPureStat(character, ATTRIBUTE_INTELLIGENCE, ATTRIBUTE_MIGHT);
        return true;
    case 183:
        drinkPureStat(character, ATTRIBUTE_PERSONALITY, ATTRIBUTE_SPEED);
        return true;
    case 184: // Pure endurance lowers every other stat by 1.
        if (!character->_pureStatPotionUsed[ATTRIBUTE_ENDURANCE]) {
            character->_pureStatPotionUsed[ATTRIBUTE_ENDURANCE] = true;
            character->_stats[ATTRIBUTE_ENDURANCE] += 15;
            for (Attribute stat : character->_stats.indices())
                if (stat != ATTRIBUTE_ENDURANCE)
                    lowerStat(character, stat, 1);
        }
        return true;
    case 185:
        drinkPureStat(character, ATTRIBUTE_ACCURACY, ATTRIBUTE_LUCK);
        return true;
    case 186:
        drinkPureStat(character, ATTRIBUTE_SPEED, ATTRIBUTE_PERSONALITY);
        return true;
    case 187:
        drinkPureStat(character, ATTRIBUTE_LUCK, ATTRIBUTE_ACCURACY);
        return true;
    case 188: // Rejuvenation.
        character->sAgeModifier = 0;
        for (Attribute stat : character->_stats.indices())
            lowerStat(character, stat, 1);
        return true;
    default:
        return false;
    }
}

} // namespace mm6_potions
