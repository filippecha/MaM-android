#include "Mm8Ids.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Mm6ExeData.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/Mm6Ids.h"
#include "Engine/mm7_data.h"

#include "Utility/GameVariant.h"
#include "Utility/IndexedArray.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

namespace {

constexpr std::array<Class, MM8_CLASS_COUNT> mm8Classes = {{
    CLASS_SORCERER, CLASS_LICH,             // Necromancer, lich.
    CLASS_CLERIC, CLASS_PRIEST_OF_SUN,      // Cleric, priest of the sun.
    CLASS_KNIGHT, CLASS_CHAMPION,           // Knight, champion.
    CLASS_THIEF, CLASS_ROGUE,               // Troll, war troll.
    CLASS_MONK, CLASS_INITIATE,             // Minotaur, minotaur lord.
    CLASS_ARCHER, CLASS_WARRIOR_MAGE,       // Dark elf, patriarch.
    CLASS_RANGER, CLASS_HUNTER,             // Vampire, nosferatu.
    CLASS_DRUID, CLASS_GREAT_DRUID,         // Dragon, great wyrm.
}};

// Tables in MM8-Rel.exe of the GOG release. MMExtension has them 0x2010 (0x200C for the stats) lower for the retail
// version 1.2.
constexpr uint32_t HP_BASE_ADDRESS = 0x4FF9EC;
constexpr uint32_t SP_BASE_ADDRESS = 0x4FF9FC;
constexpr uint32_t HP_PER_LEVEL_ADDRESS = 0x4FFA0C;
constexpr uint32_t SP_PER_LEVEL_ADDRESS = 0x4FFA1C;
constexpr uint32_t STARTING_STATS_ADDRESS = 0x4FFA2C;
constexpr uint32_t STARTING_SKILLS_ADDRESS = 0x4FFBEC;
constexpr uint32_t SKILL_MASTERIES_ADDRESS = 0x4FFD24;

IndexedArray<Mm8ClassData, CLASS_FIRST, CLASS_LAST> classData;
std::array<std::vector<std::string>, 8> characterNames; // By the columns of pcnames.txt.
std::array<std::array<int, MM8_SKILL_COUNT>, 8> startingSkills = {};

struct Mm8ItemRange {
    int mm8First;
    int mm8Last;
    int engineFirst;
};

// MM8 item ranges, taken from MM8 items.txt.
constexpr std::array<Mm8ItemRange, 7> mm8ItemRanges = {{
    {1, 134, 1},        // Weapons, armor, helmets, belts, cloaks, gauntlets and most boots, like MM7.
    {135, 151, 160},    // The last boots, rings and amulets go to unused MM7 ids.
    {152, 176, 135},    // Wands.
    {177, 186, 186},    // Gems.
    {187, 189, 197},    // Gold.
    {200, 799, 200},    // Reagents, potions, scrolls, books, artifacts, quest items and message scrolls.
    {800, 802, 782},    // Quest cheeses.
}};

} // namespace

ItemId itemIdFromMm8(int mm8Id) {
    for (const Mm8ItemRange &range : mm8ItemRanges)
        if (mm8Id >= range.mm8First && mm8Id <= range.mm8Last)
            return static_cast<ItemId>(range.engineFirst + mm8Id - range.mm8First);
    return ITEM_NULL;
}

int mm8IdFromItemId(ItemId itemId) {
    int id = std::to_underlying(itemId);
    for (const Mm8ItemRange &range : mm8ItemRanges)
        if (id >= range.engineFirst && id <= range.engineFirst + range.mm8Last - range.mm8First)
            return range.mm8First + id - range.engineFirst;
    return 0;
}

ItemId itemIdFromData(int id) {
    if (isMm6())
        return itemIdFromMm6(id);
    if (isMm8())
        return itemIdFromMm8(id);
    return static_cast<ItemId>(id);
}

namespace {

constexpr std::array<std::pair<AwardId, int>, 9> mm8CounterAwards = {{
    {AWARD_ARCOMAGE_WINS, 36}, {AWARD_ARCOMAGE_LOSES, 37}, {AWARD_DEATHS, 43}, {AWARD_BOUNTIES_COLLECTED, 44},
    {AWARD_PRISON_TERMS, 45}, {AWARD_ARENA_PAGE_WINS, 46}, {AWARD_ARENA_SQUIRE_WINS, 47}, {AWARD_ARENA_KNIGHT_WINS, 48},
    {AWARD_ARENA_LORD_WINS, 49},
}};

} // namespace

