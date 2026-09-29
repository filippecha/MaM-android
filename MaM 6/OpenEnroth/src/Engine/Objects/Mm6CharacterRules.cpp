#include "Mm6CharacterRules.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <initializer_list>
#include <utility>

namespace {

struct Mm6ClassLine {
    Class baseClass;
    std::array<int, 7> stats; // Might, Intellect, Personality, Endurance, Accuracy, Speed, Luck.
    std::initializer_list<Skill> startingSkills;
    std::initializer_list<Skill> creationSkills;
    int baseHealth;
    std::array<int, 3> healthPerLevel; // Base class and its two promotions.
    int baseMana;
    std::array<int, 3> manaPerLevel;
};

// Starting stats and skills as shown by the original MM6 party creation screen. Health and mana are from MM6.exe
// 0x4C2630 (HPBase), 0x4C2640 (HPFactor), 0x4C2638 (SPBase) and 0x4C2654 (SPFactor).
const std::array<Mm6ClassLine, 6> mm6ClassLines = {{
    {CLASS_KNIGHT, {14, 7, 7, 14, 11, 11, 9}, {SKILL_SWORD, SKILL_LEATHER},
        {SKILL_DAGGER, SKILL_AXE, SKILL_SPEAR, SKILL_BOW, SKILL_SHIELD, SKILL_CHAIN, SKILL_BODYBUILDING, SKILL_PERCEPTION, SKILL_TRAP_DISARM},
        30, {4, 6, 8}, 0, {0, 0, 0}},
    {CLASS_PALADIN, {14, 7, 14, 11, 11, 9, 7}, {SKILL_SWORD, SKILL_SPIRIT},
        {SKILL_DAGGER, SKILL_SPEAR, SKILL_MACE, SKILL_SHIELD, SKILL_LEATHER, SKILL_CHAIN, SKILL_PERCEPTION, SKILL_DIPLOMACY, SKILL_TRAP_DISARM},
        25, {3, 4, 5}, 5, {1, 2, 3}},
    {CLASS_ARCHER, {9, 14, 7, 11, 14, 11, 7}, {SKILL_BOW, SKILL_AIR},
        {SKILL_SWORD, SKILL_DAGGER, SKILL_AXE, SKILL_LEATHER, SKILL_FIRE, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_DIPLOMACY, SKILL_TRAP_DISARM},
        25, {3, 4, 5}, 5, {1, 2, 3}},
    {CLASS_CLERIC, {7, 9, 14, 11, 11, 7, 14}, {SKILL_MACE, SKILL_BODY},
        {SKILL_STAFF, SKILL_SHIELD, SKILL_LEATHER, SKILL_SPIRIT, SKILL_MIND, SKILL_ITEM_ID, SKILL_REPAIR, SKILL_MEDITATION, SKILL_DIPLOMACY},
        20, {2, 3, 4}, 10, {3, 4, 5}},
    {CLASS_DRUID, {7, 14, 14, 11, 7, 11, 9}, {SKILL_STAFF, SKILL_EARTH},
        {SKILL_MACE, SKILL_LEATHER, SKILL_WATER, SKILL_SPIRIT, SKILL_BODY, SKILL_ITEM_ID, SKILL_REPAIR, SKILL_MEDITATION, SKILL_LEARNING},
        20, {2, 3, 4}, 10, {3, 4, 5}},
    {CLASS_SORCERER, {7, 14, 9, 11, 7, 14, 11}, {SKILL_DAGGER, SKILL_FIRE},
        {SKILL_STAFF, SKILL_LEATHER, SKILL_AIR, SKILL_WATER, SKILL_EARTH, SKILL_ITEM_ID, SKILL_REPAIR, SKILL_MEDITATION, SKILL_DIPLOMACY},
        20, {2, 3, 4}, 10, {3, 4, 5}},
}};

const Mm6ClassLine &classLine(Class cls) {
    Class base = static_cast<Class>(std::to_underlying(cls) / 4 * 4);
    for (const Mm6ClassLine &line : mm6ClassLines)
        if (line.baseClass == base)
            return line;
    return mm6ClassLines[0]; // MM7-only classes can show up transiently, e.g. in Party::Reset.
}

bool contains(std::initializer_list<Skill> skills, Skill skill) {
    return std::find(skills.begin(), skills.end(), skill) != skills.end();
}

} // namespace

const char *mm6PortraitName(int face) {
    static constexpr std::array<const char *, MM6_PORTRAIT_COUNT> names = {{
        "malea", "maleb", "malec", "maled", "malee", "malef", "maleg", "maleh",
        "girla", "girlb", "girlc", "girld"
    }};
    return names[std::clamp(face, 0, MM6_PORTRAIT_COUNT - 1)];
}

Sex mm6SexForPortrait(int face) {
    return face >= MM6_FIRST_FEMALE_PORTRAIT ? SEX_FEMALE : SEX_MALE;
}

Class classFromMm6(int mm6Id) {
    // MM6 class.txt order: knight, cleric, sorcerer, paladin, archer, druid lines, three classes each.
    static constexpr std::array<Class, 6> lines = {CLASS_KNIGHT, CLASS_CLERIC, CLASS_SORCERER, CLASS_PALADIN, CLASS_ARCHER, CLASS_DRUID};
    if (mm6Id < 0 || mm6Id >= 18)
        return CLASS_KNIGHT;
    return static_cast<Class>(std::to_underlying(lines[mm6Id / 3]) + mm6Id % 3);
}

SkillAffinity mm6CreationSkillAffinity(Class cls, Skill skill) {
    const Mm6ClassLine &line = classLine(cls);
    if (contains(line.startingSkills, skill))
        return SKILL_AFFINITY_PRIMARY;
    if (contains(line.creationSkills, skill))
        return SKILL_AFFINITY_AVAILABLE;
    return SKILL_AFFINITY_DENIED;
}

int mm6StartingStat(Class cls, Attribute attribute) {
    return classLine(cls).stats[std::to_underlying(attribute)];
}

int mm6BaseHealth(Class cls) {
    return classLine(cls).baseHealth;
}

int mm6HealthPerLevel(Class cls) {
    return classLine(cls).healthPerLevel[std::min(std::to_underlying(cls) % 4, 2)];
}

int mm6BaseMana(Class cls) {
    return classLine(cls).baseMana;
}

int mm6ManaPerLevel(Class cls) {
    return classLine(cls).manaPerLevel[std::min(std::to_underlying(cls) % 4, 2)];
}
