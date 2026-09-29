#include "Tavern.h"

#include <map>
#include <string>
#include <vector>

#include "GUI/UI/Mm6NpcTalk.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"

#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"
#include "Engine/Engine.h"

#include "Arcomage/Arcomage.h"
#include "Engine/Graphics/Viewport.h"

#include "Media/MediaPlayer.h"

#include "Engine/MapEnums.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/NPCTable.h"

#include "Utility/GameVariant.h"

namespace {

bool hasArcomageDeck() {
    return isMm8() || pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK); // MM8 needs no deck.
}

// MM6 global.txt lines.
constexpr int MM6_STR_DRINK = 398;
constexpr int MM6_STR_TIP = 399;
constexpr int MM6_STR_UF = 564;
constexpr int MM6_STR_DRINK_FIRST = 565;

constexpr int MM6_DRINK_PRICE = 1;
constexpr int MM6_TIP_PRICE = 1;

/**
 * Per-tavern state of the current day: whether the party had a drink there, and the rumor it got. MM6 keeps the
 * same rumor for a tavern until the next day.
 */
struct Mm6TavernVisit {
    int day = -1;
    bool drank = false;
    std::string rumor;
};

std::map<HouseId, Mm6TavernVisit> mm6TavernVisits;

Mm6TavernVisit &mm6TavernVisit(HouseId house) {
    Mm6TavernVisit &visit = mm6TavernVisits[house];
    int day = pParty->GetPlayingTime().toDays();
    if (visit.day != day)
        visit = {day, false, {}};
    return visit;
}

std::string mm6RandomRumor() {
    std::vector<const std::string *> texts;
    for (const NPCNewsMm6 &news : pNPCStats->mm6News)
        if (news.mapId == mm6_talk::newsGroup())
            texts.push_back(&news.text);
    if (texts.empty())
        return {};
    return *texts[grng->random(texts.size())];
}

} // namespace

void GUIWindow_Tavern::mainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int foodNum = houseTable[houseId()].fPriceMultiplier;

    if (isMm6()) {
        std::vector<std::string> optionsText = {localization->format(LSTR_RENT_ROOM_FOR_D_GOLD, pPriceRoom),
                                                localization->format(LSTR_FILL_PACKS_TO_D_DAYS_FOR_D_GOLD, foodNum, pPriceFood),
                                                localization->mm6Str(MM6_STR_DRINK), localization->mm6Str(MM6_STR_TIP)};
        drawOptions(optionsText, colorTable.PaleCanary);
        DrawDialoguePanel(mm6TavernVisit(houseId()).rumor);
        return;
    }

    std::vector<std::string> optionsText = {localization->format(LSTR_RENT_ROOM_FOR_D_GOLD, pPriceRoom),
                                            localization->format(LSTR_FILL_PACKS_TO_D_DAYS_FOR_D_GOLD, foodNum, pPriceFood),
                                            localization->str(LSTR_LEARN_SKILLS)};

    if ((isMm8() || houseId() != HOUSE_TAVERN_EMERALD_ISLAND)) {
        optionsText.push_back(localization->str(LSTR_PLAY_ARCOMAGE));
    }

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Tavern::arcomageMainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText = {localization->str(LSTR_RULES), localization->str(LSTR_VICTORY_CONDITIONS)};
    if (hasArcomageDeck())
        optionsText.push_back(localization->str(LSTR_PLAY));

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Tavern::arcomageRulesDialogue() {
    DrawDialoguePanel(pNPCTopics[arcomageRulesTopic()].pText);
}

void GUIWindow_Tavern::arcomageVictoryCondDialogue() {
    DrawDialoguePanel(pNPCTopics[arcomageTopicForTavern(houseId())].pText);
}

void GUIWindow_Tavern::arcomageResultDialogue() {
    Recti dialog_window = this->frameRect;
    dialog_window.x = SIDE_TEXT_BOX_POS_X;
    dialog_window.w = SIDE_TEXT_BOX_WIDTH;

    if (!hasArcomageDeck()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }

    if (pArcomageGame->_gameInProgress == true) {
        return;
    }
    std::string pText;
    if (pArcomageGame->_gameWinner) {
        if (pArcomageGame->_gameWinner == 1)
            pText = localization->str(LSTR_YOU_WON);
        else
            pText = localization->str(LSTR_YOU_LOST);
    } else {
        pText = localization->str(LSTR_A_TIE);
    }
    int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(pText, dialog_window.w, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.PaleCanary, pText, 3, dialog_window);
}

