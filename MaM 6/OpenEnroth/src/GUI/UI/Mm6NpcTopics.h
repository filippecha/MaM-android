#pragma once

#include <string>
#include <utility>

#include "Engine/Data/AwardEnums.h"
#include "Engine/Data/HouseEnums.h"
#include "Engine/Objects/CharacterEnums.h"

/**
 * NPC topics that MM6.exe handles itself instead of running a global event, see MM6.exe 0x4A4205. Their event ids
 * differ from the MM7 ones.
 */
namespace mm6_topics {

inline constexpr int FIRST_TEACHER_EVENT = 200; // Expert and master teachers, two events per skill.
inline constexpr int LAST_TEACHER_EVENT = 259;
inline constexpr int FIRST_GUILD_EVENT = 381; // Guild memberships, in the order of MM6 awards 64-80.
inline constexpr int LAST_GUILD_EVENT = 397;
inline constexpr int SEER_PILGRIMAGE_EVENT = 41;
inline constexpr int SEER_LOST_ITEMS_EVENT = 45;
inline constexpr int BOUNTY_HUNT_EVENT = 399;
inline constexpr int ARENA_EVENT = 400;

/**
 * @param eventId                       Teacher event, 200-259.
 * @return                              Skill and mastery that the teacher teaches.
 */
std::pair<Skill, Mastery> teacherSkill(int eventId);

/**
 * Checks whether the active character can learn from a teacher, MM6.exe 0x496C90.
 *
 * @param eventId                       Teacher event, 200-259.
 * @param[out] price                    Price of the lesson.
 * @param[out] approved                 Whether the character can take the lesson.
 * @return                              Text of the dialogue option, the offer or the reason for a refusal.
 */
std::string teacherOptionString(int eventId, int *price, bool *approved);

/**
 * @param eventId                       Guild membership event, 381-397.
 * @return                              Award of the guild membership.
 */
AwardId guildMembershipAward(int eventId);

/**
 * Checks whether the active character can join a guild, MM6.exe 0x4978F9.
 *
 * @param eventId                       Guild membership event, 381-397.
 * @param[out] price                    Price of the membership.
 * @param[out] approved                 Whether the character can join.
 * @return                              Text of the dialogue option, the offer or the reason for a refusal.
 */
std::string joinGuildOptionString(int eventId, int *price, bool *approved);

/**
 * Talks to a town hall clerk about the monthly monster hunt, MM6.exe 0x4A31A5. Starts a new hunt at the start of a
 * month and pays the reward if the monster is dead.
 *
 * @param house                         MM6 town hall, houses 89-91.
 * @return                              Text of the clerk.
 */
std::string bountyHunt(HouseId house);

/**
 * The seer's pilgrimage of the month, MM6.exe 0x4A2F20 and 0x49784A. A new month clears the pilgrimage quest bits.
 *
 * @return                              Text of the seer.
 */
std::string seerPilgrimage();

/**
 * Gives back a quest item that the party has lost, MM6.exe 0x496570.
 *
 * @return                              Text of the seer.
 */
std::string seerLostItems();

/**
 * @return                              Party fame as MM6.exe 0x485510 computes it, the experience of the party in
 *                                      thousands.
 */
int partyFame();

/**
 * @return                              Reputation of the party with the bonuses of the hired NPCs, MM6.exe 0x47D600.
 *                                      Higher is better in MM6.
 */
int partyReputation();

} // namespace mm6_topics
