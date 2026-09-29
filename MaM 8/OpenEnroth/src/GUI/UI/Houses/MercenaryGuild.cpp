#include "GUI/UI/Houses/MercenaryGuild.h"

#include <array>
#include <vector>

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Media/Audio/AudioPlayer.h"

#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Tables/NPCTable.h"

#include "Utility/GameVariant.h"

std::array<AwardId, 49> word_4F0754;

/**
 * @param house                         MM6 mercenary or thieves guild, houses 141-152.
 * @return                              Skills the guild teaches, as its buttons are made in MM6.exe 0x498EC4 and
 *                                      0x4991E3.
 */
static std::vector<Skill> mm6GuildSkills(HouseId house) {
    static const std::array<std::vector<Skill>, 12> skills = {{
        {SKILL_SWORD, SKILL_AXE, SKILL_SPEAR, SKILL_STAFF, SKILL_LEATHER},                  // 141, Broken Blade.
        {SKILL_MACE, SKILL_BOW, SKILL_CHAIN, SKILL_SHIELD, SKILL_BODYBUILDING},             // 142, Double Edge.
        {SKILL_CHAIN, SKILL_BOW, SKILL_SHIELD, SKILL_PLATE, SKILL_REPAIR},                  // 143, Berserkers' Fury.
        {SKILL_MACE, SKILL_BOW, SKILL_CHAIN, SKILL_SHIELD, SKILL_BODYBUILDING},             // 144
        {SKILL_SWORD, SKILL_AXE, SKILL_SPEAR, SKILL_STAFF, SKILL_REPAIR},                   // 145
        {SKILL_CHAIN, SKILL_BOW, SKILL_SHIELD, SKILL_PLATE, SKILL_REPAIR},                  // 146
        {SKILL_DAGGER, SKILL_MERCHANT, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM}, // 147, Buccaneers' Lair.
        {SKILL_LEATHER, SKILL_DIPLOMACY, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM},
        {SKILL_DAGGER, SKILL_MERCHANT, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM},
        {SKILL_LEATHER, SKILL_DIPLOMACY, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM},
        {SKILL_DAGGER, SKILL_MERCHANT, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM},
        {SKILL_LEATHER, SKILL_DIPLOMACY, SKILL_ITEM_ID, SKILL_PERCEPTION, SKILL_TRAP_DISARM}, // 152
    }};
    int index = std::to_underlying(house) - 141;
    if (index < 0 || index >= skills.size())
        return {};
    return skills[index];
}

std::vector<DialogueId> GUIWindow_MercenaryGuild::listDialogueOptions() {
    if (!isMm6() || _currentDialogue != DIALOGUE_MAIN)
        return {};
    std::vector<DialogueId> result;
    for (Skill skill : mm6GuildSkills(houseId()))
        result.push_back(static_cast<DialogueId>(std::to_underlying(DIALOGUE_LEARN_FIRST) + std::to_underlying(skill) -
                                                 std::to_underlying(SKILL_FIRST_VISIBLE)));
    return result;
}

void GUIWindow_MercenaryGuild::updateDialogueOnEscape() {
    if (isMm6() && IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_MAIN;
        return;
    }
    GUIWindow_House::updateDialogueOnEscape();
}

void GUIWindow_MercenaryGuild::houseSpecificDialogue() {
    Recti dialog_window = this->frameRect;
    dialog_window.x = SIDE_TEXT_BOX_POS_X;
    dialog_window.w = SIDE_TEXT_BOX_WIDTH;

    if (isMm6()) {
        if (!pParty->activeCharacter()._achievedAwardsBits[mm6GuildAward(houseId())]) {
            int textHeight = assets->pFontArrus->CalcTextHeight(pNPCTopics[171].pText, dialog_window.w, 0);
            DrawTitleText(assets->pFontArrus.get(), 0, (212 - textHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[171].pText, 3, dialog_window);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        if (_currentDialogue == DIALOGUE_MAIN)
            learnSkillsDialogue(colorTable.PaleCanary);
        return;
    }

    /**
     * archiving this code just in case
     * I believe it is 250 gold cost for mercenary guild from mm6 and 100 for all other skill-learning house types in mm6
     * but they aren't used in mm7, so I'm gonna assume 250 gold cost in price calculator
     *
     *  int v32 = (uint8_t)(((houseTable[window_SpeakInHouse->houseId()].uType != BuildingType_MercenaryGuild) - 1) & 0x96) + 100;
     *  int v3 = (int64_t)((double)v32 * houseTable[window_SpeakInHouse->houseId()].fPriceMultiplier);
     *  pPrice = v3 * (100 - PriceCalculator::playerMerchant(&pParty->activeCharacter())) / 100;
     *  if (pPrice < v3 / 3) pPrice = v3 / 3;
     */
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), houseTable[window_SpeakInHouse->houseId()]);

    if (_currentDialogue == DIALOGUE_MAIN) {
        if (!pParty->activeCharacter()._achievedAwardsBits[word_4F0754[2 * std::to_underlying(window_SpeakInHouse->houseId())]]) {
            // 171 looks like Mercenary Stronghold message from NPCNews.txt in MM6
            int pTextHeight = assets->pFontArrus->CalcTextHeight(pNPCTopics[171].pText, dialog_window.w, 0);
            DrawTitleText(assets->pFontArrus.get(), 0, (212 - pTextHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[171].pText, 3, dialog_window);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        learnSkillsDialogue(colorTable.PaleCanary);
        return;
    }

    if (checkIfPlayerCanInteract()) {
        assert(false);  // what type of house that even is?
        // pSkillAvailabilityPerClass[8 + v58->uClass][4 + v23]
        // or
        // skillMaxMasteryPerClass[v58->uClass][v23 - 36]
        // or
        // skillMaxMasteryPerClass[v58->uClass - 1][v23 +
        // 1]
        assert(false);  // whacky condition - fix
        short *v6 = nullptr;
        if (false) {
            // TODO(captainurist): #mm6 this is MM6 legacy, and this decompiled code doesn't look sane.
            //                     Reimplement properly once we get to MM6.
            // if ( !*(&byte_4ED94C[37 * v1->uClass / 3] + dword_F8B19C)
            //|| (v6 = (short *)(&pParty->activeCharacter()._stats[ATTRIBUTE_INTELLIGENCE] + _currentDialogue),
            //    *(short *)v6))
            pAudioPlayer->playUISound(SOUND_error);
        } else {
            int  v27;
            if (pParty->GetGold() < pPrice) {
                engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
                v27 = 4;
            } else {
                pParty->TakeGold(pPrice);
                *(short *)v6 = 1;
                v27 = 2;
            }
            playHouseSound(houseId(), HouseSoundType(v27));
        }
    }
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_MercenaryGuild::houseDialogueOptionSelected(DialogueId option) {
    if (isMm6() && IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
        return;
    }
    _currentDialogue = option;
}
