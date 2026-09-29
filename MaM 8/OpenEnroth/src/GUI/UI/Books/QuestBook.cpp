#include <memory>

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/mm7_data.h"

#include "Engine/Tables/QuestTable.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/QuestBook.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

GraphicsImage *ui_book_quests_background = nullptr;

GUIWindow_QuestBook::GUIWindow_QuestBook() {
    this->eWindowType = WindowType::WINDOW_QuestBook;

    bFlashQuestBook = false;
    if (isMm8()) {
        createMm8Frame(MM8_BOOK_QUESTS);
        ui_book_quest_div_bar = assets->getImage_Alpha("divbar");
        pBtn_Book_1 = createMm8Ribbon(1, MM8_PREV_PAGE_POS, BOOK_PREV_PAGE, INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP));
        pBtn_Book_2 = createMm8Ribbon(2, MM8_NEXT_PAGE_POS, BOOK_NEXT_PAGE, INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN));
        collectActiveQuests();
        return;
    }
    pChildBooksOverlay = std::make_unique<GUIWindow_BooksButtonOverlay>(Pointi{493, 355}, Sizei{0, 0}, pBtn_Quests);

    ui_book_quests_background = assets->getImage_Solid(isMm6() ? "quest_bg" : "sbquiknot");
    ui_book_quest_div_bar = assets->getImage_Alpha("divbar");

    ui_book_button1_on = assets->getImage_Alpha(isMm6() ? "tab+on" : "tab-an-6b");
    ui_book_button2_on = assets->getImage_Alpha(isMm6() ? "tab--on" : "tab-an-7b");
    ui_book_button1_off = assets->getImage_Alpha(isMm6() ? "tab+off" : "tab-an-6a");
    ui_book_button2_off = assets->getImage_Alpha(isMm6() ? "tab--off" : "tab-an-7a");

    pBtn_Book_1 = CreateButton(pViewport.topLeft() + buttonPos({398, 1}), ui_book_button1_on->size(), BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + buttonPos({398, 38}), ui_book_button2_on->size(), BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    collectActiveQuests();
}

void GUIWindow_QuestBook::collectActiveQuests() {
    for (auto i : pQuestTable.indices()) {
        if (pParty->_questBits[i] && !pQuestTable[i].empty()) {
            _activeQuestsIdx.push_back(i);
        }
    }
}

void GUIWindow_QuestBook::Update() {
    int pTextHeight;
    bool morePages = (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size();
    if (isMm8()) {
        drawMm8Frame(MM8_BOOK_QUESTS);
        drawMm8Ribbon(1, MM8_PREV_PAGE_POS, BOOK_PREV_PAGE);
        drawMm8Ribbon(2, MM8_NEXT_PAGE_POS, BOOK_NEXT_PAGE);
    } else {
        drawExitButton();
        drawBackground(ui_book_quests_background);

        if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_startingQuestIdx) {
            render->DrawQuad2D(ui_book_button1_off, pViewport.topLeft() + buttonPos({407, 2}));
        } else {
            render->DrawQuad2D(ui_book_button1_on, pViewport.topLeft() + buttonPos({398, 1}));
        }

        if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || !morePages) {
            render->DrawQuad2D(ui_book_button2_off, pViewport.topLeft() + buttonPos({407, 38}));
        } else {
            render->DrawQuad2D(ui_book_button2_on, pViewport.topLeft() + buttonPos({398, 38}));
        }
    }

    // for title
    DrawTitleText(assets->pFontBookTitle.get(), 0, isMm8() ? 0 : 22, ui_book_quests_title_color, localization->str(LSTR_CURRENT_QUESTS), 3,
                  isMm8() ? mm8TitleRect() : titleRect());

    // for other text, the height is the bottom of the text
    Recti questbook_window = isMm8() ? Recti(MM8_TEXT_X, 92, MM8_TEXT_WIDTH, 425) : Recti(48, 70, 360, 264);

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_NEXT_PAGE && (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _startingQuestIdx += _currentPageQuests;
        _questsPerPage[_currentPage] = _currentPageQuests;
        _currentPage++;
    }

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_PREV_PAGE && _startingQuestIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentPage--;
        _startingQuestIdx -= _questsPerPage[_currentPage];
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    _currentPageQuests = 0;

    for (int i = _startingQuestIdx; i < _activeQuestsIdx.size(); ++i) {
        _currentPageQuests++;

        DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_quests_text_color, pQuestTable[_activeQuestsIdx[i]], questbook_window);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pQuestTable[_activeQuestsIdx[i]], questbook_window.w, 1);
        if ((questbook_window.y + pTextHeight) > questbook_window.h) {
            break;
        }

        render->DrawQuad2D(ui_book_quest_div_bar, {isMm8() ? MM8_DIV_BAR_X : 100, (questbook_window.y + pTextHeight) + 12});
        questbook_window.y = (questbook_window.y + pTextHeight) + 24;
    }
}
