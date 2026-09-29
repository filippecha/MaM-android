#pragma once

#include <array>
#include <string>

#include "CharacterEnums.h"

// MM6 resistances live in the MM7 attributes: fire -> ATTRIBUTE_RESIST_FIRE, electricity -> ATTRIBUTE_RESIST_AIR,
// cold -> ATTRIBUTE_RESIST_WATER, poison -> ATTRIBUTE_RESIST_BODY, magic -> ATTRIBUTE_RESIST_MIND.

/**
 * MM6 has 12 portraits: eight male faces followed by four female ones. The voice always matches the face.
 */
inline constexpr int MM6_PORTRAIT_COUNT = 12;
inline constexpr int MM6_FIRST_FEMALE_PORTRAIT = 8;

/**
 * @param face                          MM6 portrait index, 0-11.
 * @return                              Texture name prefix of the portrait in icons.lod, e.g. "malea". Frame
 *                                      textures are named `<prefix><frame number>`, e.g. "malea01".
 */
const char *mm6PortraitName(int face);

/**
 * @param face                          MM6 portrait index, 0-11.
 * @return                              Sex of the character with the given portrait.
 */
Sex mm6SexForPortrait(int face);

/**
 * MM6 has six base classes, each with two promotions. They are stored in the MM7 slots with the same meaning.
 *
 * @param mm6Id                         Class index in MM6 class.txt, 0-17.
 * @return                              Engine class.
 */
Class classFromMm6(int mm6Id);

/**
 * @param cls                           Any class of a MM6 class line.
 * @return                              Skill affinity at party creation: `SKILL_AFFINITY_PRIMARY` for the two starting
 *                                      skills, `SKILL_AFFINITY_AVAILABLE` for the skills offered at party creation.
 */
SkillAffinity mm6CreationSkillAffinity(Class cls, Skill skill);

/**
 * @param cls                           Any class of a MM6 class line.
 * @param attribute                     One of the seven stats.
 * @return                              Starting value of the stat.
 */
int mm6StartingStat(Class cls, Attribute attribute);

/**
 * MM6 health is `base + perLevel * (level + endurance bonus)`, mana is the same with the class's mana stats, exactly
 * like in MM7 but with different tables.
 *
 * @param cls                           Any class of a MM6 class line.
 * @return                              Base health of the class line.
 */
int mm6BaseHealth(Class cls);

/**
 * @param cls                           Any class of a MM6 class line.
 * @return                              Health per level of the class.
 */
int mm6HealthPerLevel(Class cls);

/**
 * @param cls                           Any class of a MM6 class line.
 * @return                              Base mana of the class line.
 */
int mm6BaseMana(Class cls);

/**
 * @param cls                           Any class of a MM6 class line.
 * @return                              Mana per level of the class.
 */
int mm6ManaPerLevel(Class cls);
