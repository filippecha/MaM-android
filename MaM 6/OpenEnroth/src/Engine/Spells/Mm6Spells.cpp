#include "Mm6Spells.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Mm6ExeData.h"
#include "Engine/Random/Random.h"

#include "Utility/IndexedArray.h"
#include "Utility/String/Ascii.h"

namespace {

// MM6 spell -> engine effect. MM6-only spells use the closest MM7 effect.
constexpr std::array<SpellId, 100> mm6Effects = {{
    SPELL_NONE,
    // Fire.
    SPELL_FIRE_TORCH_LIGHT,
    SPELL_FIRE_FIRE_BOLT,               // Flame Arrow.
    SPELL_FIRE_PROTECTION_FROM_FIRE,
    SPELL_FIRE_FIRE_BOLT,
    SPELL_FIRE_HASTE,
    SPELL_FIRE_FIREBALL,
    SPELL_FIRE_IMMOLATION,              // Ring of Fire, hits everything around the party.
    SPELL_FIRE_FIRE_SPIKE,              // Fire Blast.
    SPELL_FIRE_METEOR_SHOWER,
    SPELL_FIRE_INFERNO,
    SPELL_FIRE_INCINERATE,
    // Air.
    SPELL_AIR_WIZARD_EYE,
    SPELL_AIR_LIGHTNING_BOLT,           // Static Charge.
    SPELL_AIR_PROTECTION_FROM_AIR,
    SPELL_AIR_SPARKS,
    SPELL_AIR_FEATHER_FALL,
    SPELL_AIR_SHIELD,
    SPELL_AIR_LIGHTNING_BOLT,
    SPELL_AIR_JUMP,
    SPELL_AIR_IMPLOSION,
    SPELL_AIR_FLY,
    SPELL_AIR_STARBURST,
    // Water.
    SPELL_WATER_AWAKEN,
    SPELL_WATER_ICE_BOLT,               // Cold Beam.
    SPELL_WATER_PROTECTION_FROM_WATER,
    SPELL_WATER_POISON_SPRAY,
    SPELL_WATER_WATER_WALK,
    SPELL_WATER_ICE_BOLT,
    SPELL_WATER_ENCHANT_ITEM,
    SPELL_WATER_ACID_BURST,
    SPELL_WATER_TOWN_PORTAL,
    SPELL_WATER_ICE_BLAST,
    SPELL_WATER_LLOYDS_BEACON,
    // Earth.
    SPELL_EARTH_STUN,
    SPELL_MIND_MIND_BLAST,              // Magic Arrow, a plain projectile.
    SPELL_BODY_PROTECTION_FROM_MAGIC,
    SPELL_EARTH_DEADLY_SWARM,
    SPELL_EARTH_STONESKIN,
    SPELL_EARTH_BLADES,
    SPELL_EARTH_STONE_TO_FLESH,
    SPELL_EARTH_ROCK_BLAST,
    SPELL_LIGHT_PARALYZE,               // Turn to Stone.
    SPELL_EARTH_DEATH_BLOSSOM,
    SPELL_EARTH_MASS_DISTORTION,
    // Spirit.
    SPELL_MIND_MIND_BLAST,              // Spirit Arrow, a plain projectile.
    SPELL_SPIRIT_BLESS,
    SPELL_BODY_FIRST_AID,               // Healing Touch.
    SPELL_SPIRIT_FATE,                  // Lucky Day.
    SPELL_SPIRIT_REMOVE_CURSE,
    SPELL_SPIRIT_PRESERVATION,          // Guardian Angel.
    SPELL_SPIRIT_HEROISM,
    SPELL_SPIRIT_TURN_UNDEAD,
    SPELL_SPIRIT_RAISE_DEAD,
    SPELL_SPIRIT_SHARED_LIFE,
    SPELL_SPIRIT_RESSURECTION,
    // Mind.
    SPELL_BODY_REGENERATION,            // Meditation, regenerates the caster.
    SPELL_MIND_REMOVE_FEAR,
    SPELL_MIND_MIND_BLAST,
    SPELL_SPIRIT_BLESS,                 // Precision.
    SPELL_MIND_CURE_PARALYSIS,
    SPELL_MIND_CHARM,
    SPELL_MIND_MASS_FEAR,
    SPELL_EARTH_SLOW,                   // Feeblemind.
    SPELL_MIND_CURE_INSANITY,
    SPELL_MIND_PSYCHIC_SHOCK,
    SPELL_EARTH_TELEKINESIS,
    // Body.
    SPELL_BODY_CURE_WEAKNESS,
    SPELL_BODY_FIRST_AID,
    SPELL_BODY_PROTECTION_FROM_BODY,
    SPELL_BODY_HARM,
    SPELL_BODY_FIRST_AID,               // Cure Wounds.
    SPELL_BODY_CURE_POISON,
    SPELL_FIRE_HASTE,                   // Speed.
    SPELL_BODY_CURE_DISEASE,
    SPELL_SPIRIT_HEROISM,               // Power.
    SPELL_BODY_FLYING_FIST,
    SPELL_BODY_POWER_CURE,
    // Light.
    SPELL_LIGHT_DAY_OF_THE_GODS,        // Create Food, see MM6_SPELL_CREATE_FOOD.
    SPELL_WATER_ENCHANT_ITEM,           // Golden Touch, see MM6_SPELL_GOLDEN_TOUCH.
    SPELL_LIGHT_DISPEL_MAGIC,
    SPELL_EARTH_SLOW,
    SPELL_LIGHT_DESTROY_UNDEAD,
    SPELL_LIGHT_DAY_OF_THE_GODS,
    SPELL_LIGHT_PRISMATIC_LIGHT,
    SPELL_LIGHT_HOUR_OF_POWER,
    SPELL_LIGHT_PARALYZE,
    SPELL_LIGHT_SUNRAY,
    SPELL_LIGHT_DIVINE_INTERVENTION,
    // Dark.
    SPELL_DARK_REANIMATE,
    SPELL_DARK_TOXIC_CLOUD,
    SPELL_MIND_MASS_FEAR,               // Mass Curse.
    SPELL_DARK_SHARPMETAL,
    SPELL_DARK_SHRINKING_RAY,
    SPELL_LIGHT_DAY_OF_PROTECTION,
    SPELL_DARK_SOULDRINKER,             // Finger of Death.
    SPELL_LIGHT_SUNRAY,                 // Moon Ray.
    SPELL_DARK_DRAGON_BREATH,
    SPELL_DARK_ARMAGEDDON,
    SPELL_NONE,                         // Dark Containment, only used by the endgame event.
}};

// Damage type of every MM7 spell as listed in MM7 spells.txt, index is the spell id.
constexpr std::array<DamageType, 100> mm7DamageTypes = {{
    DAMAGE_PHYSICAL,
    DAMAGE_PHYSICAL, DAMAGE_FIRE, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_FIRE, DAMAGE_FIRE, DAMAGE_FIRE,
    DAMAGE_FIRE, DAMAGE_FIRE, DAMAGE_FIRE,
    DAMAGE_PHYSICAL, DAMAGE_AIR, DAMAGE_PHYSICAL, DAMAGE_AIR, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_AIR, DAMAGE_PHYSICAL,
    DAMAGE_AIR, DAMAGE_PHYSICAL, DAMAGE_AIR,
    DAMAGE_PHYSICAL, DAMAGE_WATER, DAMAGE_PHYSICAL, DAMAGE_WATER, DAMAGE_PHYSICAL, DAMAGE_WATER, DAMAGE_PHYSICAL, DAMAGE_WATER,
    DAMAGE_PHYSICAL, DAMAGE_WATER, DAMAGE_PHYSICAL,
    DAMAGE_EARTH, DAMAGE_EARTH, DAMAGE_PHYSICAL, DAMAGE_EARTH, DAMAGE_PHYSICAL, DAMAGE_EARTH, DAMAGE_PHYSICAL, DAMAGE_EARTH,
    DAMAGE_PHYSICAL, DAMAGE_EARTH, DAMAGE_EARTH,
    DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_SPIRIT, DAMAGE_SPIRIT, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL,
    DAMAGE_SPIRIT, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL,
    DAMAGE_PHYSICAL, DAMAGE_MIND, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_MIND, DAMAGE_MIND, DAMAGE_MIND, DAMAGE_MIND,
    DAMAGE_PHYSICAL, DAMAGE_MIND, DAMAGE_MIND,
    DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_BODY, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL,
    DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_BODY, DAMAGE_PHYSICAL,
    DAMAGE_LIGHT, DAMAGE_LIGHT, DAMAGE_PHYSICAL, DAMAGE_LIGHT, DAMAGE_PHYSICAL, DAMAGE_PHYSICAL, DAMAGE_LIGHT, DAMAGE_PHYSICAL,
    DAMAGE_PHYSICAL, DAMAGE_LIGHT, DAMAGE_PHYSICAL,
    DAMAGE_PHYSICAL, DAMAGE_DARK, DAMAGE_PHYSICAL, DAMAGE_DARK, DAMAGE_DARK, DAMAGE_DARK, DAMAGE_DARK, DAMAGE_PHYSICAL,
    DAMAGE_DARK, DAMAGE_DARK, DAMAGE_DARK,
}};

// Names as used in MM6 monsters.txt, index is the MM6 spell id.
constexpr std::array<std::string_view, 100> mm6SpellNames = {{
    "",
    "Torch Light", "Flame Arrow", "Protection from Fire", "Fire Bolt", "Haste", "Fireball", "Ring of Fire", "Fire Blast",
    "Meteor Shower", "Inferno", "Incinerate",
    "Wizard Eye", "Static Charge", "Protection from Electricity", "Sparks", "Feather Fall", "Shield", "Lightning Bolt",
    "Jump", "Implosion", "Fly", "Starburst",
    "Awaken", "Cold Beam", "Protection from Cold", "Poison Spray", "Water Walk", "Ice Bolt", "Enchant Item",
    "Acid Burst", "Town Portal", "Ice Blast", "Lloyd's Beacon",
    "Stun", "Magic Arrow", "Protection from Magic", "Deadly Swarm", "Stone Skin", "Blades", "Stone to Flesh",
    "Rock Blast", "Turn to Stone", "Death Blossom", "Mass Distortion",
    "Spirit Arrow", "Bless", "Healing Touch", "Lucky Day", "Remove Curse", "Guardian Angel", "Heroism", "Turn Undead",
    "Raise Dead", "Shared Life", "Resurrection",
    "Meditation", "Remove Fear", "Mind Blast", "Precision", "Cure Paralysis", "Charm", "Mass Fear", "Feeblemind",
    "Cure Insanity", "Psychic Shock", "Telekinesis",
    "Cure Weakness", "First Aid", "Protection from Poison", "Harm", "Cure Wounds", "Cure Poison", "Speed", "Cure Disease",
    "Power", "Flying Fist", "Power Cure",
    "Create Food", "Golden Touch", "Dispel Magic", "Slow", "Destroy Undead", "Day of the Gods", "Prismatic Light",
    "Hour of Power", "Paralyze", "Sun Ray", "Divine Intervention",
    "Reanimate", "Toxic Cloud", "Mass Curse", "Shrapmetal", "Shrinking Ray", "Day of Protection", "Finger of Death",
    "Moon Ray", "Dragon Breath", "Armageddon", "Dark Containment",
}};

std::array<std::array<int, 4>, 100> mm6ManaCosts = {};

constexpr uint32_t SPELL_INFO_ADDRESS = 0x4BDD70; // MMExtension's Game.Spells, 14 bytes per spell starting at spell 0.
constexpr uint32_t SPELL_INFO_SIZE = 14;
constexpr uint32_t SPELL_INFO_DELAY_OFFSET = 6; // int16[3], recovery per mastery.

std::array<std::array<int, 3>, 100> mm6Recovery = {};
bool mm6RecoveryLoaded = false;

int masteryIndex(Mastery mastery) {
    return std::clamp(std::to_underlying(mastery), std::to_underlying(MASTERY_NOVICE), std::to_underlying(MASTERY_MASTER)) -
           std::to_underlying(MASTERY_NOVICE);
}

} // namespace

