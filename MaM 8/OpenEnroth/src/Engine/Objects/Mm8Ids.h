#pragma once

#include <array>
#include <span>
#include <string>
#include <vector>

#include "Utility/Memory/Blob.h"

#include "Engine/Data/AwardEnums.h"

#include "Engine/Spells/SpellEnums.h"

#include "CharacterEnums.h"
#include "ItemEnums.h"
#include "MonsterEnums.h"

struct Item;

/**
 * MM8 numbers its items like MM7 from 200 on, below that its rings, amulets, wands, gems and gold sit elsewhere.
 * They are moved into the MM7 ranges of the same category, so that the category checks work for both games.
 *
 * @param mm8Id                         Item id as found in MM8 data files.
 * @return                              Item id used by the engine, `ITEM_NULL` for ids that MM8 doesn't use.
 */
ItemId itemIdFromMm8(int mm8Id);

/**
 * @param itemId                        Item id used by the engine.
 * @return                              Item id as found in MM8 data files, or 0 if there is none.
 */
int mm8IdFromItemId(ItemId itemId);

// MM8 artifacts. They take the ids of the MM7 artifacts, so the MM7 artifact checks must not run in MM8.
inline constexpr ItemId ITEM_MM8_ELSENRAIL = static_cast<ItemId>(500);
inline constexpr ItemId ITEM_MM8_GLOMENTHAL = static_cast<ItemId>(501);
inline constexpr ItemId ITEM_MM8_BORDERGUARD = static_cast<ItemId>(502);
inline constexpr ItemId ITEM_MM8_MACHETE_OF_JUSTICE = static_cast<ItemId>(503);
inline constexpr ItemId ITEM_MM8_ELDERS_AXE = static_cast<ItemId>(504);
inline constexpr ItemId ITEM_MM8_VOLCANO = static_cast<ItemId>(505);
inline constexpr ItemId ITEM_MM8_DRAGON_LANCE = static_cast<ItemId>(506);
inline constexpr ItemId ITEM_MM8_GUARDIAN = static_cast<ItemId>(507);
inline constexpr ItemId ITEM_MM8_CURVED_FANG = static_cast<ItemId>(508);
inline constexpr ItemId ITEM_MM8_SCEPTER_OF_KINGS = static_cast<ItemId>(509);
inline constexpr ItemId ITEM_MM8_CRUSHER = static_cast<ItemId>(510);
inline constexpr ItemId ITEM_MM8_SWAMP_STAFF = static_cast<ItemId>(511);
inline constexpr ItemId ITEM_MM8_LONG_RANGE_BOW = static_cast<ItemId>(512);
inline constexpr ItemId ITEM_MM8_SERENDINES_PRESERVATION = static_cast<ItemId>(513);
inline constexpr ItemId ITEM_MM8_ARMOR_OF_DARKNESS = static_cast<ItemId>(514);
inline constexpr ItemId ITEM_MM8_MASTERWORK_ARMOR = static_cast<ItemId>(515);
inline constexpr ItemId ITEM_MM8_ECLIPSE_SHIELD = static_cast<ItemId>(516);
inline constexpr ItemId ITEM_MM8_NIMBLE_GLOVES = static_cast<ItemId>(517);
inline constexpr ItemId ITEM_MM8_COURIERS_BOOTS = static_cast<ItemId>(518);
inline constexpr ItemId ITEM_MM8_RING_OF_PLANES = static_cast<ItemId>(519);
inline constexpr ItemId ITEM_MM8_DROGGS_HELM = static_cast<ItemId>(520);
inline constexpr ItemId ITEM_MM8_CROWN_OF_DOMINION = static_cast<ItemId>(521);
inline constexpr ItemId ITEM_MM8_ARCHANGEL_WINGS = static_cast<ItemId>(522);
inline constexpr ItemId ITEM_MM8_SNAKE = static_cast<ItemId>(523);
inline constexpr ItemId ITEM_MM8_SCOURGE = static_cast<ItemId>(524);
inline constexpr ItemId ITEM_MM8_SWORD_OF_LAW = static_cast<ItemId>(525);
inline constexpr ItemId ITEM_MM8_HELL_CLEAVER = static_cast<ItemId>(526);
inline constexpr ItemId ITEM_MM8_SOULSLAYER = static_cast<ItemId>(527);
inline constexpr ItemId ITEM_MM8_TRIDENT_OF_RULE = static_cast<ItemId>(528);
inline constexpr ItemId ITEM_MM8_BLADE_OF_MERCY = static_cast<ItemId>(529);
inline constexpr ItemId ITEM_MM8_ELEMENTAL_STAFF = static_cast<ItemId>(530);
inline constexpr ItemId ITEM_MM8_TOURNAMENT_BOW = static_cast<ItemId>(531);
inline constexpr ItemId ITEM_MM8_LIGHTNING_CROSSBOW = static_cast<ItemId>(532);
inline constexpr ItemId ITEM_MM8_PERFECT_CUIRASS = static_cast<ItemId>(533);
inline constexpr ItemId ITEM_MM8_HERONDALES_SHIELD = static_cast<ItemId>(534);
inline constexpr ItemId ITEM_MM8_RING_OF_FUSION = static_cast<ItemId>(535);
inline constexpr ItemId ITEM_MM8_LUCKY_HAT = static_cast<ItemId>(536);
inline constexpr ItemId ITEM_MM8_BERSERKERS_BELT = static_cast<ItemId>(537);

