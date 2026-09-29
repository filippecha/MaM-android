#include <array>
#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/AutonotesBook.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

GraphicsImage *ui_book_autonotes_background = nullptr;

AutonoteType autonoteBookDisplayType;

namespace {

struct Mm8NotesRibbon {
    Pointi pos;
    BookButtonAction action;
    AutonoteType type;
    int title; // MM8 global.txt line, also the hint of the ribbon.
};

constexpr std::array<Mm8NotesRibbon, 6> MM8_NOTES_RIBBONS = {{
    {{483, 143}, BOOK_NOTES_POTION, AUTONOTE_POTION_RECIPE, 741},
    {{483, 192}, BOOK_NOTES_FOUNTAIN, AUTONOTE_STAT_HINT, 742},
    {{484, 241}, BOOK_NOTES_OBELISK, AUTONOTE_OBELISK, 743},
    {{484, 291}, BOOK_NOTES_SEER, AUTONOTE_SEER, 744},
    {{483, 343}, BOOK_NOTES_MISC, AUTONOTE_MISC, 745},
    {{482, 391}, BOOK_NOTES_INSTRUCTORS, AUTONOTE_TEACHER, 746},
}};

} // namespace

void GUIWindow_AutonotesBook::recalculateCurrentNotesTypePages() {
    _startingNotesIdx = 0;
    _currentPage = 0;
    _currentPageNotes = 0;
    _activeNotesIdx.clear();
    for (int i : pParty->_autonoteBits.indices())
        if (pParty->_autonoteBits[i] && autonoteBookDisplayType == pAutonoteTxt[i].eType && !pAutonoteTxt[i].pText.empty())
            _activeNotesIdx.push_back(i);
}

GUIWindow_AutonotesBook::GUIWindow_AutonotesBook() : GUIWindow_Book() {
    this->eWindowType = WindowType::WINDOW_AutonotesBook;

    bFlashAutonotesBook = false;
    if (isMm8()) {
        // The ribbons of MM8.exe 0x4CF23A, the notes categories start with the third one.
        createMm8Frame(MM8_BOOK_NOTES);
        ui_book_quest_div_bar = assets->getImage_Alpha("divbar");
        pBtn_Book_1 = createMm8Ribbon(1, MM8_PREV_PAGE_POS, BOOK_PREV_PAGE, INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_DOWN));
        pBtn_Book_2 = createMm8Ribbon(2, MM8_NEXT_PAGE_POS, BOOK_NEXT_PAGE, INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_UP));
        for (int i = 0; i < MM8_NOTES_RIBBONS.size(); i++)
            createMm8Ribbon(i + 3, MM8_NOTES_RIBBONS[i].pos, MM8_NOTES_RIBBONS[i].action, INPUT_ACTION_INVALID, localization->mm8Str(MM8_NOTES_RIBBONS[i].title));
        recalculateCurrentNotesTypePages();
        return;
    }
    pChildBooksOverlay = std::make_unique<GUIWindow_BooksButtonOverlay>(Pointi{527, 353}, Sizei{0, 0}, pBtn_Autonotes);

    ui_book_autonotes_background = assets->getImage_ColorKey(isMm6() ? "note_bg" : "sbautnot");
    ui_book_quest_div_bar = assets->getImage_Alpha("divbar");

    if (isMm6()) {
        ui_book_button1_on = assets->getImage_Alpha("tab+on");
        ui_book_button2_on = assets->getImage_Alpha("tab--on");
        ui_book_button3_on = assets->getImage_Alpha("anot1on");
        ui_book_button4_on = assets->getImage_Alpha("anot2on");
        ui_book_button5_on = assets->getImage_Alpha("anot3on");
        ui_book_button6_on = assets->getImage_Alpha("anot5on");
        ui_book_button7_on = assets->getImage_Alpha("anot4on");
        ui_book_button1_off = assets->getImage_Alpha("tab+off");
        ui_book_button2_off = assets->getImage_Alpha("tab--off");
        ui_book_button3_off = assets->getImage_Alpha("anot1off");
        ui_book_button4_off = assets->getImage_Alpha("anot2off");
        ui_book_button5_off = assets->getImage_Alpha("anot3off");
        ui_book_button6_off = assets->getImage_Alpha("anot5off");
        ui_book_button7_off = assets->getImage_Alpha("anot4off");
    } else {
        ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
        ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
        ui_book_button3_on = assets->getImage_Alpha("tab-an-1b");
        ui_book_button4_on = assets->getImage_Alpha("tab-an-2b");
        ui_book_button5_on = assets->getImage_Alpha("tab-an-3b");
        ui_book_button6_on = assets->getImage_Alpha("tab-an-5b");
        ui_book_button7_on = assets->getImage_Alpha("tab-an-4b");
        ui_book_button8_on = assets->getImage_Alpha("tab-an-8b");
        ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
        ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");
        ui_book_button3_off = assets->getImage_Alpha("tab-an-1a");
        ui_book_button4_off = assets->getImage_Alpha("tab-an-2a");
        ui_book_button5_off = assets->getImage_Alpha("tab-an-3a");
        ui_book_button6_off = assets->getImage_Alpha("tab-an-5a");
        ui_book_button7_off = assets->getImage_Alpha("tab-an-4a");
        ui_book_button8_off = assets->getImage_Alpha("tab-an-8a");
    }

    pBtn_Book_1 = CreateButton(pViewport.topLeft() + buttonPos({398, 1}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + buttonPos({398, 38}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_UP), {ui_book_button2_on});
    pBtn_Book_3 = CreateButton(pViewport.topLeft() + buttonPos({398, 113}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_POTION), INPUT_ACTION_INVALID, localization->str(LSTR_POTION_NOTES), {ui_book_button3_on});
    pBtn_Book_4 = CreateButton(pViewport.topLeft() + buttonPos({399, 150}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_FOUNTAIN), INPUT_ACTION_INVALID, localization->str(LSTR_FOUNTAIN_NOTES), {ui_book_button4_on});
    pBtn_Book_5 = CreateButton(pViewport.topLeft() + buttonPos({397, 188}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_OBELISK), INPUT_ACTION_INVALID, localization->str(LSTR_OBELISK_NOTES), {ui_book_button5_on});
    pBtn_Book_6 = CreateButton(pViewport.topLeft() + buttonPos({397, 226}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_SEER), INPUT_ACTION_INVALID, localization->str(LSTR_SEER_NOTES), {ui_book_button6_on});
    pBtn_Autonotes_Misc = CreateButton(pViewport.topLeft() + buttonPos({397, 264}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_MISC), INPUT_ACTION_INVALID, localization->str(LSTR_MISCELLANEOUS_NOTES), {ui_book_button7_on});
    if (!isMm6()) // MM6 has no instructor notes.
        pBtn_Autonotes_Instructors = CreateButton(pViewport.topLeft() + buttonPos({397, 302}), {50, 34}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_INSTRUCTORS), INPUT_ACTION_INVALID, localization->str(LSTR_INSTRUCTORS), {ui_book_button8_on});

    recalculateCurrentNotesTypePages();
}

