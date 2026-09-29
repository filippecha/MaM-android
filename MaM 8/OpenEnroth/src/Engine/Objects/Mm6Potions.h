#pragma once

class Blob;
class Character;

/**
 * MM6 herbs and potions, items 160-188 in MM6 numbering. They mix and work differently from the MM7 ones, and MM6
 * has no alchemy skill.
 */
namespace mm6_potions {

inline constexpr int FIRST_ITEM = 160;
inline constexpr int BOTTLE = 163;
inline constexpr int LAST_ITEM = 188;

/**
 * Loads the mixing table from useitems.txt, MM6.exe 0x44653E.
 *
 * @param useItems                      Contents of useitems.txt.
 */
void loadMixTable(const Blob &useItems);

/**
 * @param held                          MM6 id of the item in the hand.
 * @param target                        MM6 id of the item it is dropped on.
 * @return                              MM6 id of the resulting potion, 1-4 for an explosion of that strength, 0 if
 *                                      the two don't mix. See MM6.exe 0x410B16.
 */
int mix(int held, int target);

/**
 * Applies a herb or potion to a character, MM6.exe 0x459097.
 *
 * @param character                     Character that drinks or eats.
 * @param mm6Id                         MM6 id of the item.
 * @return                              Whether the item was used up.
 */
bool drink(Character *character, int mm6Id);

} // namespace mm6_potions
