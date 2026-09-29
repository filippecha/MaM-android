#include <array>
#include <cstdlib>
#include <memory>
#include <utility>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Timer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "GUI/UI/UIGame.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

namespace {

constexpr Pointi MM6_BOOK_POS = {8, 8};
constexpr Pointi MM6_EXIT_POS = {360, 332};

} // namespace

GUIWindow_Book::~GUIWindow_Book() {
    if (_mm6Book)
        _mm6Book->release();
    if (_mm6Exit)
        _mm6Exit->release();
    if (ui_book_map_frame) {
        ui_book_map_frame->release();
    }
    if (ui_book_quest_div_bar) {
        ui_book_quest_div_bar->release();
    }
    if (ui_book_button8_off) {
        ui_book_button8_off->release();
    }
    if (ui_book_button8_on) {
        ui_book_button8_on->release();
    }
    if (ui_book_button7_off) {
        ui_book_button7_off->release();
    }
    if (ui_book_button7_on) {
        ui_book_button7_on->release();
    }
    if (ui_book_button6_off) {
        ui_book_button6_off->release();
    }
    if (ui_book_button6_on) {
        ui_book_button6_on->release();
    }
    if (ui_book_button5_off) {
        ui_book_button5_off->release();
    }
    if (ui_book_button5_on) {
        ui_book_button5_on->release();
    }
    if (ui_book_button4_off) {
        ui_book_button4_off->release();
    }
    if (ui_book_button4_on) {
        ui_book_button4_on->release();
    }
    if (ui_book_button3_off) {
        ui_book_button3_off->release();
    }
    if (ui_book_button3_on) {
        ui_book_button3_on->release();
    }
    if (ui_book_button2_off) {
        ui_book_button2_off->release();
    }
    if (ui_book_button2_on) {
        ui_book_button2_on->release();
    }
    if (ui_book_button1_off) {
        ui_book_button1_off->release();
    }
    if (ui_book_button1_on) {
        ui_book_button1_on->release();
    }

    pAudioPlayer->playUISound(SOUND_closebook);

    pChildBooksOverlay = nullptr;
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_Book, {0, 0}, render->GetRenderDimensions()) {
    initializeFonts();
    if (isMm6()) {
        _mm6Book = assets->getImage_ColorKey("book");
        _mm6Exit = assets->getImage_ColorKey("tabexit");
        CreateButton(MM6_EXIT_POS, _mm6Exit->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE));
    } else {
        CreateButton({475, 445}, {158, 34}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE));
    }
    current_screen_type = SCREEN_BOOKS;
    gameTimer->setPaused(true);
}

void GUIWindow_Book::initializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_book_map_frame = assets->getImage_Alpha("mapbordr");

    if (!assets->pFontBookCalendar)
        assets->pFontBookCalendar = GUIFont::LoadFont("book.fnt");
    if (!assets->pFontBookTitle)
        assets->pFontBookTitle = GUIFont::LoadFont("book2.fnt");
    if (!assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow = GUIFont::LoadFont("autonote.fnt");
    if (!assets->pFontBookLloyds)
        assets->pFontBookLloyds = GUIFont::LoadFont("spell.fnt");
}

void GUIWindow_Book::drawBackground(GraphicsImage *page, Pointi mm6PagePos) {
    if (!isMm6()) {
        render->DrawQuad2D(page, pViewport.topLeft());
        return;
    }
    render->DrawQuad2D(_mm6Book, MM6_BOOK_POS);
    if (page)
        render->DrawQuad2D(page, mm6PagePos);
    render->DrawQuad2D(_mm6Exit, MM6_EXIT_POS); // The exit tab sits on the book.
}

void GUIWindow_Book::drawExitButton() {
    if (!isMm6())
        render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});
}

Pointi GUIWindow_Book::buttonPos(Pointi mm7Pos) {
    if (!isMm6())
        return mm7Pos;

    // Measured on the original game. Pressed and released tabs are 3 pixels apart, like in MM7.
    static constexpr std::array<std::pair<int, int>, 8> rows = {{{1, 5}, {38, 40}, {113, 110}, {150, 145}, {188, 180}, {226, 215}, {263, 250}, {302, 285}}};
    int y = mm7Pos.y;
    for (auto [mm7Y, mm6Y] : rows)
        if (std::abs(mm7Pos.y - mm7Y) <= 2)
            y = mm6Y;
    return {mm7Pos.x >= 405 ? 410 : 407, y};
}

Recti GUIWindow_Book::titleRect() {
    if (!isMm6())
        return pViewport;
    return Recti(MM6_PAGE_POS.x, MM6_PAGE_POS.y - 8, 360, 300);
}

void GUIWindow_Book::bookButtonClicked(BookButtonAction action) {
    _bookButtonClicked = BOOK_BUTTON_PRESSED_FRAMES;
    _bookButtonAction = action;
}

GUIWindow_BooksButtonOverlay::GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint) :
    GUIWindow(WINDOW_BooksButtonOverlay, position, dimensions, hint),
    _button(button)
{}

void GUIWindow_BooksButtonOverlay::Update() {
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
}
