#pragma once

#include "CharacterEnums.h"
#include "ItemEnums.h"
#include "NPCEnums.h"
#include "SpriteEnums.h"

/**
 * MM6 numbers its items differently from MM7. Weapons, armor, wands and gold share ids, everything else is moved
 * into the MM7 range of the same category so that the category checks (`isPotion`, `isSpellScroll`, ...) work
 * for both games.
 *
 * @param mm6Id                         Item id as found in MM6 data files.
 * @return                              Item id used by the engine, `ITEM_NULL` for ids that MM6 doesn't use.
 */
ItemId itemIdFromMm6(int mm6Id);

/**
 * @param itemId                        Item id used by the engine.
 * @return                              Item id as found in MM6 data files, or 0 if there is none.
 */
int mm6IdFromItemId(ItemId itemId);

/**
 * MM6 skills are numbered like MM7 skills up to disarm traps, MM6 doesn't have the six MM7 skills that follow,
 * and learning comes right after disarm traps.
 *
 * @param mm6Id                         Skill id as found in MM6 data files.
 * @return                              Skill used by the engine, `SKILL_INVALID` for unknown ids.
 */
Skill skillFromMm6(int mm6Id);

/**
 * MM6 `dobjlist.bin` numbers its projectiles and traps differently from MM7, spell objects follow the MM6 spell
 * numbering, see `mm6SpellSprite`.
 *
 * @param sprite                        Sprite id as used by the engine.
 * @return                              Id of the MM6 object that looks the same, `sprite` itself if there is no
 *                                      difference.
 */
SpriteId mm6ObjectSprite(SpriteId sprite);

/**
 * @param mm6Id                         NPC profession id as found in MM6 data files, 1-77.
 * @return                              Profession used by the engine, `NoProfession` for unknown ids.
 */
NpcProfession npcProfessionFromMm6(int mm6Id);

/**
 * @param mm6Id                         NPC profession id as found in MM6 data files, 0-77.
 * @return                              Line in MM6 global.txt with the profession's name, -1 for unknown ids.
 */
int mm6NpcProfessionNameLine(int mm6Id);