void loadMm6SpellRecovery() {
    if (!mm6ExeData.isLoaded())
        return;

    for (int spell = 1; spell < static_cast<int>(mm6Recovery.size()); spell++) {
        std::vector<uint8_t> raw = mm6ExeData.bytes(SPELL_INFO_ADDRESS + spell * SPELL_INFO_SIZE + SPELL_INFO_DELAY_OFFSET, 6);
        if (raw.size() != 6)
            return;
        for (int i = 0; i < 3; i++)
            mm6Recovery[spell][i] = static_cast<int16_t>(raw[2 * i] | (raw[2 * i + 1] << 8));
    }
    mm6RecoveryLoaded = true;
}

std::optional<Duration> mm6SpellRecovery(SpellId mm6Spell, Mastery mastery) {
    int id = std::to_underlying(mm6Spell);
    if (!mm6RecoveryLoaded || id <= 0 || id >= static_cast<int>(mm6Recovery.size()) || mastery < MASTERY_NOVICE)
        return std::nullopt;
    return Duration::fromTicks(mm6Recovery[id][masteryIndex(mastery)]);
}

int mm6SpellDamage(SpellId mm6Spell, int skill, Mastery mastery, int targetHp) {
    int s = std::max(0, skill);
    switch (std::to_underlying(mm6Spell)) {
    case 2:  return grng->randomDice(1, 8);                     // Flame Arrow.
    case 4:  return grng->randomDice(s, 4);                     // Fire Bolt.
    case 6:  return grng->randomDice(s, 6);                     // Fireball.
    case 7:  return s + 6;                          // Ring of Fire.
    case 8:  return grng->randomDice(s, 3) + 4;                 // Fire Blast.
    case 9:  return s + 8;                          // Meteor Shower.
    case 10: return s + 12;                         // Inferno.
    case 11: return grng->randomDice(s, 15) + 15;               // Incinerate.
    case 13: return grng->randomDice(1, 5) + 1;                 // Static Charge.
    case 15: return s + 2;                          // Sparks.
    case 18: return grng->randomDice(s, 8);                     // Lightning Bolt.
    case 20: return grng->randomDice(s, 10) + 10;               // Implosion.
    case 22: return s + 20;                         // Starburst.
    case 24: return grng->randomDice(2, 3);                     // Cold Beam.
    case 26: return grng->randomDice(s, 2) + 2;                 // Poison Spray.
    case 28: return grng->randomDice(s, 7);                     // Ice Bolt.
    case 30: return grng->randomDice(s, 9) - s + 9;             // Acid Burst, MM6 rolls 0-8 per skill point.
    case 32: return grng->randomDice(s, 2) + 12;                // Ice Blast.
    case 35: return grng->randomDice(1, 6) + 2;                 // Magic Arrow.
    case 37: return grng->randomDice(s, 3) + 5;                 // Deadly Swarm.
    case 39: return grng->randomDice(s, 5);                     // Blades.
    case 41: return grng->randomDice(s, 8);                     // Rock Blast.
    case 43: return s + 20;                         // Death Blossom.
    case 44: return targetHp * (2 * s + 25) / 100;  // Mass Distortion, MM6.exe multiplies by mastery instead of hit points.
    case 45: return grng->randomDice(1, 6);                     // Spirit Arrow.
    case 58: return grng->randomDice(s, 2) + 5;                 // Mind Blast.
    case 65: return grng->randomDice(s, 12) + 12;               // Psychic Shock.
    case 70: return grng->randomDice(s, 2) + 8;                 // Harm.
    case 76: return grng->randomDice(s, 5) + 30;                // Flying Fist.
    case 82: return grng->randomDice(s, 16) + 16;               // Destroy Undead.
    case 84: return s + 25;                         // Prismatic Light.
    case 87: return grng->randomDice(s, 20) + 20;               // Sun Ray.
    case 90: return grng->randomDice(s, 10) + 25;               // Toxic Cloud.
    case 92: return grng->randomDice(s, 6) + 6;                 // Shrapmetal.
    case 95:                                        // Finger of Death, 3/4/5% per skill point kills outright.
        return grng->random(100) < (masteryIndex(mastery) + 3) * s ? targetHp : 0;
    case 97: return grng->randomDice(s, 25);                    // Dragon Breath.
    case 98: return s + 50;                         // Armageddon.
    case 99: return s + 50;                         // Dark Containment.
    default: return 0;
    }
}

