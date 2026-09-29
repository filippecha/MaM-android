#include "Mm8Roster.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Objects/Character.h"
#include "Engine/Objects/Mm8Ids.h"
#include "Engine/Party.h"
#include "Engine/Tables/ItemTable.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

namespace {

constexpr int MM8_ROSTER_ITEM_COUNT = 10; // The roster.txt header names eleven equipment columns, the lines have ten.
constexpr int MM8_ROSTER_SPELL_SCHOOL_COUNT = 12;
constexpr int MM8_SPELLS_PER_SCHOOL = 11;
constexpr int MM8_FIRST_ROSTER_QUEST_BIT = 400; // Set for roster characters that the Adventurer's Inn offers.

struct RosterLine {
    std::string name;
    int mm8Class = 0;
    int birthYear = 0;
    int face = 0;
    int voice = 0;
    int64_t experience = 0;
    int level = 1;
    std::array<int, 7> stats = {}; // Might, intellect, personality, endurance, speed, accuracy, luck.
    std::array<int, 6> resistances = {}; // Fire, air, water, earth, mind, body.
    int skillPoints = 0;
    std::array<CombinedSkillValue, MM8_SKILL_COUNT> skills;
    std::array<int, MM8_ROSTER_SPELL_SCHOOL_COUNT> spellCounts = {};
    std::array<int, MM8_ROSTER_ITEM_COUNT> items = {}; // MM8 item id + 1000 * treasure level.
    std::string blurb;
};

std::vector<RosterLine> rosterLines;

int toInt(std::string_view s) {
    s = trim(s);
    if (s.empty() || !(s[0] == '-' || (s[0] >= '0' && s[0] <= '9')))
        return 0;
    int64_t result = 0;
    bool negative = s[0] == '-';
    for (char c : s.substr(negative ? 1 : 0)) {
        if (c < '0' || c > '9')
            break;
        result = result * 10 + (c - '0');
    }
    return static_cast<int>(negative ? -result : result);
}

int64_t toInt64(std::string_view s) {
    int64_t result = 0;
    for (char c : trim(s)) {
        if (c < '0' || c > '9')
            break;
        result = result * 10 + (c - '0');
    }
    return result;
}

/**
 * @param name                          Class name from roster.txt.
 * @return                              MM8 class id, MM8.exe 0x496CAE.
 */
int mm8ClassFromName(std::string_view name) {
    static constexpr std::array<std::pair<std::string_view, int>, 19> classes = {{
        {"necromancer", 0}, {"necromancer2", 1}, {"lich", 1}, {"cleric", 2}, {"cleric2", 3}, {"priestofsun", 3},
        {"knight", 4}, {"knight2", 5}, {"champion", 5}, {"troll", 6}, {"troll2", 7}, {"minotaur", 8},
        {"minotaur2", 9}, {"darkelf", 10}, {"darkelf2", 11}, {"vampire", 12}, {"vampire2", 13}, {"dragon", 14},
        {"dragon2", 15},
    }};
    for (const auto &[className, id] : classes)
        if (ascii::noCaseEquals(trim(name), className))
            return id;
    return 0;
}

Mastery masteryFromLetter(std::string_view letter) {
    letter = trim(letter);
    if (ascii::noCaseEquals(letter, "G"))
        return MASTERY_GRANDMASTER;
    if (ascii::noCaseEquals(letter, "M"))
        return MASTERY_MASTER;
    if (ascii::noCaseEquals(letter, "E"))
        return MASTERY_EXPERT;
    return MASTERY_NOVICE;
}

/**
 * Wears an item in the first free slot that fits it, where MM8.exe 0x492D41 would.
 *
 * @return                              Whether the item is worn now.
 */
bool tryWear(Character &character, const Item &item) {
    ItemSlot slot = ITEM_SLOT_INVALID;
    InventoryEntry mainHand = character.inventory.entry(ITEM_SLOT_MAIN_HAND);
    InventoryEntry offHand = character.inventory.entry(ITEM_SLOT_OFF_HAND);
    bool twoHanded = mainHand && mainHand->type() == ITEM_TYPE_TWO_HANDED;
    switch (item.type()) {
    case ITEM_TYPE_SINGLE_HANDED:
    case ITEM_TYPE_WAND:
        if (!mainHand) {
            slot = ITEM_SLOT_MAIN_HAND;
        } else if (!offHand && !twoHanded && item.type() == ITEM_TYPE_SINGLE_HANDED) {
            CombinedSkillValue skill = character.getActualSkillValue(item.skill());
            if ((item.skill() == SKILL_DAGGER && skill.mastery() >= MASTERY_EXPERT) ||
                (item.skill() == SKILL_SWORD && skill.mastery() >= MASTERY_MASTER))
                slot = ITEM_SLOT_OFF_HAND;
        }
        break;
    case ITEM_TYPE_TWO_HANDED:
        if (!mainHand && !offHand)
            slot = ITEM_SLOT_MAIN_HAND;
        break;
    case ITEM_TYPE_SHIELD:
        if (!offHand && !twoHanded)
            slot = ITEM_SLOT_OFF_HAND;
        break;
    case ITEM_TYPE_RING:
        for (ItemSlot ring : allRingSlots()) {
            if (!character.inventory.entry(ring)) {
                slot = ring;
                break;
            }
        }
        break;
    default:
        for (ItemSlot candidate : itemSlotsForItemType(item.type())) {
            if (!character.inventory.entry(candidate)) {
                slot = candidate;
                break;
            }
        }
        break;
    }
    if (slot == ITEM_SLOT_INVALID || !character.inventory.canEquip(slot))
        return false;
    character.inventory.equip(slot, item);
    return true;
}

} // namespace

