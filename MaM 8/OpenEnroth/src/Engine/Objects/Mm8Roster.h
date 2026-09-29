#pragma once

#include <string>

#include "Utility/Memory/Blob.h"

class Character;

inline constexpr int MM8_ROSTER_SIZE = 50; // MM8 reads this many lines of roster.txt, line 0 is the main character.
inline constexpr int MM8_FIRST_ROSTER_EVENT = 600; // NPC topic events 600-649 offer roster characters 0-49 to join.

/**
 * Loads MM8 roster.txt, the characters that NPCs turn into when they join the party.
 *
 * @param roster                        Contents of roster.txt.
 */
void loadMm8Roster(const Blob &roster);

/**
 * Makes a roster character as MM8.exe 0x49680A does when a new game starts: stats, skills and spells from roster.txt,
 * and the equipment with random enchantments of the listed treasure level, worn where possible.
 *
 * @param rosterId                      Line in roster.txt, 1-49.
 * @param character                     Character to fill in.
 */
void createMm8RosterCharacter(int rosterId, Character *character);

/**
 * @param rosterId                      Line in roster.txt.
 * @return                              Blurb that the Adventurer's Inn shows for the character.
 */
const std::string &mm8RosterBlurb(int rosterId);

/**
 * @param rosterId                      Line in roster.txt.
 * @return                              Whether the character is in the party.
 */
bool isMm8RosterCharacterInParty(int rosterId);

enum class Mm8JoinResult {
    MM8_JOINED,
    MM8_JOIN_PARTY_FULL,
    MM8_JOIN_ALREADY_IN_PARTY,
};
using enum Mm8JoinResult;

/**
 * Adds a roster character to the party like MM8.exe 0x48DC48. A character that has been in the party before comes
 * back from the Adventurer's Inn as it left, the others are made from roster.txt.
 *
 * @param rosterId                      Line in roster.txt, 1-49.
 * @return                              What happened.
 */
Mm8JoinResult joinMm8RosterCharacter(int rosterId);

/**
 * Lets the Adventurer's Inn offer a roster character, MM8 does this for every character that was offered to join,
 * whatever the answer. A character that isn't in the party waits in `Party::mm8InnCharacters`.
 *
 * @param rosterId                      Line in roster.txt, 1-49.
 */
void sendMm8RosterCharacterToInn(int rosterId);

/**
 * Sends a party member to the Adventurer's Inn.
 *
 * @param partyIndex                    Index of the character in the party, the main character can't leave.
 */
void dismissMm8RosterCharacter(int partyIndex);
