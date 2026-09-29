#pragma once

#include <optional>
#include <string_view>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/SpriteEnums.h"
#include "Core/Time/Duration.h"
#include "Engine/Spells/SpellEnums.h"

/**
 * MM6 spells keep their MM6 ids everywhere the player sees them (spellbook, guilds, learned spells, quick spell), and
 * are translated into an engine spell effect when cast. Effects are MM7 spells with the same or the closest
 * behavior.
 *
 * @param mm6Spell                      MM6 spell id, 1-99.
 * @return                              Engine spell effect.
 */
SpellId mm6SpellEffect(SpellId mm6Spell);

/**
 * @param effect                        Engine spell effect.
 * @return                              MM6 spell that has this effect as its own, and not only borrows it, or
 *                                      `SPELL_NONE`.
 */
SpellId mm6SpellWithEffect(SpellId effect);

// MM6 spells without a MM7 counterpart. They are cast through a carrier effect with the same targeting, the caster
// checks the MM6 spell id first.
inline constexpr SpellId MM6_SPELL_CREATE_FOOD = static_cast<SpellId>(78); // Carried by Day of the Gods.
inline constexpr SpellId MM6_SPELL_GOLDEN_TOUCH = static_cast<SpellId>(79); // Carried by Enchant Item.

/**
 * @param effect                        Engine spell effect, i.e. a MM7 spell.
 * @return                              Damage type the effect deals, as listed in MM7 spells.txt.
 */
DamageType mm7SpellEffectDamageType(SpellId effect);

void setMm6SpellManaCost(SpellId mm6Spell, Mastery mastery, int mana);

/**
 * @param mm6Spell                      MM6 spell id.
 * @param mastery                       Caster's mastery, grandmaster counts as master.
 * @return                              Mana cost from MM6 spells.txt.
 */
int mm6SpellManaCost(SpellId mm6Spell, Mastery mastery);

/**
 * Loads the hard-coded MM6 spell recovery times from MM6.exe. Must be called after `mm6ExeData` is loaded.
 */
void loadMm6SpellRecovery();

/**
 * @param mm6Spell                      MM6 spell id.
 * @param mastery                       Caster's mastery, grandmaster counts as master.
 * @return                              Recovery time from MM6.exe, or `std::nullopt` if it wasn't loaded.
 */
std::optional<Duration> mm6SpellRecovery(SpellId mm6Spell, Mastery mastery);

/**
 * @offset 0x47F0A0 in MM6.exe.
 *
 * @param mm6Spell                      MM6 attack spell id.
 * @param skill                         Caster's skill level.
 * @param mastery                       Caster's mastery.
 * @param targetHp                      Current hit points of the target.
 * @return                              Damage dealt, 0 for spells that deal no damage.
 */
int mm6SpellDamage(SpellId mm6Spell, int skill, Mastery mastery, int targetHp);

/**
 * @return                              Whether the spell uses a to-hit roll like an arrow instead of always hitting.
 */
bool mm6SpellCanMiss(SpellId mm6Spell);

/**
 * @return                              MM6 `dobjlist.bin` id of the projectile of the given MM6 spell. MM6 numbers its
 *                                      spell objects as `1000 * school + 10 * spell index in the school`.
 */
SpriteId mm6SpellSprite(SpellId mm6Spell);

/**
 * @param name                          Spell name as used in MM6 monsters.txt, e.g. "Flame Arrow".
 * @return                              Engine spell effect, `SPELL_NONE` for unknown names.
 */
SpellId mm6MonsterSpellEffect(std::string_view name);