void loadMm8Roster(const Blob &roster) {
    rosterLines.clear();
    for (std::string_view line : split(roster.str()).by("\r\n").drop(2)) {
        if (rosterLines.size() == MM8_ROSTER_SIZE)
            break;
        std::vector<std::string_view> columns;
        for (std::string_view column : split(line).by('\t'))
            columns.push_back(column);
        columns.resize(std::max<size_t>(columns.size(), 124));

        RosterLine &result = rosterLines.emplace_back();
        result.name = trim(columns[1]);
        result.mm8Class = mm8ClassFromName(columns[2]);
        result.birthYear = std::min(toInt(columns[3]), 1156);
        result.face = toInt(columns[4]);
        result.voice = toInt(columns[5]);
        result.experience = toInt64(columns[6]);
        result.level = std::max(toInt(columns[7]), 1);
        for (int i = 0; i < 7; i++)
            result.stats[i] = toInt(columns[8 + i]);
        for (int i = 0; i < 6; i++)
            result.resistances[i] = toInt(columns[15 + i]);
        result.skillPoints = toInt(columns[21]);
        for (int i = 0; i < MM8_SKILL_COUNT; i++) {
            int level = toInt(columns[23 + 2 * i]);
            if (level > 0)
                result.skills[i] = CombinedSkillValue(level, masteryFromLetter(columns[22 + 2 * i]));
        }
        for (int i = 0; i < MM8_ROSTER_SPELL_SCHOOL_COUNT; i++)
            result.spellCounts[i] = std::clamp(toInt(columns[100 + i]), 0, MM8_SPELLS_PER_SCHOOL);
        for (int i = 0; i < MM8_ROSTER_ITEM_COUNT; i++)
            result.items[i] = toInt(columns[113 + i]);
        result.blurb = unquote(trim(columns[123]));
    }
}