bool mm6SpellCanMiss(SpellId mm6Spell) {
    switch (std::to_underlying(mm6Spell)) {
    case 2:  // Flame Arrow.
    case 35: // Magic Arrow.
    case 39: // Blades.
    case 45: // Spirit Arrow.
        return true;
    default:
        return false;
    }
}

SpriteId mm6SpellSprite(SpellId mm6Spell) {
    int id = std::to_underlying(mm6Spell);
    if (id <= 0 || id > 99)
        return SPRITE_NULL;
    return static_cast<SpriteId>(1000 * ((id - 1) / 11 + 1) + 10 * ((id - 1) % 11));
}

SpellId mm6SpellEffect(SpellId mm6Spell) {
    int id = std::to_underlying(mm6Spell);
    if (id <= 0 || id >= static_cast<int>(mm6Effects.size()))
        return mm6Spell;
    return mm6Effects[id];
}

SpellId mm6SpellWithEffect(SpellId effect) {
    // Spells marked above as using the effect of another spell.
    static constexpr std::array<int, 18> borrowers = {2, 13, 24, 35, 42, 45, 47, 56, 59, 63, 71, 73, 75, 78, 79, 91, 95, 96};

    SpellId result = SPELL_NONE;
    for (int id = 1; id < static_cast<int>(mm6Effects.size()); id++) {
        if (mm6Effects[id] != effect)
            continue;
        if (std::find(borrowers.begin(), borrowers.end(), id) == borrowers.end())
            return static_cast<SpellId>(id);
        if (result == SPELL_NONE)
            result = static_cast<SpellId>(id);
    }
    return result;
}

