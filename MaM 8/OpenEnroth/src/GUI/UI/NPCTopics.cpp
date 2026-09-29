#include "NPCTopics.h"

#include <algorithm>
#include <array>
#include <ranges>
#include <tuple>
#include <utility>
#include <string>
#include <vector>

#include "Engine/ArenaEnumFunctions.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/Mm8Ids.h"
#include "Engine/Objects/Mm8Roster.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Party.h"
#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Random/Random.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/Mm6NpcTalk.h"
#include "GUI/UI/Mm6NpcTopics.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"
#include "Utility/String/Ascii.h"

int membershipOrTrainingApproved;
int topicEventId; // event id of currently viewed scripted NPC event
DialogueId guildMembershipNPCTopicId;

int gold_transaction_amount;

static constexpr std::array<Vec2f, 20> pMonsterArenaPlacements = {{
    Vec2f(1524, 8332),    Vec2f(2186, 8844),
    Vec2f(3219, 9339),    Vec2f(4500, 9339),
    Vec2f(5323, 9004),    Vec2f(0x177D, 0x2098),
    Vec2f(0x50B, 0x1E15), Vec2f(0x18FF, 0x1E15),
    Vec2f(0x50B, 0xD69),  Vec2f(0x18FF, 0x1B15),
    Vec2f(0x50B, 0x1021), Vec2f(0x18FF, 0x1848),
    Vec2f(0x50B, 0x12D7), Vec2f(0x18FF, 0x15A3),
    Vec2f(0x50B, 0x14DB), Vec2f(0x18FF, 0x12D7),
    Vec2f(0x50B, 0x1848), Vec2f(0x18FF, 0x1021),
    Vec2f(0x50B, 0x1B15), Vec2f(0x18FF, 0xD69),
}};

static constexpr IndexedArray<int, GUILD_FIRST, GUILD_LAST> priceForMembership = {{
    {GUILD_OF_ELEMENTS, 100},
    {GUILD_OF_SELF,     100},
    {GUILD_OF_AIR,      50},
    {GUILD_OF_EARTH,    50},
    {GUILD_OF_FIRE,     50},
    {GUILD_OF_WATER,    50},
    {GUILD_OF_BODY,     50},
    {GUILD_OF_MIND,     50},
    {GUILD_OF_SPIRIT,   50},
    {GUILD_OF_LIGHT,    1000},
    {GUILD_OF_DARK,     1000}
}};

static constexpr IndexedArray<int, SKILL_FIRST, SKILL_LAST> expertSkillMasteryCost = {{
    {SKILL_STAFF,        2000},
    {SKILL_SWORD,        2000},
    {SKILL_DAGGER,       2000},
    {SKILL_AXE,          2000},
    {SKILL_SPEAR,        2000},
    {SKILL_BOW,          2000},
    {SKILL_MACE,         2000},
    {SKILL_BLASTER,      0},
    {SKILL_SHIELD,       1000},
    {SKILL_LEATHER,      1000},
    {SKILL_CHAIN,        1000},
    {SKILL_PLATE,        1000},
    {SKILL_FIRE,         1000},
    {SKILL_AIR,          1000},
    {SKILL_WATER,        1000},
    {SKILL_EARTH,        1000},
    {SKILL_SPIRIT,       1000},
    {SKILL_MIND,         1000},
    {SKILL_BODY,         1000},
    {SKILL_LIGHT,        2000},
    {SKILL_DARK,         2000},
    {SKILL_ITEM_ID,      500},
    {SKILL_MERCHANT,     2000},
    {SKILL_REPAIR,       500},
    {SKILL_BODYBUILDING, 500},
    {SKILL_MEDITATION,   500},
    {SKILL_PERCEPTION,   500},
    {SKILL_DIPLOMACY,    0}, // not used
    {SKILL_THIEVERY,     0}, // not used
    {SKILL_TRAP_DISARM,  500},
    {SKILL_DODGE,        2000},
    {SKILL_UNARMED,      2000},
    {SKILL_MONSTER_ID,   500},
    {SKILL_ARMSMASTER,   2000},
    {SKILL_STEALING,     500},
    {SKILL_ALCHEMY,      500},
    {SKILL_LEARNING,     2000},
    {SKILL_CLUB,         500},
    {SKILL_MISC,         0} // hidden, not used
}};

static constexpr IndexedArray<int, SKILL_FIRST, SKILL_LAST> masterSkillMasteryCost = {{
    {SKILL_STAFF,        5000},
    {SKILL_SWORD,        5000},
    {SKILL_DAGGER,       5000},
    {SKILL_AXE,          5000},
    {SKILL_SPEAR,        5000},
    {SKILL_BOW,          5000},
    {SKILL_MACE,         5000},
    {SKILL_BLASTER,      0},
    {SKILL_SHIELD,       3000},
    {SKILL_LEATHER,      3000},
    {SKILL_CHAIN,        3000},
    {SKILL_PLATE,        3000},
    {SKILL_FIRE,         4000},
    {SKILL_AIR,          4000},
    {SKILL_WATER,        4000},
    {SKILL_EARTH,        4000},
    {SKILL_SPIRIT,       4000},
    {SKILL_MIND,         4000},
    {SKILL_BODY,         4000},
    {SKILL_LIGHT,        5000},
    {SKILL_DARK,         5000},
    {SKILL_ITEM_ID,      2500},
    {SKILL_MERCHANT,     5000},
    {SKILL_REPAIR,       2500},
    {SKILL_BODYBUILDING, 2500},
    {SKILL_MEDITATION,   2500},
    {SKILL_PERCEPTION,   2500},
    {SKILL_DIPLOMACY,    0}, // not used
    {SKILL_THIEVERY,     0}, // not used
    {SKILL_TRAP_DISARM,  2500},
    {SKILL_DODGE,        5000},
    {SKILL_UNARMED,      5000},
    {SKILL_MONSTER_ID,   2500},
    {SKILL_ARMSMASTER,   5000},
    {SKILL_STEALING,     2500},
    {SKILL_ALCHEMY,      2500},
    {SKILL_LEARNING,     5000},
    {SKILL_CLUB,         2500},
    {SKILL_MISC,         0} // hidden, not used
}};