void GUIWindow_Tavern::restDialogue() {
    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if (pParty->GetGold() >= pPriceRoom) {
        pParty->TakeGold(pPriceRoom);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_RENT_ROOM);
        _currentDialogue = DIALOGUE_NULL;
        houseDialogPressEscape();
        playHouseGoodbyeSpeech();
        pMediaPlayer->Unload();

        engine->_messageQueue->addMessageCurrentFrame(UIMSG_RentRoom, std::to_underlying(houseId()), 1);
        window_SpeakInHouse = nullptr;
        return;
    }
    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::buyFoodDialogue() {
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if ((double)pParty->GetFood() >= houseTable[houseId()].fPriceMultiplier) {
        engine->_statusBar->setEvent(LSTR_YOUR_PACKS_ARE_ALREADY_FULL);
        if (pParty->hasActiveCharacter()) {
            pParty->activeCharacter().playReaction(SPEECH_PACKS_FULL);
        }
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }
    if (pParty->GetGold() >= pPriceFood) {
        pParty->TakeGold(pPriceFood);
        pParty->SetFood(houseTable[houseId()].fPriceMultiplier);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_BUY_FOOD);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }
    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

/**
 * @offset 0x49F45C in MM6.exe.
 */
void GUIWindow_Tavern::drinkDialogueMm6() {
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    if (pParty->GetGold() < MM6_DRINK_PRICE) {
        engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
        return;
    }

    pParty->TakeGold(MM6_DRINK_PRICE);
    engine->_statusBar->setEvent(localization->mm6Str(MM6_STR_UF));
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_BUY_FOOD);
    mm6TavernVisit(houseId()).drank = true;

    Character &character = pParty->activeCharacter();
    if (grng->randomBool()) {
        character.playReaction(SPEECH_TAVERN_GOT_DRUNK);
        if (grng->random(3) == 1)
            character.SetCondition(CONDITION_DRUNK, false);
        return;
    }
    if (grng->random(4) == 1) {
        Attribute stat = static_cast<Attribute>(std::to_underlying(ATTRIBUTE_FIRST_STAT) + grng->random(7));
        character._statBonuses[stat] += 5 + grng->random(6);
    }
    character.playReaction(SPEECH_TAVERN_DRINK);
}

/**
 * @offset 0x49F716 in MM6.exe.
 */
void GUIWindow_Tavern::tipDialogueMm6() {
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    Mm6TavernVisit &visit = mm6TavernVisit(houseId());
    if (!visit.drank) {
        engine->_statusBar->setEvent(localization->mm6Str(MM6_STR_DRINK_FIRST));
        return;
    }
    if (pParty->GetGold() < MM6_TIP_PRICE) {
        engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
        return;
    }

    pParty->TakeGold(MM6_TIP_PRICE);
    if (visit.rumor.empty())
        visit.rumor = mm6RandomRumor();
    pParty->activeCharacter().playReaction(SPEECH_TAVERN_TIP);
}

void GUIWindow_Tavern::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_PlayArcomage, 0, 0);
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Tavern::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        arcomageMainDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_RULES:
        arcomageRulesDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
        arcomageVictoryCondDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
        arcomageResultDialogue();
        break;
      case DIALOGUE_TAVERN_REST:
        restDialogue();
        break;
      case DIALOGUE_TAVERN_BUY_FOOD:
        buyFoodDialogue();
        break;
      case DIALOGUE_TAVERN_DRINK:
        drinkDialogueMm6();
        break;
      case DIALOGUE_TAVERN_TIP:
        tipDialogueMm6();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.PaleCanary);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DialogueId> GUIWindow_Tavern::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        if (isMm6()) {
            return {DIALOGUE_TAVERN_REST, DIALOGUE_TAVERN_BUY_FOOD, DIALOGUE_TAVERN_DRINK, DIALOGUE_TAVERN_TIP};
        } else if (!isMm8() && houseId() == HOUSE_TAVERN_EMERALD_ISLAND) {
            return {DIALOGUE_TAVERN_REST, DIALOGUE_TAVERN_BUY_FOOD, DIALOGUE_LEARN_SKILLS};
        } else {
            return {DIALOGUE_TAVERN_REST, DIALOGUE_TAVERN_BUY_FOOD, DIALOGUE_LEARN_SKILLS, DIALOGUE_TAVERN_ARCOMAGE_MAIN};
        }
      case DIALOGUE_LEARN_SKILLS:
        return {DIALOGUE_LEARN_STEALING, DIALOGUE_LEARN_TRAP_DISARM, DIALOGUE_LEARN_PERCEPTION};
      case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        if (hasArcomageDeck()) {
            return {DIALOGUE_TAVERN_ARCOMAGE_RULES, DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS, DIALOGUE_TAVERN_ARCOMAGE_RESULT};
        } else {
            return {DIALOGUE_TAVERN_ARCOMAGE_RULES, DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS};
        }
      default:
        return {};
    }
}

void GUIWindow_Tavern::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_RULES ||
        _currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS ||
        _currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        _currentDialogue = DIALOGUE_TAVERN_ARCOMAGE_MAIN;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}
