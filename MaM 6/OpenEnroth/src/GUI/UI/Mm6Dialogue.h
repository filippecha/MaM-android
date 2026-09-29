#pragma once

#include <string>
#include <vector>

#include "Library/Color/Color.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Size.h"

class GUIFont;

/**
 * Layout of the MM6 dialogue panel on the right side of the screen, used in houses and in street conversations.
 * Positions were measured on the original game.
 */
namespace mm6_dialogue {

inline constexpr Pointi PANEL_POS = {481, 0}; // evpanNNN.
inline constexpr int STREET_PANEL = 19; // MM6.exe 0x43BEAB, street conversations always use evpan019.
inline constexpr int TRAVEL_PANEL = 4; // MM6.exe 0x43A4F1, walking to another map uses evpan004.
inline constexpr Pointi YES_BUTTON_POS = {486, 313}; // MM6.exe 0x43A3AA, BUTTYES1.
inline constexpr Pointi NO_BUTTON_POS = {566, 313}; // MM6.exe 0x43A392, BUTTESC1.
inline constexpr Pointi EXIT_BUTTON_POS = {526, 313};
inline constexpr Sizei EXIT_BUTTON_SIZE = {61, 28};
inline constexpr int TITLE_Y = 2;
inline constexpr int NAME_Y = 108; // Name under the first portrait.
inline constexpr Pointi PORTRAIT_SHIFT = {4, -4}; // Relative to the MM7 portrait positions.
inline constexpr Color NAME_COLOR = Color(16, 154, 239);
inline constexpr Color OPTION_HIGHLIGHT_COLOR = Color(255, 255, 156);

/**
 * @param textHeights                   Height of every option's text, as returned by `GUIFont::CalcTextHeight` for
 *                                      the arrus font.
 * @param areaTop                       Top of the space for the options, below the portrait by default.
 * @return                              Y coordinate of every option. MM6 spreads the options evenly over the space
 *                                      between `areaTop` and the exit button.
 */
std::vector<int> optionPositions(const std::vector<int> &textHeights, int areaTop = 125);

/**
 * @param panel                         Panel number, e.g. 15 for evpan015.
 * @return                              Name of the panel image in icons.lod.
 */
std::string panelName(int panel);

} // namespace mm6_dialogue