AwardId counterAward(AwardId award) {
    if (!isMm8())
        return award;
    for (auto [mm7Award, mm8Award] : mm8CounterAwards)
        if (mm7Award == award)
            return static_cast<AwardId>(mm8Award);
    return award;
}

AwardId counterAwardFromMm8(AwardId mm8Award) {
    for (auto [mm7Award, id] : mm8CounterAwards)
        if (id == std::to_underlying(mm8Award))
            return mm7Award;
    return AWARD_INVALID;
}

SpellId spellFromMm8(int mm8Id) {
    if (mm8Id >= 1 && mm8Id <= std::to_underlying(SPELL_LAST_REGULAR))
        return static_cast<SpellId>(mm8Id);
    for (int line = 0; line < 3; line++) {
        int first = 100 + 11 * line; // Dark elf 100-103, vampire 111-114 and dragon 122-125.
        if (mm8Id >= first && mm8Id < first + 4)
            return static_cast<SpellId>(std::to_underlying(SPELL_FIRST_MM8_RACIAL) + 4 * line + mm8Id - first);
    }
    return SPELL_NONE;
}

std::span<const SpellId> mm8RacialSpells(Mm8ClassKind kind) {
    static constexpr std::array<SpellId, 4> darkElf = {SPELL_DARK_ELF_GLAMOUR, SPELL_DARK_ELF_TRAVELERS_BOON, SPELL_DARK_ELF_BLIND,
                                                      SPELL_DARK_ELF_DARKFIRE_BOLT};
    static constexpr std::array<SpellId, 4> vampire = {SPELL_VAMPIRE_LIFEDRAIN, SPELL_VAMPIRE_LEVITATE, SPELL_VAMPIRE_CHARM,
                                                      SPELL_VAMPIRE_MISTFORM};
    static constexpr std::array<SpellId, 4> dragon = {SPELL_DRAGON_FEAR, SPELL_DRAGON_FLAME_BLAST, SPELL_DRAGON_FLIGHT,
                                                     SPELL_DRAGON_WING_BUFFET};
    switch (kind) {
    case MM8_DARK_ELF: return darkElf;
    case MM8_VAMPIRE: return vampire;
    case MM8_DRAGON: return dragon;
    default: return {};
    }
}

bool mm8MonsterInGroup(MonsterId monsterId, Mm8MonsterGroup group) {
    // Each range is a monster in its A, B and C variants.
    auto inAny = [id = std::to_underlying(monsterId)](std::initializer_list<int> firstIds) {
        return std::ranges::any_of(firstIds, [id](int first) { return id >= first && id <= first + 2; });
    };
    switch (group) {
    case MM8_MONSTER_UNDEAD: return inAny({49, 52, 163, 172});
    case MM8_MONSTER_DRAGON: return inAny({70, 136, 163, 112, 190});
    case MM8_MONSTER_SWIMMER: return inAny({178, 76, 136});
    case MM8_MONSTER_IMMOBILE: return inAny({142});
    case MM8_MONSTER_PEASANT: return inAny({1, 19, 22, 28, 49, 61, 67, 166});
    case MM8_MONSTER_NOT_IN_ARENA:
        return mm8MonsterInGroup(monsterId, MM8_MONSTER_IMMOBILE) || mm8MonsterInGroup(monsterId, MM8_MONSTER_SWIMMER) ||
               mm8MonsterInGroup(monsterId, MM8_MONSTER_PEASANT);
    case MM8_MONSTER_OGRE: return inAny({31, 28, 106, 58, 61, 91});
    case MM8_MONSTER_ELEMENTAL: return inAny({73, 76, 79, 82, 157, 130, 115, 127, 103});
    }
    return false;
}

