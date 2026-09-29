#include "Mm6Ids.h"

#include <array>
#include <utility>

namespace {

struct Mm6ItemRange {
    int mm6First;
    int mm6Last;
    int engineFirst;
};

// MM6 item ranges, taken from MM6 items.txt.
constexpr std::array<Mm6ItemRange, 13> mm6ItemRanges = {{
    {1, 159, 1},        // Weapons, armor, jewelry and wands, same ids as MM7.
    {160, 162, 200},    // Herbs -> reagents.
    {163, 163, 220},    // Empty potion bottle.
    {164, 196, 222},    // Potions and spare bottles -> real potions.
    {197, 199, 197},    // Gold.
    {200, 299, 300},    // Spell scrolls.
    {300, 399, 400},    // Spell books.
    {400, 429, 500},    // Artifacts and relics.
    {430, 499, 600},    // Quest & misc items.
    {500, 539, 700},    // Message scrolls, placed around MM7 recipes (740-771).
    {540, 549, 772},
    {550, 579, 670},    // More quest items.
    {580, 580, 782},
}};

} // namespace

ItemId itemIdFromMm6(int mm6Id) {
    for (const Mm6ItemRange &range : mm6ItemRanges)
        if (mm6Id >= range.mm6First && mm6Id <= range.mm6Last)
            return static_cast<ItemId>(range.engineFirst + mm6Id - range.mm6First);
    return ITEM_NULL;
}

Skill skillFromMm6(int mm6Id) {
    if (mm6Id >= std::to_underlying(SKILL_STAFF) && mm6Id <= std::to_underlying(SKILL_TRAP_DISARM))
        return static_cast<Skill>(mm6Id);
    if (mm6Id == std::to_underlying(SKILL_TRAP_DISARM) + 1)
        return SKILL_LEARNING;
    return SKILL_INVALID;
}

int mm6IdFromItemId(ItemId itemId) {
    int id = std::to_underlying(itemId);
    for (const Mm6ItemRange &range : mm6ItemRanges)
        if (id >= range.engineFirst && id <= range.engineFirst + range.mm6Last - range.mm6First)
            return range.mm6First + id - range.engineFirst;
    return 0;
}

SpriteId mm6ObjectSprite(SpriteId sprite) {
    // MM6 projectiles: 500 arrow, 510 fire arrow, 520 fire, 530 electric, 540 cold, 550 poison, 560 energy, 570 magic,
    // 580 rock, 590 laser bolt. The explosion of each is the next id.
    static constexpr std::array<std::pair<SpriteId, int>, 12> projectiles = {{
        {SPRITE_PROJECTILE_ARROW, 500},
        {SPRITE_PROJECTILE_FLAMING_ARROW, 510},
        {SPRITE_PROJECTILE_FIRE_BOLT, 520},
        {SPRITE_PROJECTILE_AIR_BOLT, 530},
        {SPRITE_PROJECTILE_WATER_BOLT, 540},
        {SPRITE_PROJECTILE_BODY_BOLT, 550},
        {SPRITE_PROJECTILE_LIGHT_BOLT, 560},
        {SPRITE_PROJECTILE_SPIRIT_BOLT, 570},
        {SPRITE_PROJECTILE_MIND_BOLT, 570},
        {SPRITE_PROJECTILE_DARK_BOLT, 570},
        {SPRITE_PROJECTILE_EARTH_BOLT, 580},
        {SPRITE_PROJECTILE_BLASTER, 590},
    }};

    for (auto [mm7, mm6] : projectiles) {
        if (sprite == mm7)
            return static_cast<SpriteId>(mm6);
        if (std::to_underlying(sprite) == std::to_underlying(mm7) + 1)
            return static_cast<SpriteId>(mm6 + 1);
    }

    switch (sprite) {
    case SPRITE_TRAP_LIGHTNING: return SPRITE_TRAP_COLD; // MM6 has cold at 812 and electricity at 813.
    case SPRITE_TRAP_COLD:      return SPRITE_TRAP_LIGHTNING;
    default:                    return sprite;
    }
}

NpcProfession npcProfessionFromMm6(int mm6Id) {
    if (mm6Id <= 0 || mm6Id > std::to_underlying(Child) - std::to_underlying(NPC_PROFESSION_MM6_SHIFT))
        return NoProfession;
    if (mm6Id >= std::to_underlying(FallenWizard))
        mm6Id += std::to_underlying(NPC_PROFESSION_MM6_SHIFT);
    return static_cast<NpcProfession>(mm6Id);
}

int mm6NpcProfessionNameLine(int mm6Id) {
    // MM6.exe 0x445D33 fills its profession names from these lines.
    static constexpr std::array<int, 78> lines = {
        153, 308, 309, 7,   306, 310, 311, 312, 313, 314, 105, 315, 316, 317, 115, 318, 319, 320, 321, 322,
        323, 293, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341,
        342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 305, 354, 355, 356, 357, 358, 359, 360,
        361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 42,  100, 373, 303, 374, 558
    };
    if (mm6Id < 0 || mm6Id >= static_cast<int>(lines.size()))
        return -1;
    return lines[mm6Id];
}