/**
 * Monster groups that MM8.exe 0x436FDC checks. MM8 numbers them differently from the MM7 supertypes and a monster can
 * be in more than one of them.
 */
enum class Mm8MonsterGroup {
    MM8_MONSTER_UNDEAD = 1,
    MM8_MONSTER_DRAGON = 2,
    MM8_MONSTER_SWIMMER = 3,        // Walks on water.
    MM8_MONSTER_IMMOBILE = 4,
    MM8_MONSTER_PEASANT = 5,
    MM8_MONSTER_NOT_IN_ARENA = 6,   // Swimmers, immobile monsters and peasants.
    MM8_MONSTER_OGRE = 7,           // Ogres, trolls and cyclopes.
    MM8_MONSTER_ELEMENTAL = 8,
};
using enum Mm8MonsterGroup;

/**
 * @param monsterId                     MM8 monster id.
 * @param group                         Monster group to check.
 * @return                              Whether the monster is in the group, MM8.exe 0x436FDC.
 */
bool mm8MonsterInGroup(MonsterId monsterId, Mm8MonsterGroup group);

/**
 * @param monsterId                     MM8 monster id.
 * @return                              Items that the monster drops when it dies, each with a chance of 20%, MM8.exe
 *                                      0x402F96. These are reagents and the monster parts that quests ask for.
 */
std::vector<ItemId> mm8DeathDrops(MonsterId monsterId);

/**
 * @param weapon                        Weapon to check.
 * @param monsterId                     MM8 monster id of the target.
 * @return                              Whether the weapon does double damage to the monster, MM8.exe 0x48E3CE.
 */
bool mm8WeaponSlays(const Item &weapon, MonsterId monsterId);

/**
 * MM8 numbers its awards differently from MM7. The engine gives the awards that count deaths, bounties, prison terms,
 * arena wins and Arcomage games itself, so these need their MM8 numbers.
 *
 * @param award                         MM7 award with a counter in its text.
 * @return                              Award to give in the loaded game, the MM8 one in MM8.
 */
AwardId counterAward(AwardId award);

/**
 * @param mm8Award                      MM8 award.
 * @return                              MM7 award with the same counter, `AWARD_INVALID` if the MM8 award has no
 *                                      counter in its text.
 */
AwardId counterAwardFromMm8(AwardId mm8Award);

/**
 * @param id                            Item id as found in the data files of the loaded game.
 * @return                              Item id used by the engine.
 */
ItemId itemIdFromData(int id);

/**
 * MM8 has eight class lines with one promotion each. They are stored in the class lines of the engine that MM8 doesn't
 * use, so that the base class and its promotion are next to each other like in MM7.
 *
 * @param mm8Id                         Class id as found in MM8 class.txt, 0-15.
 * @return                              Class used by the engine.
 */
Class classFromMm8(int mm8Id);

/**
 * @param cls                           Class used by the engine.
 * @return                              MM8 class id, -1 for classes that MM8 doesn't have.
 */
int mm8IdFromClass(Class cls);

/**
 * MM8 numbers its skills like MM7 up to dark magic, then come the three racial abilities of dark elves, vampires
 * and dragons. Diplomacy and thievery are gone and regeneration is new. A character has at most one racial ability,
 * it's stored as `SKILL_MM8_RACIAL`, regeneration as `SKILL_MM8_REGENERATION`.
 *
 * @param mm8Id                         Skill id as found in MM8 data files, 0-38.
 * @return                              Skill used by the engine, `SKILL_INVALID` for unknown ids.
 */