void GUIWindow_AutonotesBook::updateMm8() {
    drawMm8Frame(MM8_BOOK_NOTES);
    drawMm8Ribbon(1, MM8_PREV_PAGE_POS, BOOK_PREV_PAGE);
    drawMm8Ribbon(2, MM8_NEXT_PAGE_POS, BOOK_NEXT_PAGE);
    std::string title = localization->str(LSTR_AUTO_NOTES);
    for (int i = 0; i < MM8_NOTES_RIBBONS.size(); i++) {
        const Mm8NotesRibbon &ribbon = MM8_NOTES_RIBBONS[i];
        drawMm8Ribbon(i + 3, ribbon.pos, ribbon.action);
        if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == ribbon.action && autonoteBookDisplayType != ribbon.type) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = ribbon.type;
            recalculateCurrentNotesTypePages();
        }
        if (autonoteBookDisplayType == ribbon.type)
            title = localization->mm8Str(ribbon.title);
    }
    DrawTitleText(assets->pFontBookTitle.get(), 0, -5, ui_book_autonotes_title_color, title, 3, mm8TitleRect()); // Like the history.
}

void GUIWindow_AutonotesBook::Update() {
    bool noteTypeChanged = false;

    if (isMm8()) {
        updateMm8();
        drawNotes(Recti(MM8_TEXT_X, 87, MM8_TEXT_WIDTH, 425), MM8_DIV_BAR_X);
        return;
    }

    drawExitButton();

    drawBackground(ui_book_autonotes_background);
    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_startingNotesIdx) {
        render->DrawQuad2D(ui_book_button1_off, pViewport.topLeft() + buttonPos({407, 2}));
    } else {
        render->DrawQuad2D(ui_book_button1_on, pViewport.topLeft() + buttonPos({398, 1}));
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_startingNotesIdx + _currentPageNotes) >= _activeNotesIdx.size()) {
        render->DrawQuad2D(ui_book_button2_off, pViewport.topLeft() + buttonPos({407, 38}));
    } else {
        render->DrawQuad2D(ui_book_button2_on, pViewport.topLeft() + buttonPos({398, 38}));
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_POTION) {
        if (autonoteBookDisplayType == AUTONOTE_POTION_RECIPE) {
            render->DrawQuad2D(ui_book_button3_on, pViewport.topLeft() + buttonPos({398, 113}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_POTION_RECIPE;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button3_on, pViewport.topLeft() + buttonPos({398, 113}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_POTION_RECIPE) {
            render->DrawQuad2D(ui_book_button3_on, pViewport.topLeft() + buttonPos({398, 113}));
        } else {
            render->DrawQuad2D(ui_book_button3_off, pViewport.topLeft() + buttonPos({408, 113}));
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_FOUNTAIN) {
        if (autonoteBookDisplayType == AUTONOTE_STAT_HINT) {
            render->DrawQuad2D(ui_book_button4_on, pViewport.topLeft() + buttonPos({399, 150}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_STAT_HINT;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button4_on, pViewport.topLeft() + buttonPos({399, 150}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_STAT_HINT) {
            render->DrawQuad2D(ui_book_button4_on, pViewport.topLeft() + buttonPos({399, 150}));
        } else {
            render->DrawQuad2D(ui_book_button4_off, pViewport.topLeft() + buttonPos({408, 150}));
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_OBELISK) {
        if (autonoteBookDisplayType == AUTONOTE_OBELISK) {
            render->DrawQuad2D(ui_book_button5_on, pViewport.topLeft() + buttonPos({397, 188}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_OBELISK;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button5_on, pViewport.topLeft() + buttonPos({397, 188}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_OBELISK) {
            render->DrawQuad2D(ui_book_button5_on, pViewport.topLeft() + buttonPos({397, 188}));
        } else {
            render->DrawQuad2D(ui_book_button5_off, pViewport.topLeft() + buttonPos({408, 188}));
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_SEER) {
        if (autonoteBookDisplayType == AUTONOTE_SEER) {
            render->DrawQuad2D(ui_book_button6_on, pViewport.topLeft() + buttonPos({397, 226}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_SEER;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button6_on, pViewport.topLeft() + buttonPos({397, 226}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_SEER) {
            render->DrawQuad2D(ui_book_button6_on, pViewport.topLeft() + buttonPos({397, 226}));
        } else {
            render->DrawQuad2D(ui_book_button6_off, pViewport.topLeft() + buttonPos({408, 226}));
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_MISC) {
        if (autonoteBookDisplayType == AUTONOTE_MISC) {
            render->DrawQuad2D(ui_book_button7_on, pViewport.topLeft() + buttonPos({397, 264}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_MISC;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button7_on, pViewport.topLeft() + buttonPos({397, 264}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_MISC) {
            render->DrawQuad2D(ui_book_button7_on, pViewport.topLeft() + buttonPos({397, 264}));
        } else {
            render->DrawQuad2D(ui_book_button7_off, pViewport.topLeft() + buttonPos({408, 263}));
        }
    }

    if (isMm6()) {
    } else if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_INSTRUCTORS) {
        if (autonoteBookDisplayType == AUTONOTE_TEACHER) {
            render->DrawQuad2D(ui_book_button8_on, pViewport.topLeft() + buttonPos({397, 302}));
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_TEACHER;
            noteTypeChanged = true;
            render->DrawQuad2D(ui_book_button8_on, pViewport.topLeft() + buttonPos({397, 302}));
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_TEACHER) {
            render->DrawQuad2D(ui_book_button8_on, pViewport.topLeft() + buttonPos({397, 302}));
        } else {
            render->DrawQuad2D(ui_book_button8_off, pViewport.topLeft() + buttonPos({408, 302}));
        }
    }

    // for title
    DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_autonotes_title_color, localization->str(LSTR_AUTO_NOTES), 3, titleRect());

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && noteTypeChanged &&
        _bookButtonAction >= BOOK_NOTES_POTION && _bookButtonAction <= BOOK_NOTES_INSTRUCTORS)
        recalculateCurrentNotesTypePages();
    drawNotes(Recti(48, 70, 360, 264), 100);
}

void GUIWindow_AutonotesBook::drawNotes(Recti autonotes_frameRect, int divBarX) {
    int pTextHeight;
    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES) {
        if (_bookButtonAction < BOOK_NOTES_POTION || _bookButtonAction > BOOK_NOTES_INSTRUCTORS) {
            if (_bookButtonAction == BOOK_NEXT_PAGE && (_startingNotesIdx + _currentPageNotes) < _activeNotesIdx.size()) {
                pAudioPlayer->playUISound(SOUND_openbook);
                _startingNotesIdx += _currentPageNotes;
                _notesPerPage[_currentPage] = _currentPageNotes;
                _currentPage++;
            }
            if (_bookButtonAction == BOOK_PREV_PAGE && _startingNotesIdx) {
                pAudioPlayer->playUISound(SOUND_openbook);
                _currentPage--;
                _startingNotesIdx -= _notesPerPage[_currentPage];
            }
        }
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    _currentPageNotes = 0;

    for (int i = _startingNotesIdx; i < _activeNotesIdx.size(); ++i) {
        _currentPageNotes++;

        DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_autonotes_text_color, pAutonoteTxt[_activeNotesIdx[i]].pText, autonotes_frameRect);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pAutonoteTxt[_activeNotesIdx[i]].pText, autonotes_frameRect.w, 1);
        if ((autonotes_frameRect.y + pTextHeight) > autonotes_frameRect.h) {
            break;
        }

        render->DrawQuad2D(ui_book_quest_div_bar, {divBarX, autonotes_frameRect.y + pTextHeight + 12});
        autonotes_frameRect.y = (autonotes_frameRect.y + pTextHeight) + 24;
    }
}