static constexpr IndexedArray<int, SKILL_FIRST, SKILL_LAST> grandmasterSkillMasteryCost = {{
    {SKILL_STAFF,        8000},
    {SKILL_SWORD,        8000},
    {SKILL_DAGGER,       8000},
    {SKILL_AXE,          8000},
    {SKILL_SPEAR,        8000},
    {SKILL_BOW,          8000},
    {SKILL_MACE,         8000},
    {SKILL_BLASTER,      0},
    {SKILL_SHIELD,       7000},
    {SKILL_LEATHER,      7000},
    {SKILL_CHAIN,        7000},
    {SKILL_PLATE,        7000},
    {SKILL_FIRE,         8000},
    {SKILL_AIR,          8000},
    {SKILL_WATER,        8000},
    {SKILL_EARTH,        8000},
    {SKILL_SPIRIT,       8000},
    {SKILL_MIND,         8000},
    {SKILL_BODY,         8000},
    {SKILL_LIGHT,        8000},
    {SKILL_DARK,         8000},
    {SKILL_ITEM_ID,      6000},
    {SKILL_MERCHANT,     8000},
    {SKILL_REPAIR,       6000},
    {SKILL_BODYBUILDING, 6000},
    {SKILL_MEDITATION,   6000},
    {SKILL_PERCEPTION,   6000},
    {SKILL_DIPLOMACY,    0}, // not used
    {SKILL_THIEVERY,     0}, // not used
    {SKILL_TRAP_DISARM,  6000},
    {SKILL_DODGE,        8000},
    {SKILL_UNARMED,      8000},
    {SKILL_MONSTER_ID,   6000},
    {SKILL_ARMSMASTER,   8000},
    {SKILL_STEALING,     6000},
    {SKILL_ALCHEMY,      6000},
    {SKILL_LEARNING,     8000},
    {SKILL_CLUB,         6000},
    {SKILL_MISC,         0} // hidden, not used
}};

static constexpr std::array<std::pair<QuestBit, ItemId>, 27> _4F0882_evt_VAR_PlayerItemInHands_vals = {{
    {QBIT_212, ITEM_QUEST_VASE},
    {QBIT_213, ITEM_SPECIAL_LADY_CARMINES_DAGGER},
    {QBIT_214, ITEM_MESSAGE_SCROLL_OF_WAVES},
    {QBIT_215, ITEM_MESSAGE_CIPHER},
    {QBIT_216, ITEM_QUEST_WORN_BELT},
    {QBIT_217, ITEM_QUEST_HEART_OF_THE_WOOD},
    {QBIT_218, ITEM_MESSAGE_MAP_TO_EVENMORN_ISLAND},
    {QBIT_219, ITEM_QUEST_GOLEM_HEAD},
    {QBIT_220, ITEM_QUEST_ABBEY_NORMAL_GOLEM_HEAD},
    {QBIT_221, ITEM_QUEST_GOLEM_RIGHT_ARM},
    {QBIT_222, ITEM_QUEST_GOLEM_LEFT_ARM},
    {QBIT_223, ITEM_QUEST_GOLEM_RIGHT_LEG},
    {QBIT_224, ITEM_QUEST_GOLEM_LEFT_LEG},
    {QBIT_225, ITEM_QUEST_GOLEM_CHEST},
    {QBIT_226, ITEM_SPELLBOOK_DIVINE_INTERVENTION},
    {QBIT_227, ITEM_QUEST_DRAGON_EGG},
    {QBIT_228, ITEM_QUEST_ZOKARR_IVS_SKULL},
    {QBIT_229, ITEM_QUEST_LICH_JAR_EMPTY},
    {QBIT_230, ITEM_QUEST_ELIXIR},
    {QBIT_231, ITEM_QUEST_CASE_OF_SOUL_JARS},
    {QBIT_232, ITEM_QUEST_ALTAR_PIECE_1},
    {QBIT_233, ITEM_QUEST_ALTAR_PIECE_2},
    {QBIT_234, ITEM_QUEST_CONTROL_CUBE},
    {QBIT_235, ITEM_QUEST_WETSUIT},
    {QBIT_236, ITEM_QUEST_OSCILLATION_OVERTHRUSTER},
    {QBIT_237, ITEM_QUEST_LICH_JAR_FULL},
    {QBIT_241, ITEM_SPECIAL_THE_PERFECT_BOW}
}};

static void teleportPartyToArena() {
    pParty->pos = Vec3f(3849, 5770, 1);
    pParty->velocity = Vec3f();
    pParty->uFallStartZ = 1;
    pParty->_viewYaw = 512;
    pParty->_viewPitch = 0;
}

DialogueId arenaMainDialogue() {
    if (pParty->arenaState == ARENA_STATE_INITIAL)
        return DIALOGUE_ARENA_WELCOME;

    if (pParty->arenaState == ARENA_STATE_WON)
        return DIALOGUE_ARENA_ALREADY_WON;

    assert(pParty->arenaState == ARENA_STATE_FIGHTING);

    int killedMonsters = 0;
    for (Actor &actor : pActors) {
        if (actor.aiState == Dead ||
            actor.aiState == Removed ||
            actor.aiState == Disabled ||
            (actor.summonerId && actor.summonerId.type() == OBJECT_Character)) {
            killedMonsters++;
        }
    }

    if (killedMonsters >= pActors.size() || pActors.size() <= 0) {
        pParty->uNumArenaWins[pParty->arenaLevel]++;
        for (Character &player : pParty->pCharacters) {
            player.giveAward(counterAward(awardForArenaLevel(pParty->arenaLevel)));
        }
        pParty->partyFindsGold(gold_transaction_amount, GOLD_RECEIVE_SHARE);
        pAudioPlayer->playUISound(SOUND_51heroism03);
        pParty->arenaState = ARENA_STATE_WON;
        pParty->arenaLevel = ARENA_LEVEL_INVALID;
        return DIALOGUE_ARENA_REWARD;
    } else {
        teleportPartyToArena();
        pAudioPlayer->playUISound(SOUND_51heroism03);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return DIALOGUE_NULL;
    }
}

/**
 * @offset 0x4BC109
 */
