#pragma once

#include <string>

struct NPCData;
class Character;

/**
 * MM6 street NPC conversations. An NPC talks to the party only if the party is famous enough and has the reputation
 * the NPC likes, otherwise the party has to beg, threaten or bribe it. The texts come from npcbtb.txt, see
 * `Mm6NpcTalkTable`.
 */
namespace mm6_talk {

enum class TalkKind {
    NORMAL, // Topics of the NPC.
    LACK_FAME, // No topics, the party isn't famous enough.
    BEG_THREAT_BRIBE,
    THREAT_BRIBE, // The NPC was begged before, begging again does nothing.
};
using enum TalkKind;

/**
 * @return                              Group of npcnews.txt rumors told on the current map. MM6.exe 0x43BC20 uses
 *                                      groups 26-40 for the 15 outdoor maps and group 1 everywhere else.
 */
int newsGroup();

/**
 * @param npc                           NPC.
 * @return                              State of the NPC: 0 not met yet, 1 met, 2 begged, 3 bribed, 4 threatened.
 */
int npcState(const NPCData *npc);

/**
 * @param npc                           NPC the party talks to on the street, MM6.exe 0x43BF39.
 * @return                              Which topics the NPC offers.
 */
TalkKind talkKind(const NPCData *npc);

/**
 * @param npc                           NPC the party talks to on the street, MM6.exe 0x43AEB3.
 * @return                              What the NPC says when the conversation starts, empty if nothing.
 */
std::string greeting(NPCData *npc);

/**
 * Remembers that the party has met the NPC, so that the next greeting is the second one.
 *
 * @param npc                           NPC.
 */
void markMet(NPCData *npc);

/**
 * @param character                     Character that talks.
 * @return                              Diplomacy of the character with the bonuses of the hired NPCs, MM6.exe
 *                                      0x4852D0.
 */
int diplomacy(const Character &character);

/**
 * @return                              Price of a bribe for the active character, MM6.exe 0x4A4092.
 */
int bribePrice();

/**
 * @return                              Text of the bribe option.
 */
std::string bribeOptionString();

/**
 * Begs, threatens or bribes the NPC, MM6.exe 0x4A3E19, 0x4A3F16 and 0x4A4077.
 *
 * @param npc                           NPC.
 * @param[out] success                  Whether the NPC agreed to talk.
 * @return                              What the NPC answers, empty to show the greeting again.
 */
std::string beg(NPCData *npc, bool *success);
std::string threaten(NPCData *npc, bool *success);
std::string bribe(NPCData *npc, bool *success);

/**
 * @param npc                           NPC.
 * @return                              Topic of the day for the NPC's profession, from proftext.txt.
 */
std::string dayTopic(const NPCData *npc);
std::string dayTopicText(const NPCData *npc);

/**
 * Picks a news item of the current map for the NPC the first time it is asked, MM6.exe 0x43BC20.
 *
 * @param npc                           NPC.
 * @return                              Topic of the NPC's news.
 */
std::string newsTopic(NPCData *npc);
std::string newsText(NPCData *npc);

/**
 * @param reputation                    MM6 reputation.
 * @return                              Its name, MM6.exe 0x489C60.
 */
std::string reputationString(int reputation);

} // namespace mm6_talk