std::vector<ItemId> mm8DeathDrops(MonsterId monsterId) {
    struct Drop {
        int firstMonster; // With its B and C variants.
        int mm8Item;
    };
    static constexpr std::array<Drop, 11> drops = {{
        {31, 653},  // Ogre warriors, ogre ears.
        {85, 633},  // Dire wolves, pelts.
        {97, 207},  // Wisps, wisp hearts.
        {109, 214}, // Unicorns, unicorn horns.
        {118, 217}, // Wasp warriors, wasp wings.
        {118, 654}, // And wasp stingers.
        {121, 640}, // Wyverns, wyvern horns.
        {130, 204}, // Phoenixes, phoenix feathers.
        {136, 209}, // Dragon turtles, dragon turtle fangs.
        {160, 202}, // Gogs, gog blood.
        {169, 632}, // Nagas, naga hides.
    }};
    std::vector<ItemId> result;
    int id = std::to_underlying(monsterId);
    for (const Drop &drop : drops)
        if (id >= drop.firstMonster && id <= drop.firstMonster + 2)
            result.push_back(itemIdFromMm8(drop.mm8Item));
    return result;
}

bool mm8WeaponSlays(const Item &weapon, MonsterId monsterId) {
    // MM8 keeps the MM7 enchantment ids, 39 and 63 slay ogres and elementals instead of demons and elves.
    ItemEnchantment enchantment = weapon.specialEnchantment;
    if (enchantment == ITEM_ENCHANTMENT_UNDEAD_SLAYING)
        return mm8MonsterInGroup(monsterId, MM8_MONSTER_UNDEAD);
    if (enchantment == ITEM_ENCHANTMENT_DEMON_SLAYING || weapon.itemId == ITEM_MM8_MACHETE_OF_JUSTICE)
        return mm8MonsterInGroup(monsterId, MM8_MONSTER_OGRE);
    if (enchantment == ITEM_ENCHANTMENT_DRAGON_SLAYING || weapon.itemId == ITEM_MM8_DRAGON_LANCE)
        return mm8MonsterInGroup(monsterId, MM8_MONSTER_DRAGON);
    if (enchantment == ITEM_ENCHANTMENT_ELF_SLAYING)
        return mm8MonsterInGroup(monsterId, MM8_MONSTER_ELEMENTAL);
    return false;
}

Class classFromMm8(int mm8Id) {
    return mm8Classes[mm8Id];
}

int mm8IdFromClass(Class cls) {
    for (int i = 0; i < MM8_CLASS_COUNT; i++)
        if (mm8Classes[i] == cls)
            return i;
    return -1;
}

Skill skillFromMm8(int mm8Id) {
    if (mm8Id < 0 || mm8Id >= MM8_SKILL_COUNT)
        return SKILL_INVALID;
    if (mm8Id <= std::to_underlying(SKILL_DARK))
        return static_cast<Skill>(mm8Id);
    if (mm8Id <= 23)
        return SKILL_MM8_RACIAL;
    if (mm8Id == 30)
        return SKILL_MM8_REGENERATION;
    if (mm8Id < 30)
        return static_cast<Skill>(mm8Id - 3); // Item id to perception.
    return static_cast<Skill>(mm8Id - 2); // Disarm traps to learning.
}

int mm8RacialLine(Mm8ClassKind kind) {
    switch (kind) {
    case MM8_DARK_ELF: return 0;
    case MM8_VAMPIRE: return 1;
    case MM8_DRAGON: return 2;
    default: return -1;
    }
}

Class mm8BaseClass(Mm8ClassKind kind) {
    static constexpr std::array<int, 8> ids = {0, 2, 4, 6, 8, 10, 12, 14};
    return classFromMm8(ids[std::to_underlying(kind)]);
}

Mm8ClassKind mm8ClassKind(Class cls) {
    int id = mm8IdFromClass(cls);
    return static_cast<Mm8ClassKind>(id < 0 ? 0 : id / 2);
}

Mm8ClassKind mm8FaceClassKind(int face) {
    if (face < 4)
        return MM8_KNIGHT;
    if (face < 8)
        return MM8_CLERIC;
    if (face < 12)
        return MM8_NECROMANCER;
    if (face < 16)
        return MM8_VAMPIRE;
    if (face < 20)
        return MM8_DARK_ELF;
    if (face < 22)
        return MM8_MINOTAUR;
    if (face < 24)
        return MM8_TROLL;
    if (face < 26)
        return MM8_DRAGON;
    return MM8_NECROMANCER;
}

bool mm8FaceIsFemale(int face) {
    return face < 20 || face >= 26 ? face % 2 == 1 : false;
}

