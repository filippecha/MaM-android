#include "Mm6NpcTalk.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/MapEnums.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/Mm6Ids.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/NPCTable.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/Mm6NpcTopics.h"
#include "GUI/UI/UIStatusBar.h"

namespace mm6_talk {

namespace {

constexpr int STATE_NEW = 0;
constexpr int STATE_MET = 1;
constexpr int STATE_BEGGED = 2;
constexpr int STATE_BRIBED = 3;
constexpr int STATE_THREATENED = 4;

constexpr int BEG = 0;
constexpr int BRIBE = 1;
constexpr int THREAT = 2;

void setNpcState(NPCData *npc, int state) {
    npc->flags = NpcFlags((static_cast<uint32_t>(npc->flags) & std::to_underlying(NPC_HIRED)) | state);
}

int personality(const NPCData *npc) {
    return pNPCStats->mm6Personality[npc->profession];
}

/**
 * @param npc                           NPC.
 * @param message                       Message number in npcbtb.txt, 1-24.
 * @return                              The message for the NPC's personality, with the placeholders filled in.
 */
std::string message(NPCData *npc, int message) {
    std::string text = pNPCStats->mm6Talk.messages[message][personality(npc)];
    if (text.empty() || text == "n/a")
        return {};
    std::string price = std::to_string(bribePrice());
    for (size_t pos = text.find("%04"); pos != std::string::npos; pos = text.find("%04", pos + price.size()))
        text.replace(pos, 3, price);
    return BuildDialogueString(text, pParty->hasActiveCharacter() ? pParty->activeCharacterIndex() : 0, npc);
}

bool isAllowed(const NPCData *npc, int action) {
    return pNPCStats->mm6Talk.allowed[action][personality(npc)];
}

/**
 * @param npc                           NPC.
 * @return                              Whether the NPC likes the party's reputation, MM6.exe 0x43AEE7.
 */
bool likesReputation(const NPCData *npc) {
    int reputation = mm6_topics::partyReputation();
    return npc->rep == 0 || (reputation > 0 && npc->rep > 0 && reputation > npc->rep) ||
           (reputation < 0 && npc->rep < 0 && reputation < npc->rep);
}

const NPCNewsMm6 *news(NPCData *npc) {
    if (!npc->newsTopic) {
        std::vector<int> candidates;
        for (int i = 0; i < pNPCStats->mm6News.size(); i++)
            if (pNPCStats->mm6News[i].mapId == newsGroup())
                candidates.push_back(i);
        if (candidates.empty())
            return nullptr;
        npc->newsTopic = candidates[grng->random(candidates.size())] + 1;
    }
    if (npc->newsTopic > pNPCStats->mm6News.size())
        return nullptr;
    return &pNPCStats->mm6News[npc->newsTopic - 1];
}

/**
 * Makes the NPC talk after a successful beg, threat or bribe.
 *
 * @param npc                           NPC.
 * @param state                         New state of the NPC.
 * @param baseCost                      Reputation that the action costs, the active character's diplomacy lowers it.
 */
void agree(NPCData *npc, int state, int baseCost) {
    setNpcState(npc, state);
    pParty->mm6Reputation -= std::max(0, baseCost - diplomacy(pParty->activeCharacter()));
}

} // namespace

int newsGroup() {
    int mm6Map = std::to_underlying(engine->_currentLoadedMapId) - std::to_underlying(MAP_FIRST_MM6) + 1;
    return mm6Map >= 1 && mm6Map <= 15 ? 25 + mm6Map : 1;
}

int npcState(const NPCData *npc) {
    return static_cast<uint32_t>(npc->flags) & 0x7F;
}

TalkKind talkKind(const NPCData *npc) {
    if (npc->Hired() || std::to_underlying(npc->house) < 0)
        return NORMAL;
    if (mm6_topics::partyFame() <= npc->fame)
        return LACK_FAME;
    switch (npcState(npc)) {
    case STATE_NEW:
    case STATE_MET:
        return likesReputation(npc) ? NORMAL : BEG_THREAT_BRIBE;
    case STATE_BEGGED:
        return THREAT_BRIBE;
    case STATE_BRIBED:
        return BEG_THREAT_BRIBE;
    default:
        return NORMAL;
    }
}

std::string greeting(NPCData *npc) {
    if (npc->Hired() || std::to_underlying(npc->house) < 0)
        return {};
    if (mm6_topics::partyFame() <= npc->fame)
        return message(npc, 6); // "Fame too low".

    int state = npcState(npc);
    if (state == STATE_BEGGED)
        return message(npc, 3);
    if (state == STATE_BRIBED)
        return message(npc, 4);
    if (state == STATE_THREATENED)
        return message(npc, 5);

    bool first = state == STATE_NEW;
    int reputation = mm6_topics::partyReputation();
    int wanted = npc->rep;
    if (likesReputation(npc)) {
        if (reputation <= -1000 && wanted < 0)
            return message(npc, 8); // "Rep notorious, I'm evil".
        if (reputation >= 1000 && wanted > 0)
            return message(npc, 9); // "Rep saintly, I'm good".
        return message(npc, first ? 1 : 2);
    }
    if (reputation <= -1000 && wanted > 0)
        return message(npc, 7); // "Rep notorious, I'm good".
    if (reputation >= 1000 && wanted < 0)
        return message(npc, 10); // "Rep saintly, I'm evil".
    if (reputation <= 0 && wanted > 0)
        return message(npc, first ? 11 : 15); // "Rep below zero".
    if (reputation >= 0 && wanted < 0)
        return message(npc, first ? 12 : 16); // "Rep above 10, I'm evil".
    if (reputation > 0 && wanted >= reputation)
        return message(npc, first ? 13 : 17); // "You aren't good enough for me".
    return message(npc, first ? 14 : 18); // "You aren't bad enough for me".
}

void markMet(NPCData *npc) {
    if (npcState(npc) == STATE_NEW)
        setNpcState(npc, STATE_MET);
}

int diplomacy(const Character &character) {
    CombinedSkillValue skill = character.getSkillValue(SKILL_DIPLOMACY);
    int level = skill.level();
    if (CheckHiredNPCSpeciality(npcProfessionFromMm6(23))) // Counselor.
        level += 4;
    if (CheckHiredNPCSpeciality(npcProfessionFromMm6(24))) // Barrister.
        level += 8;
    if (CheckHiredNPCSpeciality(npcProfessionFromMm6(49))) // Negotiator.
        level += 4;
    switch (skill.mastery()) {
    case MASTERY_MASTER: return level * 4;
    case MASTERY_EXPERT: return level * 3;
    default: return level * 2;
    }
}

int bribePrice() {
    int price = (100 - diplomacy(pParty->activeCharacter())) * (pParty->mm6Bribes + 1) * 50 / 100;
    return std::max(10, price);
}

std::string bribeOptionString() {
    return fmt::format("{} {} {}", localization->mm6Str(31), bribePrice(), localization->mm6Str(97)); // "Bribe 55 gold".
}

std::string beg(NPCData *npc, bool *success) {
    *success = false;
    if (npcState(npc) == STATE_BEGGED)
        return {};
    if (!isAllowed(npc, BEG)) {
        pParty->activeCharacter().playReaction(SPEECH_BEG_FAIL);
        return message(npc, 20); // "I don't like begging".
    }
    agree(npc, STATE_BEGGED, 10);
    *success = true;
    pParty->activeCharacter().playReaction(SPEECH_BEG);
    return message(npc, 19); // "I accept your beg".
}

std::string threaten(NPCData *npc, bool *success) {
    *success = false;
    if (!isAllowed(npc, THREAT)) {
        pParty->activeCharacter().playReaction(SPEECH_THREAT_FAIL);
        return message(npc, 24); // "I don't like threats".
    }
    agree(npc, STATE_THREATENED, 50);
    *success = true;
    pParty->activeCharacter().playReaction(SPEECH_THREAT);
    return message(npc, 23); // "I accept your threat".
}

std::string bribe(NPCData *npc, bool *success) {
    *success = false;
    if (!isAllowed(npc, BRIBE)) {
        pParty->activeCharacter().playReaction(SPEECH_BRIBE_FAIL);
        return message(npc, 22); // "I don't like bribes".
    }
    int price = bribePrice();
    if (pParty->GetGold() < price) {
        pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
        engine->_statusBar->setEvent(localization->mm6Str(155)); // "You don't have enough gold".
        return {};
    }
    pParty->TakeGold(price);
    pParty->mm6Bribes++;
    agree(npc, STATE_BRIBED, 20);
    *success = true;
    pParty->activeCharacter().playReaction(SPEECH_BRIBE);
    return message(npc, 21); // "Thanks for the bribe".
}

std::string dayTopic(const NPCData *npc) {
    return pNPCStats->mm6DayTopics[npc->profession][pParty->uCurrentDayOfMonth % 7];
}

std::string dayTopicText(const NPCData *npc) {
    return pNPCStats->mm6DayTexts[npc->profession][pParty->uCurrentDayOfMonth % 7];
}

std::string newsTopic(NPCData *npc) {
    const NPCNewsMm6 *item = news(npc);
    return item ? item->topic : std::string();
}

std::string newsText(NPCData *npc) {
    const NPCNewsMm6 *item = news(npc);
    return item ? item->text : std::string();
}

std::string reputationString(int reputation) {
    static constexpr std::array<std::pair<int, int>, 6> good = {{{1000, 510}, {800, 511}, {600, 512}, {400, 513}, {200, 514}, {0, 515}}};
    for (auto [threshold, line] : good)
        if (reputation >= threshold)
            return localization->mm6Str(line);
    static constexpr std::array<std::pair<int, int>, 4> bad = {{{-1000, 520}, {-800, 519}, {-600, 518}, {-300, 517}}};
    for (auto [threshold, line] : bad)
        if (reputation <= threshold)
            return localization->mm6Str(line);
    return localization->mm6Str(516);
}

} // namespace mm6_talk