void prepareArenaFight(ArenaLevel level) {
    pParty->arenaState = ARENA_STATE_FIGHTING;
    pParty->arenaLevel = level;

    // TODO(pskelton): This doesnt work properly and we dont want draw calls here
    render->BeginScene3D();
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        pOutdoor->Draw();
    }
    render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    render->BeginScene2D();
    pDialogueWindow->DrawDialoguePanel(localization->str(LSTR_PLEASE_WAIT_WHILE_I_SUMMON_THE_MONSTERS));
    render->Present();

    teleportPartyToArena();
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);

    int characterMaxLevel = std::ranges::max(pParty->pCharacters | std::views::transform(&Character::GetActualLevel));
    int monsterMinLevel = std::clamp(characterMaxLevel / 2, 2, 100);
    int monsterMaxLevel = std::clamp(static_cast<int>(characterMaxLevel * monsterLevelMultiplierForArenaLevel(level)), 2, 100);

    std::vector<MonsterId> candidateIds;
    if (isMm8()) {
        // MM8.exe 0x4BCAC0 skips the wimps and the monsters that can't be in the arena.
        for (MonsterId i : allMonsters()) {
            const MonsterInfo &info = pMonsterStats->infos[i];
            if (!info.name.empty() && info.aiType != MONSTER_AI_WIMP && !mm8MonsterInGroup(i, MM8_MONSTER_NOT_IN_ARENA) &&
                info.level >= monsterMinLevel && info.level <= monsterMaxLevel)
                candidateIds.push_back(i);
        }
    } else if (isMm6()) {
        // MM6.exe 0x4A3965 takes every monster in the level range, and the lord arena has no weak monsters.
        monsterMinLevel = std::clamp(level == ARENA_LEVEL_LORD ? characterMaxLevel : characterMaxLevel / 2, 1, 100);
        for (MonsterId i : allMonsters())
            if (!pMonsterStats->infos[i].name.empty() && pMonsterStats->infos[i].level >= monsterMinLevel &&
                pMonsterStats->infos[i].level <= monsterMaxLevel)
                candidateIds.push_back(i);
    } else {
        for (MonsterId i : allArenaMonsters()) {
            if (pMonsterStats->infos[i].level >= monsterMinLevel && pMonsterStats->infos[i].level <= monsterMaxLevel) {
                candidateIds.push_back(i);
            }
        }
    }
    assert(!candidateIds.empty());

    int maxIdsNum = std::min(6, static_cast<int>(candidateIds.size()));

    std::vector<MonsterId> monsterIds;
    for (int i = 0; i < maxIdsNum; i++) {
        monsterIds.push_back(grng->randomSample(candidateIds));
    }

    int baseReward = 0, monstersNum = 0;

    if (level == ARENA_LEVEL_PAGE) {
        baseReward = 50;
        monstersNum = grng->random(3) + 6; // [6:8] monsters
    } else if (level == ARENA_LEVEL_SQUIRE) {
        baseReward = 100;
        monstersNum = grng->random(7) + 6; // [6:12] monsters
    } else if (level == ARENA_LEVEL_KNIGHT) {
        baseReward = 200;
        monstersNum = grng->random(11) + 10; // [10:19] monsters
    } else if (level == ARENA_LEVEL_LORD) {
        baseReward = 500;
        monstersNum = 20;
    }

    gold_transaction_amount = characterMaxLevel * baseReward;

    for (int i = 0; i < monstersNum; ++i) {
        Vec2f pos = pMonsterArenaPlacements[i];
        Actor::Arena_summon_actor(grng->randomSample(monsterIds), Vec3f(pos.x, pos.y, 1));
    }
    pAudioPlayer->playUISound(SOUND_51heroism03);
}

/**
 * @offset 0x004B1ECE.
 *
 * @brief Oracle's 'I lost it!' dialog option
 */
void oracleDialogue() {
    ItemId item_id = ITEM_NULL;

    // display "You never had it" if nothing missing will be found
    current_npc_text = pNPCTopics[667].pText;

    // only items with special subquest in range 212-237 and also 241 are recoverable
    for (auto pair : _4F0882_evt_VAR_PlayerItemInHands_vals) {
        QuestBit quest_id = pair.first;
        if (pParty->_questBits[quest_id]) {
            ItemId search_item_id = pair.second;
            if (!pParty->hasItem(search_item_id) && pParty->pPickedItem.itemId != search_item_id) {
                item_id = search_item_id;
                break;
            }
        }
    }

    // missing item found
    if (item_id != ITEM_NULL) {
        Item item;
        item.itemId = item_id;
        item.flags = ITEM_IDENTIFIED;
        item.postGenerate(ITEM_SOURCE_SCRIPT);
        pParty->setHoldingItem(item);
        // TODO(captainurist): what if fmt throws?
        current_npc_text = fmt::sprintf(pNPCTopics[666].pText, // "Here's %s that you lost. Be careful" // NOLINT: this is not ::sprintf.
                                        fmt::format("{::}{}\f00000", colorTable.Sunflower.tag(),
                                                    pItemTable->items[item_id].unidentifiedName));
    }

    // missing item is lich jar and we need to bind soul vessel to lich class character
    // TODO(Nik-RE-dev): this code is walking only through inventory, but item was added to hand, so it will not bind new item if it was acquired
    //                   rather this code will bind jars that already present in inventory to liches that currently do not have binded jars
    if (item_id == ITEM_QUEST_LICH_JAR_FULL) {
        for (int i = 0; i < pParty->pCharacters.size(); i++) {
            if (pParty->pCharacters[i].classType == CLASS_LICH) {
                bool have_vessels_soul = false;
                InventoryEntry jar;
                for (Character &player : pParty->pCharacters) {
                    for (InventoryEntry entry : player.inventory.entries(ITEM_QUEST_LICH_JAR_FULL)) {
                        if (entry->lichJarCharacterIndex == -1)
                            jar = entry;
                        if (entry->lichJarCharacterIndex == i)
                            have_vessels_soul = true;
                    }
                }

                if (jar && !have_vessels_soul) {
                    jar->lichJarCharacterIndex = i;
                    break;
                }
            }
        }
    }
}

/**
 * @offset 0x4B29F2
 */
std::string joinGuildOptionString() {
    if (isMm6()) {
        bool approved;
        std::string result = mm6_topics::joinGuildOptionString(topicEventId, &gold_transaction_amount, &approved);
        membershipOrTrainingApproved = approved;
        return result;
    }

    GuildId guild_id = static_cast<GuildId>(topicEventId - 400);
    static const int dialogue_base = 110;
    AwardId guildMembershipAwardBit = membershipAwardForGuild(guild_id);

    membershipOrTrainingApproved = false;
    gold_transaction_amount = priceForMembership[guild_id];

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())
        pParty->setActiveToFirstCanAct();  // avoid nzi

    if (pParty->activeCharacter().CanAct()) {
        if (pParty->activeCharacter()._achievedAwardsBits[guildMembershipAwardBit]) {
            return pNPCTopics[dialogue_base + 13].pText;
        } else {
            if (gold_transaction_amount <= pParty->GetGold()) {
                membershipOrTrainingApproved = true;
                return pNPCTopics[dialogue_base + std::to_underlying(guild_id)].pText;
            } else {
                return pNPCTopics[dialogue_base + 14].pText;
            }
        }
    } else {
        return pNPCTopics[dialogue_base + 12].pText;
    }
}

