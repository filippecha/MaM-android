#include "Mm6NpcTopics.h"

#include <array>
#include <cstdint>
#include <string>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/Mm6CharacterRules.h"
#include "Engine/Objects/Mm6Ids.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/NPCTable.h"

#include "Library/Color/ColorTable.h"

namespace mm6_topics {

namespace {

constexpr int FIRST_TOWN_HALL = 89;
constexpr int LAST_TOWN_HALL = 91;
constexpr AwardId FIRST_GUILD_AWARD = static_cast<AwardId>(64);
constexpr AwardId BOUNTY_HUNT_AWARD = static_cast<AwardId>(81);

/**
 * @param id                            Line in MM6 npctext.txt.
 * @return                              The text.
 */
std::string npcText(int id) {
    return pNPCTopics[id - 1].pText;
}

bool isMm6Class(const Character &character, int mm6Class) {
    return character.classType == classFromMm6(mm6Class);
}

bool hasAward(const Character &character, int award) {
    return character._achievedAwardsBits[static_cast<AwardId>(award)];
}

bool hasBlaster(const Character &character) {
    for (InventoryConstEntry entry : character.inventory.entries())
        if (entry->skill() == SKILL_BLASTER)
            return true;
    return false;
}

/**
 * Master requirements of MM6.exe 0x496D67. Price 0 means that the lesson is free.
 *
 * @param character                     Character that wants to learn.
 * @param mm6Skill                      MM6 skill id.
 * @param level                         Level of the skill.
 * @param[out] price                    Price of the lesson.
 * @return                              Whether the character meets the requirements.
 */
bool meetsMasterRequirements(const Character &character, int mm6Skill, int level, int *price) {
    // MM6 class ids: 1 Cavalier, 2 Champion, 5 High Priest, 8 Arch Mage, 10 Crusader, 11 Hero, 13 Battle Mage,
    // 14 Warrior Mage. The awards are the promotions to these classes.
    *price = 5000;
    switch (mm6Skill) {
    case 0: // Staff.
        return level >= 8;
    case 1: // Sword.
        *price = 0;
        return level >= 8 && (isMm6Class(character, 1) || isMm6Class(character, 2) || hasAward(character, 17) || hasAward(character, 19));
    case 2: // Dagger.
        return level >= 8 && character.GetActualSpeed() >= 40;
    case 3: // Axe, MM6 has no axe master.
        *price = 0;
        return true;
    case 4: // Spear.
        return level >= 8 && (isMm6Class(character, 1) || isMm6Class(character, 2) || hasAward(character, 17) || hasAward(character, 19));
    case 5: // Bow.
        *price = 0;
        return level >= 8 && (isMm6Class(character, 13) || isMm6Class(character, 14) || hasAward(character, 29) || hasAward(character, 31));
    case 6: // Mace.
        return level >= 8 && character.GetActualMight() >= 40;
    case 7: // Blaster.
        *price = 0;
        return hasBlaster(character);
    case 8: // Shield.
        return level >= 10;
    case 9: // Leather.
        *price = 3000;
        return level >= 10;
    case 10: // Chain.
        *price = 0;
        return level >= 10 && (isMm6Class(character, 10) || isMm6Class(character, 11) || hasAward(character, 9) || hasAward(character, 11));
    case 11: // Plate.
        *price = 0;
        return isMm6Class(character, 11) || hasAward(character, 11);
    case 13: // Air.
        *price = 4000;
        return isMm6Class(character, 8) || hasAward(character, 15);
    case 12: // Fire.
    case 14: // Water.
    case 15: // Earth.
    case 17: // Mind.
    case 18: // Body.
        *price = 4000;
        return level >= 12;
    case 16: // Spirit.
        *price = 0;
        return isMm6Class(character, 5) || hasAward(character, 23);
    case 19: // Light.
        *price = 0;
        return partyReputation() >= 1000;
    case 20: // Dark.
        *price = 0;
        return partyReputation() <= -1000;
    case 21: // Identify item.
        *price = 2500;
        return level >= 7 && character.GetActualIntelligence() >= 30;
    case 22: // Merchant.
        *price = 4000;
        return level >= 7 && character.GetActualPersonality() >= 30;
    case 23: // Repair.
    case 29: // Disarm traps.
        *price = 2500;
        return level >= 7 && character.GetActualAccuracy() >= 30;
    case 24: // Bodybuilding.
        *price = 2500;
        return level >= 7 && character.GetActualEndurance() >= 30;
    case 25: // Meditation.
        *price = 2500;
        return level >= 7 && character.GetActualPersonality() >= 30;
    case 26: // Perception.
        *price = 2500;
        return level >= 7 && character.GetActualLuck() >= 30;
    case 27: // Diplomacy.
        *price = 2500;
        return partyFame() >= 200;
    case 30: // Learning.
        return level >= 7 && character.GetActualIntelligence() >= 30;
    default: // Thievery.
        return false;
    }
}

/**
 * @param mm6Skill                      MM6 skill id.
 * @return                              Price of becoming an expert, MM6.exe 0x497102.
 */
int expertPrice(int mm6Skill) {
    if (mm6Skill >= 9 && mm6Skill <= 18) // Armor and elemental & self magic.
        return 1000;
    switch (mm6Skill) {
    case 21: // Identify item.
    case 23: // Repair.
    case 24: // Bodybuilding.
    case 25: // Meditation.
    case 26: // Perception.
    case 27: // Diplomacy.
    case 29: // Disarm traps.
        return 500;
    default:
        return 2000;
    }
}

/**
 * @param eventId                       Teacher event, 200-259.
 * @return                              MM6 skill id. Thievery has no teachers, the teachers after it are shifted.
 */
int teacherMm6Skill(int eventId) {
    int index = (eventId - FIRST_TEACHER_EVENT) / 2;
    return index > 27 ? index + 1 : index;
}

} // namespace

int partyFame() {
    uint64_t experience = 0;
    for (const Character &character : pParty->pCharacters)
        experience += character.experience;
    return static_cast<int>(experience / 1000);
}

std::pair<Skill, Mastery> teacherSkill(int eventId) {
    bool master = (eventId - FIRST_TEACHER_EVENT) % 2;
    return {skillFromMm6(teacherMm6Skill(eventId)), master ? MASTERY_MASTER : MASTERY_EXPERT};
}

std::string teacherOptionString(int eventId, int *price, bool *approved) {
    Character &character = pParty->activeCharacter();
    int mm6Skill = teacherMm6Skill(eventId);
    auto [skill, mastery] = teacherSkill(eventId);
    *approved = false;
    *price = mastery == MASTERY_MASTER ? 5000 : 2000;

    if (!character.CanAct())
        return npcText(264); // "Not in your condition!"

    int level = character.getSkillValue(skill).level();
    if (!level)
        return npcText(262); // "You must know the skill before you can become an expert in it!"

    Mastery currentMastery = character.getSkillValue(skill).mastery();
    if (currentMastery >= mastery)
        return npcText(mastery == MASTERY_MASTER ? 266 : 265); // "You are already an expert / a master in this skill."

    bool canLearn;
    if (mastery == MASTERY_MASTER) {
        if (currentMastery < MASTERY_EXPERT)
            return npcText(263); // "You must become an expert before you can become a master."
        canLearn = meetsMasterRequirements(character, mm6Skill, level, price);
    } else {
        canLearn = level >= 4;
        *price = expertPrice(mm6Skill);
    }

    if (!canLearn)
        return npcText(261); // "You don't meet the requirements, and cannot be taught until you do."
    if (*price > pParty->GetGold())
        return npcText(260); // "You don't have enough gold"

    *approved = true;
    return fmt::sprintf(localization->mm6Str(534), localization->mm6Str(mastery == MASTERY_MASTER ? 432 : 433), // NOLINT: this is not ::sprintf.
                        localization->skillName(skill), *price);
}

AwardId guildMembershipAward(int eventId) {
    return static_cast<AwardId>(std::to_underlying(FIRST_GUILD_AWARD) + eventId - FIRST_GUILD_EVENT);
}

std::string joinGuildOptionString(int eventId, int *price, bool *approved) {
    static constexpr std::array<int, 17> prices = {100, 100, 25, 50, 50, 25, 50, 50, 50, 50, 50, 50, 50, 50, 50, 1000, 1000}; // MM6.exe 0x4C3E10.
    int guild = eventId - FIRST_GUILD_EVENT;
    Character &character = pParty->activeCharacter();
    *approved = false;
    *price = prices[guild];

    if (!character.CanAct())
        return npcText(264); // "Not in your condition!"
    if (character._achievedAwardsBits[guildMembershipAward(eventId)])
        return npcText(137); // "You are already a member."
    if (*price > pParty->GetGold())
        return npcText(260); // "You don't have enough gold"

    *approved = true;
    return npcText(155 + guild); // "Join the ... guild for ... gold".
}

std::string bountyHunt(HouseId house) {
    int townHall = std::to_underlying(house);
    if (townHall < FIRST_TOWN_HALL || townHall > LAST_TOWN_HALL)
        return {};
    HouseId slot = static_cast<HouseId>(std::to_underlying(HOUSE_FIRST_TOWN_HALL) + townHall - FIRST_TOWN_HALL);

    if (pParty->PartyTimes.bountyHuntNextGenTime[slot] <= pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[slot] = false;
        pParty->PartyTimes.bountyHuntNextGenTime[slot] = Time::fromMonths(pParty->GetPlayingTime().toMonths() + 1);
        while (true) {
            // Peasants, merchants, VARN guards and robots are never hunted.
            int id = grng->random(171) + 1;
            if ((id >= 88 && id <= 90) || (id >= 103 && id <= 105) || (id >= 121 && id <= 126) || (id >= 133 && id <= 135) ||
                (id >= 148 && id <= 150))
                continue;
            pParty->monster_id_for_hunting[slot] = static_cast<MonsterId>(id);
            break;
        }
    }

    MonsterId monster = pParty->monster_id_for_hunting[slot];
    if (monster == MONSTER_INVALID)
        return npcText(370); // "Someone has already claimed the bounty this month."

    const MonsterInfo &info = pMonsterStats->infos[monster];
    int reward = 100 * info.level;
    std::string name = fmt::format("{::}{}{::}", colorTable.PaleCanary.tag(), info.name, colorTable.White.tag());
    if (!pParty->monster_for_hunting_killed[slot])
        return fmt::sprintf(npcText(368), name, reward); // NOLINT: this is not ::sprintf.

    pParty->partyFindsGold(reward, GOLD_RECEIVE_SHARE);
    for (Character &character : pParty->pCharacters)
        character.giveAward(BOUNTY_HUNT_AWARD);
    pParty->mm6Reputation += info.level;
    pParty->uNumBountiesCollected++;
    pParty->monster_id_for_hunting[slot] = MONSTER_INVALID;
    pParty->monster_for_hunting_killed[slot] = false;
    return fmt::sprintf(npcText(369), name, reward); // NOLINT: this is not ::sprintf.
}

std::string seerPilgrimage() {
    constexpr QuestBit pilgrimageStartedBit = static_cast<QuestBit>(205);
    constexpr QuestBit pilgrimageDoneBit = static_cast<QuestBit>(206);
    // Shrine of each month: might, intellect, personality, endurance, accuracy, speed, luck, fire, electricity, cold,
    // poison and magic, as global.txt lines.
    static constexpr std::array<int, 12> shrines = {144, 116, 163, 75, 1, 211, 136, 87, 71, 43, 166, 138};

    if (pParty->mm6NextPilgrimageTime <= pParty->GetPlayingTime()) {
        pParty->mm6NextPilgrimageTime = Time::fromMonths(pParty->GetPlayingTime().toMonths() + 1);
        pParty->_questBits.reset(pilgrimageStartedBit);
        pParty->_questBits.reset(pilgrimageDoneBit);
    }

    if (pParty->_questBits[pilgrimageDoneBit])
        return npcText(55); // "You have to wait for a new month to make a pilgrimage."
    int month = pParty->uCurrentMonth;
    const std::string &shrine = localization->mm6Str(shrines[month]);
    return fmt::sprintf(npcText(54), localization->monthName(month), shrine, shrine); // NOLINT: this is not ::sprintf.
}

std::string seerLostItems() {
    // Quest bits and the items that go with them, MM6.exe 0x4C3DAC.
    static constexpr std::array<std::pair<int, int>, 25> questItems = {{
        {181, 505}, {182, 499}, {183, 433}, {184, 506}, {185, 455}, {186, 457}, {187, 508}, {188, 434}, {189, 486},
        {190, 502}, {191, 550}, {192, 551}, {193, 552}, {194, 553}, {195, 456}, {196, 446}, {197, 461}, {198, 544},
        {199, 487}, {229, 538}, {230, 542}, {231, 537}, {232, 539}, {233, 541}, {234, 540}
    }};

    auto giveBack = [](int mm6Item) {
        Item item;
        item.itemId = itemIdFromMm6(mm6Item);
        item.flags = ITEM_IDENTIFIED;
        pParty->addItemToParty(&item, true);
        return fmt::sprintf(npcText(176), pItemTable->items[item.itemId].name); // NOLINT: this is not ::sprintf.
    };
    auto lost = [](int mm6Item) {
        ItemId item = itemIdFromMm6(mm6Item);
        return !pParty->hasItem(item) && pParty->pPickedItem.itemId != item;
    };

    // The ritual of the void (544) comes back only before the final battle.
    if (!pParty->_questBits[static_cast<QuestBit>(237)] && pParty->_questBits[static_cast<QuestBit>(177)] && lost(544))
        return giveBack(544);
    for (auto [bit, mm6Item] : questItems)
        if (pParty->_questBits[static_cast<QuestBit>(bit)] && lost(mm6Item))
            return giveBack(mm6Item);
    return npcText(175); // "You never had it."
}

int partyReputation() {
    int result = pParty->mm6Reputation;
    if (CheckHiredNPCSpeciality(Bard))
        result += 200;
    for (NpcProfession profession : {Pirate, Gypsy, Duper, Burglar})
        if (CheckHiredNPCSpeciality(profession))
            result -= 200;
    return result;
}

} // namespace mm6_topics