void loadMm8ClassTables() {
    if (!mm8ExeData.isLoaded())
        return;

    std::vector<uint8_t> hpBase = mm8ExeData.bytes(HP_BASE_ADDRESS, MM8_CLASS_COUNT);
    std::vector<uint8_t> spBase = mm8ExeData.bytes(SP_BASE_ADDRESS, MM8_CLASS_COUNT);
    std::vector<uint8_t> hpPerLevel = mm8ExeData.bytes(HP_PER_LEVEL_ADDRESS, MM8_CLASS_COUNT);
    std::vector<uint8_t> spPerLevel = mm8ExeData.bytes(SP_PER_LEVEL_ADDRESS, MM8_CLASS_COUNT);
    std::vector<uint8_t> stats = mm8ExeData.bytes(STARTING_STATS_ADDRESS, MM8_CLASS_COUNT * 7 * 4);
    std::vector<uint8_t> masteries = mm8ExeData.bytes(SKILL_MASTERIES_ADDRESS, MM8_CLASS_COUNT * MM8_SKILL_COUNT);
    std::vector<uint8_t> starting = mm8ExeData.bytes(STARTING_SKILLS_ADDRESS, 8 * MM8_SKILL_COUNT);

    for (int id = 0; id < MM8_CLASS_COUNT; id++) {
        Class cls = classFromMm8(id);
        Mm8ClassData &data = classData[cls];
        data.hpBase = hpBase[id];
        data.spBase = spBase[id];
        data.hpPerLevel = hpPerLevel[id];
        data.spPerLevel = spPerLevel[id];
        for (int stat = 0; stat < 7; stat++) {
            const uint8_t *raw = &stats[(id * 7 + stat) * 4];
            data.stats[stat] = {static_cast<int8_t>(raw[0]), static_cast<int8_t>(raw[1]), static_cast<int8_t>(raw[2]), static_cast<int8_t>(raw[3])};
        }

        skillMaxMasteryPerClass[cls].fill(MASTERY_NONE);
        for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++) {
            Skill skill = skillFromMm8(mm8Skill);
            Mastery mastery = static_cast<Mastery>(masteries[id * MM8_SKILL_COUNT + mm8Skill]);
            if (skill != SKILL_INVALID && mastery > skillMaxMasteryPerClass[cls][skill])
                skillMaxMasteryPerClass[cls][skill] = mastery;
        }
    }

    for (int kind = 0; kind < 8; kind++)
        for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++)
            startingSkills[kind][mm8Skill] = starting[kind * MM8_SKILL_COUNT + mm8Skill];
}

void loadMm8CharacterNames(const Blob &pcNames) {
    for (std::vector<std::string> &names : characterNames)
        names.clear();
    for (std::string_view line : split(pcNames.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 8> tokens = split(line).by('\t');
        for (int column = 0; column < 8; column++)
            if (!trim(tokens[column]).empty() && trim(tokens[column]) != "*end") // "*end" closes each column.
                characterNames[column].emplace_back(trim(tokens[column]));
    }
}

std::string mm8RandomCharacterName(int face) {
    // Columns: knight & cleric male, female, dark elf male, female, troll & minotaur, vampire & necromancer female, male,
    // dragon.
    bool female = mm8FaceIsFemale(face);
    int column = 0;
    switch (mm8FaceClassKind(face)) {
    case MM8_KNIGHT:
    case MM8_CLERIC:
        column = female ? 1 : 0;
        break;
    case MM8_DARK_ELF:
        column = female ? 3 : 2;
        break;
    case MM8_TROLL:
    case MM8_MINOTAUR:
        column = 4;
        break;
    case MM8_NECROMANCER:
    case MM8_VAMPIRE:
        column = female ? 5 : 6;
        break;
    case MM8_DRAGON:
        column = 7;
        break;
    }
    if (characterNames[column].empty())
        return {};
    return grng->randomSample(characterNames[column]);
}

const Mm8ClassData &mm8ClassData(Class cls) {
    return classData[cls];
}

int mm8StartingSkill(Mm8ClassKind kind, Skill skill) {
    int result = 0;
    for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++)
        if (skillFromMm8(mm8Skill) == skill)
            result = std::max(result, startingSkills[std::to_underlying(kind)][mm8Skill]);
    return result;
}