DamageType mm7SpellEffectDamageType(SpellId effect) {
    int id = std::to_underlying(effect);
    if (id <= 0 || id >= static_cast<int>(mm7DamageTypes.size()))
        return DAMAGE_PHYSICAL;
    return mm7DamageTypes[id];
}

void setMm6SpellManaCost(SpellId mm6Spell, Mastery mastery, int mana) {
    int id = std::to_underlying(mm6Spell);
    if (id > 0 && id < static_cast<int>(mm6ManaCosts.size()) && mastery >= MASTERY_NOVICE && mastery <= MASTERY_GRANDMASTER)
        mm6ManaCosts[id][std::to_underlying(mastery) - std::to_underlying(MASTERY_NOVICE)] = mana;
}

int mm6SpellManaCost(SpellId mm6Spell, Mastery mastery) {
    int id = std::to_underlying(mm6Spell);
    if (id <= 0 || id >= static_cast<int>(mm6ManaCosts.size()) || mastery < MASTERY_NOVICE)
        return 0;
    if (mastery > MASTERY_MASTER)
        mastery = MASTERY_MASTER; // MM6 has no grandmaster.
    return mm6ManaCosts[id][std::to_underlying(mastery) - std::to_underlying(MASTERY_NOVICE)];
}

SpellId mm6MonsterSpellEffect(std::string_view name) {
    // Typos in MM6 monsters.txt.
    if (ascii::noCaseEquals(name, "Psychic Shockt"))
        name = "Psychic Shock";
    if (ascii::noCaseEquals(name, "Dispell Magic"))
        name = "Dispel Magic";
    for (size_t i = 1; i < mm6SpellNames.size(); i++)
        if (ascii::noCaseEquals(name, mm6SpellNames[i]))
            return mm6SpellEffect(static_cast<SpellId>(i));
    return SPELL_NONE;
}
