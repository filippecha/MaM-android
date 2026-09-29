#pragma once

#include "Engine/Objects/CharacterEnums.h"

#include "GUI/GUIEnums.h"

#include "Library/Geometry/Point.h"

class Character;

/**
 * Handles the messages of the MM8 party creation screen that work differently from MM7.
 *
 * @param message                       Message from the queue.
 * @param param                         Its first parameter.
 * @return                              Whether the message was handled.
 */
bool handleMm8CreationMessage(UIMessageType message, int param);

/**
 * Makes `character` the character that the MM8 party creation screen starts with, a female knight with her stats
 * already spent.
 */
void createMm8DefaultCharacter(Character &character);

/**
 * @param character                     The character being created.
 * @param pos                           Screen position.
 * @param[out] known                    Whether the character has the skill, then its popup lists the masteries.
 * @return                              Skill whose name is at `pos` on the MM8 party creation screen, `SKILL_INVALID`
 *                                      if there is none.
 */
Skill mm8CreationSkillAt(const Character &character, Pointi pos, bool *known);

/**
 * Gives a freshly created MM8 character the items of its skills.
 */
void giveMm8StartingItems(Character &character);