void createMm8RosterCharacter(int rosterId, Character *character) {
    assert(rosterId >= 0 && rosterId < rosterLines.size());
    const RosterLine &line = rosterLines[rosterId];

    *character = Character();
    character->mm8RosterId = rosterId;
    character->name = line.name;
    character->classType = classFromMm8(line.mm8Class);
    character->uCurrentFace = line.face;
    character->uPrevFace = line.face;
    character->uVoiceID = line.voice;
    character->uPrevVoiceID = line.voice;
    character->SetSexByVoice();
    character->uBirthYear = line.birthYear;
    character->experience = line.experience;
    character->uLevel = line.level;
    static constexpr std::array<Attribute, 7> stats = {ATTRIBUTE_MIGHT, ATTRIBUTE_INTELLIGENCE, ATTRIBUTE_PERSONALITY,
                                                       ATTRIBUTE_ENDURANCE, ATTRIBUTE_SPEED, ATTRIBUTE_ACCURACY, ATTRIBUTE_LUCK};
    for (int i = 0; i < 7; i++)
        character->_stats[stats[i]] = line.stats[i];
    character->sResFireBase = line.resistances[0];
    character->sResAirBase = line.resistances[1];
    character->sResWaterBase = line.resistances[2];
    character->sResEarthBase = line.resistances[3];
    character->sResMindBase = line.resistances[4];
    character->sResBodyBase = line.resistances[5];
    character->uSkillPoints = line.skillPoints;
    for (int i = 0; i < MM8_SKILL_COUNT; i++) {
        Skill skill = skillFromMm8(i);
        if (skill != SKILL_INVALID && line.skills[i])
            character->pActiveSkills[skill] = line.skills[i];
    }

    // The dark elf, vampire and dragon schools after the regular ones come with the racial skill, see
    // Character::knowsSpell, only their page is taken from here.
    bool bookPageSet = false;
    for (int school = 0; school < MM8_ROSTER_SPELL_SCHOOL_COUNT; school++) {
        if (line.spellCounts[school] > 0 && !bookPageSet) {
            bookPageSet = true;
            character->lastOpenedSpellbookPage = school <= std::to_underlying(MAGIC_SCHOOL_LAST) ? static_cast<MagicSchool>(school)
                                                                                                 : MAGIC_SCHOOL_MM8_RACIAL;
        }
        for (int i = 0; i < line.spellCounts[school]; i++) {
            int spell = 1 + school * MM8_SPELLS_PER_SCHOOL + i;
            if (spell <= std::to_underlying(SPELL_LAST_REGULAR))
                character->bHaveSpell[static_cast<SpellId>(spell)] = true;
        }
    }

    for (int value : line.items) {
        if (value <= 0)
            continue;
        ItemId itemId = itemIdFromMm8(value % 1000);
        if (itemId == ITEM_NULL)
            continue;
        Item item;
        item.itemId = itemId;
        pItemTable->enchantItem(static_cast<ItemTreasureLevel>(std::clamp(value / 1000, 1, 6)), &item);
        item.itemId = itemId;
        item.flags |= ITEM_IDENTIFIED;
        Skill skill = item.skill();
        if ((skill == SKILL_MISC || (skill != SKILL_INVALID && character->pActiveSkills[skill])) && tryWear(*character, item))
            continue;
        character->inventory.tryAdd(item);
    }

    character->health = character->GetMaxHealth();
    character->mana = character->GetMaxMana();
}

const std::string &mm8RosterBlurb(int rosterId) {
    static const std::string empty;
    return rosterId >= 0 && rosterId < rosterLines.size() ? rosterLines[rosterId].blurb : empty;
}

bool isMm8RosterCharacterInParty(int rosterId) {
    return std::ranges::any_of(pParty->pCharacters, [&](const Character &character) { return character.mm8RosterId == rosterId; });
}

Mm8JoinResult joinMm8RosterCharacter(int rosterId) {
    if (isMm8RosterCharacterInParty(rosterId))
        return MM8_JOIN_ALREADY_IN_PARTY;
    if (pParty->pCharacters.size() == pParty->pCharacters.CAPACITY)
        return MM8_JOIN_PARTY_FULL;

    Character character;
    auto inn = std::ranges::find(pParty->mm8InnCharacters, rosterId, &Character::mm8RosterId);
    if (inn != pParty->mm8InnCharacters.end()) {
        character = std::move(*inn);
        pParty->mm8InnCharacters.erase(inn);
    } else {
        createMm8RosterCharacter(rosterId, &character);
    }

    size_t index = pParty->pCharacters.size();
    pParty->pCharacters.resize(index + 1);
    pParty->pCharacters[index] = std::move(character);
    return MM8_JOINED;
}

void sendMm8RosterCharacterToInn(int rosterId) {
    pParty->_questBits.set(static_cast<QuestBit>(MM8_FIRST_ROSTER_QUEST_BIT + rosterId));
    if (isMm8RosterCharacterInParty(rosterId) ||
        std::ranges::find(pParty->mm8InnCharacters, rosterId, &Character::mm8RosterId) != pParty->mm8InnCharacters.end())
        return;
    createMm8RosterCharacter(rosterId, &pParty->mm8InnCharacters.emplace_back());
    std::ranges::sort(pParty->mm8InnCharacters, std::less(), &Character::mm8RosterId); // The inn lists them by roster id.
}

void dismissMm8RosterCharacter(int partyIndex) {
    assert(partyIndex > 0 && partyIndex < pParty->pCharacters.size());
    std::span<Character, PartyCharacters::CAPACITY> slots = pParty->pCharacters.slots();
    pParty->mm8InnCharacters.push_back(std::move(slots[partyIndex]));
    std::ranges::sort(pParty->mm8InnCharacters, std::less(), &Character::mm8RosterId);
    for (int i = partyIndex; i + 1 < pParty->pCharacters.size(); i++)
        slots[i] = std::move(slots[i + 1]);
    pParty->pCharacters.resize(pParty->pCharacters.size() - 1);
}