namespace {

constexpr int MM8_FIRST_TEACHER_EVENT = 300; // Three topics per MM8 skill, expert, master and grandmaster.
constexpr int MM8_LAST_TEACHER_EVENT = 416;
constexpr int MM8_BOUNTY_HUNT_EVENT = 579;
constexpr int MM8_SEER_LOST_ITEMS_EVENT = 705;
constexpr int MM8_ARENA_EVENT = 704;

/**
 * @param eventId                       MM8 teacher topic.
 * @return                              MM8 skill id that the topic teaches, MM8.exe 0x4B301C. -1 for the ids that
 *                                      MM8 has no teachers for: blaster, dodging, unarmed and stealing.
 */
int mm8TeacherSkill(int eventId) {
    int mm8Skill = (eventId - MM8_FIRST_TEACHER_EVENT) / 3;
    if (mm8Skill == 7 || mm8Skill == 32 || mm8Skill == 33 || mm8Skill == 36)
        return -1;
    return mm8Skill;
}

/**
 * @param mm8Skill                      MM8 skill id.
 * @param mastery                       Mastery to learn.
 * @return                              Price in gold, MM8.exe 0x4B31FE.
 */
int mm8TeacherPrice(int mm8Skill, Mastery mastery) {
    auto in = [&](std::initializer_list<int> skills) { return std::ranges::find(skills, mm8Skill) != skills.end(); };
    if (mastery == MASTERY_EXPERT) {
        if (mm8Skill >= 8 && mm8Skill <= 23)
            return 1000;
        if (mm8Skill == 24 || (mm8Skill >= 26 && mm8Skill <= 31) || in({34, 37}))
            return 500;
        return 2000;
    }
    if (mastery == MASTERY_MASTER) {
        if (mm8Skill >= 8 && mm8Skill <= 11)
            return 3000;
        if (mm8Skill >= 12 && mm8Skill <= 23)
            return 4000;
        if (in({24, 26, 27, 28, 29, 31, 34, 37}))
            return 2500;
        return 5000;
    }
    if (mm8Skill >= 8 && mm8Skill <= 11)
        return 7000;
    if (in({24, 26, 27, 28, 29, 31, 34, 36, 37}))
        return 6000;
    return 8000;
}

/**
 * @param mm8Skill                      MM8 skill id.
 * @param character                     Character to teach.
 * @return                              Whether the character meets the stat needed for the master level, MM8.exe
 *                                      0x4B3456 asks 50 base personality for merchant, endurance for bodybuilding
 *                                      and intellect for learning.
 */
bool mm8MeetsMasterStat(int mm8Skill, const Character &character) {
    switch (mm8Skill) {
    case 25: return character.GetBasePersonality() >= 50;
    case 27: return character.GetBaseEndurance() >= 50;
    case 38: return character.GetBaseIntelligence() >= 50;
    default: return true;
    }
}

std::string mm8TeacherOptionString() {
    membershipOrTrainingApproved = false;
    int mm8Skill = mm8TeacherSkill(topicEventId);
    if (mm8Skill < 0)
        return {};
    Skill skill = skillFromMm8(mm8Skill);
    Mastery mastery = static_cast<Mastery>((topicEventId - MM8_FIRST_TEACHER_EVENT) % 3 + 2);
    Character &player = pParty->activeCharacter();

    if (skillMaxMasteryPerClass[player.classType][skill] < mastery) {
        Class promoted = classFromMm8(mm8IdFromClass(player.classType) | 1);
        if (skillMaxMasteryPerClass[promoted][skill] >= mastery)
            return localization->format(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->className(promoted));
        return localization->format(LSTR_THIS_SKILL_LEVEL_CAN_NOT_BE_LEARNED_BY, localization->className(player.classType));
    }
    if (!player.CanAct())
        return pNPCTopics[122].pText; // "You are in no condition to do anything right now!"
    CombinedSkillValue value = player.getSkillValue(skill);
    if (!value.level())
        return pNPCTopics[131].pText; // "You must know the skill before you can become an expert in it!"
    if (value.mastery() >= mastery)
        return pNPCTopics[std::to_underlying(mastery) + 126].pText; // "You are already an expert in this skill." and so on.

    int neededLevel = mastery == MASTERY_EXPERT ? 4 : mastery == MASTERY_MASTER ? 7 : 10;
    bool canLearn = value.level() >= neededLevel && std::to_underlying(value.mastery()) >= std::to_underlying(mastery) - 1;
    if (mastery == MASTERY_MASTER)
        canLearn = canLearn && mm8MeetsMasterStat(mm8Skill, player);
    if (!canLearn)
        return pNPCTopics[127].pText; // "You don't meet the requirements, and cannot be taught until you do."

    gold_transaction_amount = mm8TeacherPrice(mm8Skill, mastery);
    if (gold_transaction_amount > pParty->GetGold())
        return pNPCTopics[124].pText; // "You don't have enough gold!"

    membershipOrTrainingApproved = true;
    return localization->format(LSTR_BECOME_S_IN_S_FOR_LU_GOLD, localization->masteryNameLong(mastery), localization->skillName(skill),
                                gold_transaction_amount);
}

/**
 * @return                              Text of the seer's "I lost it!" topic, MM8.exe 0x4B2AC7. Gives back the first
 *                                      lost quest item whose quest bit is set.
 */
std::string mm8SeerLostItems() {
    // Quest bits and the items that go with them, MM8.exe 0x502C08.
    static constexpr std::array<std::pair<int, int>, 25> questItems = {{
        {199, 539}, {200, 540}, {201, 541}, {202, 603}, {203, 604}, {204, 605}, {205, 606}, {206, 607}, {207, 608},
        {208, 609}, {209, 610}, {210, 611}, {211, 612}, {212, 617}, {213, 618}, {214, 619}, {215, 620}, {216, 621},
        {217, 623}, {218, 626}, {219, 627}, {220, 629}, {221, 741}, {222, 742}, {224, 662}
    }};
    for (auto [bit, mm8Item] : questItems) {
        ItemId itemId = itemIdFromMm8(mm8Item);
        if (!pParty->_questBits[static_cast<QuestBit>(bit)] || pParty->hasItem(itemId) || pParty->pPickedItem.itemId == itemId)
            continue;
        Item item;
        item.itemId = itemId;
        item.flags = ITEM_IDENTIFIED;
        pParty->addItemToParty(&item, true);
        return pNPCTopics[851].pText; // "Here is your lost item."
    }
    return pNPCTopics[850].pText; // "You never had it!"
}

} // namespace