Skill skillFromMm8(int mm8Id);

inline constexpr Skill SKILL_MM8_RACIAL = SKILL_DIPLOMACY;

inline constexpr Skill SKILL_MM8_REGENERATION = SKILL_THIEVERY;
inline constexpr int MM8_SKILL_COUNT = 39;
inline constexpr int MM8_IMMUNE_RESISTANCE = 65000; // What MM8 gives as the resistance of a character that is immune.
inline constexpr int MM8_CLASS_COUNT = 16;

/**
 * Class lines of MM8, in the order of the starting skills and faces.
 */
enum class Mm8ClassKind {
    MM8_NECROMANCER = 0,
    MM8_CLERIC = 1,
    MM8_KNIGHT = 2,
    MM8_TROLL = 3,
    MM8_MINOTAUR = 4,
    MM8_DARK_ELF = 5,
    MM8_VAMPIRE = 6,
    MM8_DRAGON = 7,
};
using enum Mm8ClassKind;

/**
 * @param mm8Id                         Spell id as found in MM8 data files.
 * @return                              Spell used by the engine, `SPELL_NONE` for the unused MM8 ids.
 */
SpellId spellFromMm8(int mm8Id);

/**
 * @param kind                          MM8 class line.
 * @return                              The four racial abilities of the line, from the one learned first, empty for
 *                                      the lines without them.
 */
std::span<const SpellId> mm8RacialSpells(Mm8ClassKind kind);

/**
 * @param kind                          MM8 class line.
 * @return                              0 for dark elves, 1 for vampires, 2 for dragons, -1 for the lines without a
 *                                      racial ability.
 */
int mm8RacialLine(Mm8ClassKind kind);

/**
 * @param kind                          MM8 class line.
 * @return                              Base class of the line.
 */
Class mm8BaseClass(Mm8ClassKind kind);

/**
 * @param cls                           Class used by the engine, one of the MM8 classes.
 * @return                              MM8 class line of the class.
 */
Mm8ClassKind mm8ClassKind(Class cls);

/**
 * @param face                          Character portrait, 0-based, `pcNN` in icons.lod is face NN - 1.
 * @return                              Class line of the face. Faces 0-23 can be picked at the start, 24-25 are
 *                                      dragons and 26-27 liches.
 */
Mm8ClassKind mm8FaceClassKind(int face);

/**
 * @param face                          Character portrait, 0-based.
 * @return                              Whether the face is a woman.
 */
bool mm8FaceIsFemale(int face);

inline constexpr int MM8_STARTING_FACE_COUNT = 24;

/**
 * One stat of a class as MM8 starts a character with it.
 */
struct Mm8StartingStat {
    int base = 11;
    int max = 25;
    int spend = 1; // Bonus points it costs to raise the stat.
    int add = 1; // How much the stat grows for that.
};

/**
 * Class tables that MM8 keeps in MM8-Rel.exe.
 */
struct Mm8ClassData {
    int hpBase = 0;
    int hpPerLevel = 0;
    int spBase = 0;
    int spPerLevel = 0;
    std::array<Mm8StartingStat, 7> stats; // Might, intellect, personality, endurance, accuracy, speed, luck.
};

/**
 * Loads the MM8 class tables from MM8-Rel.exe into `mm8ClassData`, `mm8StartingSkills` and the engine's
 * `skillMaxMasteryPerClass`.
 */
void loadMm8ClassTables();

/**
 * @param pcNames                       Contents of MM8 pcnames.txt.
 */
void loadMm8CharacterNames(const Blob &pcNames);

/**
 * @param face                          Character portrait, 0-based.
 * @return                              Random name for a character with this face, from MM8 pcnames.txt.
 */
std::string mm8RandomCharacterName(int face);

/**
 * @param cls                           Class used by the engine.
 * @return                              MM8 data of the class.
 */
const Mm8ClassData &mm8ClassData(Class cls);

/**
 * @param kind                          MM8 class line.
 * @param skill                         Skill used by the engine.
 * @return                              2 if a new character of the line has the skill, 1 if it can pick it, 0 if not.
 */
int mm8StartingSkill(Mm8ClassKind kind, Skill skill);