/**
 * @offset 0x4B254D
 */
std::string masteryTeacherOptionString() {
    if (isMm6()) {
        bool approved;
        std::string result = mm6_topics::teacherOptionString(topicEventId, &gold_transaction_amount, &approved);
        membershipOrTrainingApproved = approved;
        return result;
    }
    if (isMm8())
        return mm8TeacherOptionString();

    int teacherLevel = (topicEventId - 200) % 3;
    Skill skillBeingTaught = static_cast<Skill>((topicEventId - 200) / 3);
    Character *activePlayer = &pParty->activeCharacter();
    Class pClassType = activePlayer->classType;
    Mastery currClassMaxMastery = skillMaxMasteryPerClass[pClassType][skillBeingTaught];
    Mastery masteryLevelBeingTaught = static_cast<Mastery>(teacherLevel + 2);

    membershipOrTrainingApproved = false;

    if (currClassMaxMastery < masteryLevelBeingTaught) {
        if (skillMaxMasteryPerClass[getTier2Class(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->format(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->className(getTier2Class(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught &&
                skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->format(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_OR_S_TO,
                    localization->className(getTier3LightClass(pClassType)),
                    localization->className(getTier3DarkClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->format(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->className(getTier3LightClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->format(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->className(getTier3DarkClass(pClassType)));
        } else {
            return localization->format(LSTR_THIS_SKILL_LEVEL_CAN_NOT_BE_LEARNED_BY, localization->className(pClassType));
        }
    }

    // Not in your condition!
    if (!activePlayer->CanAct()) {
        return std::string(pNPCTopics[122].pText);
    }

    // You must know the skill before you can become an expert in it!
    int skillLevel = activePlayer->getSkillValue(skillBeingTaught).level();
    if (!skillLevel) {
        return std::string(pNPCTopics[131].pText);
    }

    // You are already have this mastery in this skill.
    Mastery skillMastery = activePlayer->getSkillValue(skillBeingTaught).mastery();
    if (std::to_underlying(skillMastery) > teacherLevel + 1) {
        return std::string(pNPCTopics[teacherLevel + 128].pText);
    }

    bool canLearn = true;

    if (masteryLevelBeingTaught == MASTERY_EXPERT) {
        canLearn = skillLevel >= 4;
        gold_transaction_amount = expertSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == MASTERY_MASTER) {
        switch (skillBeingTaught) {
          case SKILL_LIGHT:
            canLearn = pParty->_questBits[QBIT_114];
            break;
          case SKILL_DARK:
            canLearn = pParty->_questBits[QBIT_110];
            break;
          case SKILL_MERCHANT:
            canLearn = activePlayer->GetBasePersonality() >= 50;
            break;
          case SKILL_BODYBUILDING:
            canLearn = activePlayer->GetBaseEndurance() >= 50;
            break;
          case SKILL_LEARNING:
            canLearn = activePlayer->GetBaseIntelligence() >= 50;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 7) && (skillMastery == MASTERY_EXPERT);
        gold_transaction_amount = masterSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == MASTERY_GRANDMASTER) {
        switch (skillBeingTaught) {
          case SKILL_LIGHT:
            canLearn = activePlayer->isClass(CLASS_ARCHAMGE) || activePlayer->isClass(CLASS_PRIEST_OF_SUN);
            break;
          case SKILL_DARK:
            canLearn = activePlayer->isClass(CLASS_LICH) || activePlayer->isClass(CLASS_PRIEST_OF_MOON);
            break;
          case SKILL_DODGE:
            canLearn = activePlayer->pActiveSkills[SKILL_UNARMED].level() >= 10;
            break;
          case SKILL_UNARMED:
            canLearn = activePlayer->pActiveSkills[SKILL_DODGE].level() >= 10;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 10) && (skillMastery == MASTERY_MASTER);
        gold_transaction_amount = grandmasterSkillMasteryCost[skillBeingTaught];
    }

    // You don't meet the requirements, and cannot be taught until you do.
    if (!canLearn) {
        return std::string(pNPCTopics[127].pText);
    }

    // You don't have enough gold!
    if (gold_transaction_amount > pParty->GetGold()) {
        return std::string(pNPCTopics[124].pText);
    }

    membershipOrTrainingApproved = true;

    return localization->format(LSTR_BECOME_S_IN_S_FOR_LU_GOLD, localization->masteryNameLong(masteryLevelBeingTaught),
                                      localization->skillName(skillBeingTaught), gold_transaction_amount);
}

static std::string scriptedTopicString(int eventId) {
    if (isMm8() && eventId >= MM8_FIRST_ROSTER_EVENT && eventId < MM8_FIRST_ROSTER_EVENT + MM8_ROSTER_SIZE)
        return localization->str(LSTR_JOIN); // npctopic.txt only has "Roster Join Event" for these.
    return pNPCTopics[eventId].pTopic;
}

std::string npcDialogueOptionString(DialogueId topic, NPCData *npcData) {
    switch (topic) {
      case DIALOGUE_SCRIPTED_LINE_1:
        return scriptedTopicString(npcData->dialogue_1_evt_id);
      case DIALOGUE_SCRIPTED_LINE_2:
        return scriptedTopicString(npcData->dialogue_2_evt_id);
      case DIALOGUE_SCRIPTED_LINE_3:
        return scriptedTopicString(npcData->dialogue_3_evt_id);
      case DIALOGUE_SCRIPTED_LINE_4:
        return scriptedTopicString(npcData->dialogue_4_evt_id);
      case DIALOGUE_SCRIPTED_LINE_5:
        return scriptedTopicString(npcData->dialogue_5_evt_id);
      case DIALOGUE_SCRIPTED_LINE_6:
        return scriptedTopicString(npcData->dialogue_6_evt_id);
      case DIALOGUE_HIRE_FIRE:
        if (npcData->Hired()) {
            return localization->format(LSTR_DISMISS_S, npcData->name);
        } else {
            return localization->str(LSTR_HIRE);
        }
      case DIALOGUE_13_hiring_related:
        if (isMm6()) // "Dismiss %s" and "Join us".
            return npcData->Hired() ? fmt::sprintf(localization->mm6Str(408), npcData->name) : localization->mm6Str(122); // NOLINT: this is not ::sprintf.
        if (npcData->Hired()) {
            return localization->format(LSTR_DISMISS_S, npcData->name);
        } else {
            return localization->str(LSTR_JOIN);
        }
      case DIALOGUE_MM6_DAY_TOPIC:
        return mm6_talk::dayTopic(npcData);
      case DIALOGUE_MM6_NEWS:
        return mm6_talk::newsTopic(npcData);
      case DIALOGUE_MM6_BEG:
        return localization->mm6Str(27);
      case DIALOGUE_MM6_THREATEN:
        return localization->mm6Str(226);
      case DIALOGUE_MM6_BRIBE:
        return mm6_talk::bribeOptionString();
      case DIALOGUE_PROFESSION_DETAILS:
        return localization->str(LSTR_MORE_INFORMATION);
      case DIALOGUE_MASTERY_TEACHER_LEARN:
        return masteryTeacherOptionString();
      case DIALOGUE_MAGIC_GUILD_JOIN:
        return joinGuildOptionString();
      case DIALOGUE_ARENA_SELECT_LORD:
        return localization->str(LSTR_ARENA_DIFFICULTY_LORD);
      case DIALOGUE_ARENA_SELECT_KNIGHT:
        return localization->str(LSTR_ARENA_DIFFICULTY_KNIGHT);
      case DIALOGUE_ARENA_SELECT_SQUIRE:
        return localization->str(LSTR_ARENA_DIFFICULTY_SQUIRE);
      case DIALOGUE_ARENA_SELECT_PAGE:
        return localization->str(LSTR_ARENA_DIFFICULTY_PAGE);
      case DIALOGUE_USE_HIRED_NPC_ABILITY:
        return GetProfessionActionText(npcData->profession);
      case DIALOGUE_MM8_ROSTER_JOIN:
        return localization->mm8Str(704); // "Yes".
      case DIALOGUE_MM8_ROSTER_DECLINE:
        return localization->mm8Str(705); // "No".
      default:
        return "";
    }
}

std::vector<DialogueId> prepareScriptedNPCDialogueTopics(NPCData *npcData) {
    std::vector<DialogueId> optionList;

    if (isMm6()) { // MM6.exe 0x41975C.
        if (npcData->profession != NoProfession)
            optionList.push_back(DIALOGUE_MM6_DAY_TOPIC);
        if (npcData->canJoin || npcData->Hired())
            optionList.push_back(DIALOGUE_13_hiring_related);
        if (npcData->field_24)
            optionList.push_back(DIALOGUE_MM6_NEWS);
        for (auto [eventId, topic] : {std::pair(npcData->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1),
                                      std::pair(npcData->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2),
                                      std::pair(npcData->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3)}) {
            if (eventId && optionList.size() < 6) {
                int res = npcDialogueEventProcessor(eventId);
                if (res == 1 || res == 2)
                    optionList.push_back(topic);
            }
        }
        return optionList;
    }

    if (npcData->canJoin) {
        optionList.push_back(DIALOGUE_13_hiring_related);
    }

    // TODO(Nik-RE-dev): place NPC events in array
#define ADD_NPC_SCRIPTED_DIALOGUE(EVENT_ID, MSG_PARAM) \
    if (EVENT_ID) { \
        if (optionList.size() < 4) { \
            int res = npcDialogueEventProcessor(EVENT_ID); \
            if (res == 1 || res == 2) { \
                optionList.push_back(MSG_PARAM); \
            } \
        } \
    }

    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);

#undef ADD_NPC_SCRIPTED_DIALOGUE

    return optionList;
}

/**
 * @param topic                         Selected topic.
 * @param eventId                       Event of the topic.
 * @return                              Dialogue to switch to, MM6.exe 0x4A4205.
 */
static DialogueId handleMm6NPCTopicSelection(DialogueId topic, int eventId) {
    if (eventId >= mm6_topics::FIRST_TEACHER_EVENT && eventId <= mm6_topics::LAST_TEACHER_EVENT) {
        current_npc_text = pNPCTopics[eventId - 1].pText;
        topicEventId = eventId;
        return DIALOGUE_MASTERY_TEACHER_OFFER;
    }
    if (eventId >= mm6_topics::FIRST_GUILD_EVENT && eventId <= mm6_topics::LAST_GUILD_EVENT) {
        guildMembershipNPCTopicId = topic;
        current_npc_text = pNPCTopics[eventId - mm6_topics::FIRST_GUILD_EVENT + 137].pText; // npctext.txt 138-154.
        topicEventId = eventId;
        return DIALOGUE_MAGIC_GUILD_OFFER;
    }
    if (eventId == mm6_topics::SEER_PILGRIMAGE_EVENT || eventId == mm6_topics::SEER_LOST_ITEMS_EVENT) {
        current_npc_text = eventId == mm6_topics::SEER_PILGRIMAGE_EVENT ? mm6_topics::seerPilgrimage() : mm6_topics::seerLostItems();
        return DIALOGUE_MAIN;
    }
    if (eventId == mm6_topics::BOUNTY_HUNT_EVENT) {
        if (window_SpeakInHouse)
            current_npc_text = mm6_topics::bountyHunt(window_SpeakInHouse->houseId());
        return DIALOGUE_MAIN;
    }
    if (eventId == mm6_topics::ARENA_EVENT)
        return arenaMainDialogue();

    activeLevelDecoration = (LevelDecoration *)1;
    current_npc_text.clear();
    eventProcessor(eventId, Pid(), 1);
    activeLevelDecoration = nullptr;
    return DIALOGUE_MAIN;
}

/**
 * MM8.exe 0x4BC140, NPC topic 579. MM8 has one bounty for the whole game, it's kept in the slot of the first MM7 town
 * hall.
 *
 * @return                              Text about this month's bounty, pays out the reward if the monster is dead.
 */
static std::string mm8BountyHunt() {
    HouseId slot = HOUSE_FIRST_TOWN_HALL;
    if (pParty->PartyTimes.bountyHuntNextGenTime[slot] <= pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[slot] = false;
        pParty->PartyTimes.bountyHuntNextGenTime[slot] = Time::fromMonths(pParty->GetPlayingTime().toMonths() + 1);
        pParty->monster_id_for_hunting[slot] = static_cast<MonsterId>(grng->random(192) + 4); // Any monster from the fourth on.
    }

    MonsterId monster = pParty->monster_id_for_hunting[slot];
    if (monster == MONSTER_INVALID)
        return pNPCTopics[134].pText; // "Someone has already claimed the bounty this month..."

    int bounty = 100 * pMonsterStats->infos[monster].level;
    int text = 132; // "This month's bounty is on a %s..."
    if (pParty->monster_for_hunting_killed[slot]) {
        pParty->partyFindsGold(bounty, GOLD_RECEIVE_SHARE);
        for (Character &character : pParty->pCharacters)
            character.giveAward(counterAward(AWARD_BOUNTIES_COLLECTED));
        pParty->uNumBountiesCollected += bounty;
        pParty->monster_id_for_hunting[slot] = MONSTER_INVALID;
        pParty->monster_for_hunting_killed[slot] = false;
        text = 133; // "Congratulations on defeating the %s! Here is the %lu gold reward..."
    }
    std::string name = fmt::format("{::}{}{::}", colorTable.PaleCanary.tag(), pMonsterStats->infos[monster].name, colorTable.White.tag());
    return fmt::sprintf(pNPCTopics[text].pText, name, bounty); // NOLINT: this is not ::sprintf.
}

/**
 * @param eventId                       Event of the selected topic.
 * @return                              Dialogue to switch to, MM8.exe 0x4BCC7C.
 */
static DialogueId handleMm8NPCTopicSelection(int eventId) {
    if (eventId >= MM8_FIRST_TEACHER_EVENT && eventId <= MM8_LAST_TEACHER_EVENT && mm8TeacherSkill(eventId) >= 0) {
        current_npc_text = pNPCTopics[eventId - 1].pText; // The npctext.txt line has the number of the topic.
        topicEventId = eventId;
        return DIALOGUE_MASTERY_TEACHER_OFFER;
    }
    if (eventId == MM8_BOUNTY_HUNT_EVENT) {
        current_npc_text = mm8BountyHunt();
        return DIALOGUE_MAIN;
    }
    if (eventId == MM8_SEER_LOST_ITEMS_EVENT) {
        current_npc_text = mm8SeerLostItems();
        return DIALOGUE_MAIN;
    }
    if (eventId >= MM8_FIRST_ROSTER_EVENT && eventId < MM8_FIRST_ROSTER_EVENT + MM8_ROSTER_SIZE) { // MM8.exe 0x4B4D70.
        topicEventId = eventId;
        current_npc_text = pNPCTopics[197 + 2 * (eventId - MM8_FIRST_ROSTER_EVENT)].pText; // npctext.txt "roster N join".
        return DIALOGUE_MM8_ROSTER_OFFER;
    }

    if (eventId == MM8_ARENA_EVENT)
        return arenaMainDialogue();

    activeLevelDecoration = (LevelDecoration *)1;
    current_npc_text.clear();
    eventProcessor(eventId, Pid(), 1);
    activeLevelDecoration = nullptr;
    return DIALOGUE_MAIN;
}

DialogueId handleScriptedNPCTopicSelection(DialogueId topic, NPCData *npcData) {
    int eventId;

    if (topic == DIALOGUE_SCRIPTED_LINE_1) {
        eventId = npcData->dialogue_1_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_2) {
        eventId = npcData->dialogue_2_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_3) {
        eventId = npcData->dialogue_3_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_4) {
        eventId = npcData->dialogue_4_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_5) {
        eventId = npcData->dialogue_5_evt_id;
    } else {
        assert(topic == DIALOGUE_SCRIPTED_LINE_6);
        eventId = npcData->dialogue_6_evt_id;
    }

    if (isMm6())
        return handleMm6NPCTopicSelection(topic, eventId);

    if (isMm8())
        return handleMm8NPCTopicSelection(eventId);


    if (eventId == 311) {
        // Original code also listed this event which presumably opened bounty dialogue but MM7
        // use event 311 for some teleport in Bracada
        assert(false);
        return DIALOGUE_MAIN;
    }

    if (eventId == 139) {
        oracleDialogue();
    } else if (eventId == 399) {
        return arenaMainDialogue();
    } else if (eventId >= 400 && eventId <= 410) {
        guildMembershipNPCTopicId = topic;
        current_npc_text = pNPCTopics[eventId - 301].pText;
        topicEventId = eventId;
        return DIALOGUE_MAGIC_GUILD_OFFER;
    } else if (eventId >= 200 && eventId <= 310) {
        current_npc_text = pNPCTopics[eventId + 168].pText;
        topicEventId = eventId;
        return DIALOGUE_MASTERY_TEACHER_OFFER;
    } else {
        activeLevelDecoration = (LevelDecoration *)1;
        current_npc_text.clear();
        eventProcessor(eventId, Pid(), 1);
        activeLevelDecoration = nullptr;
    }

    return DIALOGUE_MAIN;
}

std::vector<DialogueId> listNPCDialogueOptions(DialogueId topic) {
    switch (topic) {
      case DIALOGUE_MAGIC_GUILD_OFFER:
        return {DIALOGUE_MAGIC_GUILD_JOIN};
      case DIALOGUE_MASTERY_TEACHER_OFFER:
        return {DIALOGUE_MASTERY_TEACHER_LEARN};
      case DIALOGUE_ARENA_WELCOME:
        return {DIALOGUE_ARENA_SELECT_PAGE, DIALOGUE_ARENA_SELECT_SQUIRE, DIALOGUE_ARENA_SELECT_KNIGHT, DIALOGUE_ARENA_SELECT_LORD};
      case DIALOGUE_MM8_ROSTER_OFFER:
        return {DIALOGUE_MM8_ROSTER_JOIN, DIALOGUE_MM8_ROSTER_DECLINE};
      default:
        return {};
    }
}


void selectSpecialNPCTopicSelection(DialogueId topic, NPCData* npcData) {
    if (topic == DIALOGUE_MASTERY_TEACHER_LEARN) {
        if (membershipOrTrainingApproved) {
            if (pParty->hasActiveCharacter()) {
                uint8_t teacherLevel = (topicEventId - 200) % 3;
                Skill skillBeingTaught = static_cast<Skill>((topicEventId - 200) / 3);
                Mastery newMastery = static_cast<Mastery>(teacherLevel + 2);
                if (isMm6())
                    std::tie(skillBeingTaught, newMastery) = mm6_topics::teacherSkill(topicEventId);
                if (isMm8()) {
                    skillBeingTaught = skillFromMm8(mm8TeacherSkill(topicEventId));
                    newMastery = static_cast<Mastery>((topicEventId - MM8_FIRST_TEACHER_EVENT) % 3 + 2);
                }
                CombinedSkillValue skillValue = CombinedSkillValue::increaseMastery(pParty->activeCharacter().getSkillValue(skillBeingTaught), newMastery);
                pParty->activeCharacter().setSkillValue(skillBeingTaught, skillValue);
                pParty->activeCharacter().playReaction(SPEECH_SKILL_MASTERY_INC);
                pParty->TakeGold(gold_transaction_amount);
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            }
        }
    } else if (topic == DIALOGUE_MAGIC_GUILD_JOIN) {
        if (membershipOrTrainingApproved) {
            AwardId guildMembershipAwardBit = isMm6() ? mm6_topics::guildMembershipAward(topicEventId)
                                                      : membershipAwardForGuild(static_cast<GuildId>(topicEventId - 400));
            int firstGuildEvent = isMm6() ? mm6_topics::FIRST_GUILD_EVENT : 400;
            int lastGuildEvent = isMm6() ? mm6_topics::LAST_GUILD_EVENT : 416;
            pParty->TakeGold(gold_transaction_amount, true);
            for (Character &player : pParty->pCharacters) {
                player.giveAward(guildMembershipAwardBit);
            }

            switch (guildMembershipNPCTopicId) {
              case DIALOGUE_SCRIPTED_LINE_1:
                if (npcData->dialogue_1_evt_id >= firstGuildEvent && npcData->dialogue_1_evt_id <= lastGuildEvent)
                    npcData->dialogue_1_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_2:
                if (npcData->dialogue_2_evt_id >= firstGuildEvent && npcData->dialogue_2_evt_id <= lastGuildEvent)
                    npcData->dialogue_2_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_3:
                if (npcData->dialogue_3_evt_id >= firstGuildEvent && npcData->dialogue_3_evt_id <= lastGuildEvent)
                    npcData->dialogue_3_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_4:
                if (npcData->dialogue_4_evt_id >= firstGuildEvent && npcData->dialogue_4_evt_id <= lastGuildEvent)
                    npcData->dialogue_4_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_5:
                if (npcData->dialogue_5_evt_id >= firstGuildEvent && npcData->dialogue_5_evt_id <= lastGuildEvent)
                    npcData->dialogue_5_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_6:
                if (npcData->dialogue_6_evt_id >= firstGuildEvent && npcData->dialogue_6_evt_id <= lastGuildEvent)
                    npcData->dialogue_6_evt_id = 0;
                break;
              default:
                break;
            }

            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_JOINED_GUILD);
            }
        }
    } else if (topic == DIALOGUE_MM8_ROSTER_JOIN || topic == DIALOGUE_MM8_ROSTER_DECLINE) { // MM8.exe 0x4BCC7C.
        int rosterId = topicEventId - MM8_FIRST_ROSTER_EVENT;
        if (topic == DIALOGUE_MM8_ROSTER_JOIN) {
            Mm8JoinResult result = joinMm8RosterCharacter(rosterId);
            if (result == MM8_JOINED) {
                int index = pParty->pCharacters.size() - 1;
                GameUI_ReloadPlayerPortraits(index, pParty->pCharacters[index].uCurrentFace);
                pAudioPlayer->playUISound(SOUND_51heroism03);
            } else if (result == MM8_JOIN_PARTY_FULL) {
                current_npc_text = pNPCTopics[198 + 2 * rosterId].pText; // npctext.txt "roster N full".
                keepNpcTextOnEscape = true;
            }
        }
        npcData->flags |= NPC_HIRED;
        sendMm8RosterCharacterToInn(rosterId);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    } else if (topic == DIALOGUE_PROFESSION_DETAILS) {
        dialogue_show_profession_details = ~dialogue_show_profession_details;
    } else if (topic >= DIALOGUE_ARENA_SELECT_PAGE && topic <= DIALOGUE_ARENA_SELECT_LORD) {
        prepareArenaFight(arenaLevelForDialogue(topic));
    } else if (topic == DIALOGUE_USE_HIRED_NPC_ABILITY) {
        int hirelingId;
        for (hirelingId = 0; hirelingId < pParty->pHirelings.size(); hirelingId++) {
            if (ascii::noCaseEquals(pParty->pHirelings[hirelingId].name, npcData->name)) { // TODO(captainurist): #unicode
                break;
            }
        }
        assert(hirelingId < pParty->pHirelings.size());
        if (UseNPCSkill(npcData->profession, hirelingId) == 0) {
            if (npcData->profession != GateMaster) {
                npcData->hasUsedAbility = 1;
            }
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        } else {
            engine->_statusBar->setEvent(LSTR_YOUR_PACKS_ARE_ALREADY_FULL);
        }
    } else if (topic == DIALOGUE_HIRE_FIRE) {
        if (npcData->Hired()) {
            if (pNPCStats->uNumNewNPCs > 0) {
                for (int i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
                    if (pNPCStats->pNPCData[i].Hired() && npcData->name == pNPCStats->pNPCData[i].name) {
                        pNPCStats->pNPCData[i].flags &= ~NPC_HIRED;
                    }
                }
            }
            if (ascii::noCaseEquals(pParty->pHirelings[0].name, npcData->name)) { // TODO(captainurist): #unicode
                pParty->pHirelings[0] = NPCData();
            } else if (ascii::noCaseEquals(pParty->pHirelings[1].name, npcData->name)) { // TODO(captainurist): #unicode
                pParty->pHirelings[1] = NPCData();
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }
        if (!pParty->pHirelings[0].name.empty() && !pParty->pHirelings[1].name.empty()) {
            engine->_statusBar->setEvent(LSTR_I_CANNOT_JOIN_YOU_YOURE_PARTY_IS_FULL);
        } else {
            if (npcData->profession != Burglar) {
                // burglars have no hiring price
                if (pParty->GetGold() < pNPCStats->pProfessions[npcData->profession].uHirePrice) {
                    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
                    dialogue_show_profession_details = false;
                    //uDialogueType = DIALOGUE_13_hiring_related;
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
                    }
                    return;
                }
                pParty->TakeGold(pNPCStats->pProfessions[npcData->profession].uHirePrice);
            }
            npcData->flags |= NPC_HIRED;
            if (!pParty->pHirelings[0].name.empty()) {
                pParty->pHirelings[1] = *npcData;
                pParty->pHireling2Name = npcData->name;
            } else {
                pParty->pHirelings[0] = *npcData;
                pParty->pHireling1Name = npcData->name;
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_HIRE_NPC);
            }
        }
    }
}
