#include <array>
#include <cstdlib>
#include <memory>
#include <utility>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Timer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "GUI/UI/UIGame.h"

#include "Io/Mouse.h"

#include "Library/Platform/Interface/Platform.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

namespace {

constexpr Pointi MM6_BOOK_POS = {8, 8};
constexpr Pointi MM6_EXIT_POS = {360, 332};

constexpr int MM8_ICON_X = 556;
constexpr std::array<int, 4> MM8_ICON_YS = {24, 123, 222, 321};
constexpr std::array<int, 4> MM8_ICON_FRAMES = {10, 10, 9, 10};
constexpr Sizei MM8_ICON_SIZE = {80, 80};

} // namespace

Mm8BookPage mm8LastBookPage = MM8_BOOK_MAP;

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
    } else if (!isMm8()) { // MM8 books create their frame with createMm8Frame.
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

void GUIWindow_Book::createMm8Frame(Mm8BookPage page) {
    mm8LastBookPage = page;
    createMm8CloseButton();

    // Param 1 switches to the book, while the keys close a book that is already open.
    const std::array<std::pair<UIMessageType, LstrId>, 4> books = {{{UIMSG_OpenMapBook, LSTR_MAPS}, {UIMSG_OpenQuestBook, LSTR_CURRENT_QUESTS},
                                                                     {UIMSG_OpenHistoryBook, LSTR_HISTORY}, {UIMSG_OpenAutonotes, LSTR_AUTO_NOTES}}};
    for (int i = 0; i < books.size(); i++)
        CreateButton({MM8_ICON_X, MM8_ICON_YS[i]}, MM8_ICON_SIZE, BUTTON_TYPE_NORMAL, 0, books[i].first, 1, INPUT_ACTION_INVALID,
                     localization->str(books[i].second));
}

void GUIWindow_Book::drawMm8Frame(Mm8BookPage page) {
    render->DrawQuad2D(assets->getImage_Solid("irbgrnd"), {0, 0});
    render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("IRB-{}", std::to_underlying(page) + 1)), {0, 0});

    // The globe keeps turning, the other icons move while the mouse is over them.
    Pointi mousePos = mouse->position();
    for (int i = 0; i < MM8_ICON_YS.size(); i++) {
        Pointi pos(MM8_ICON_X, MM8_ICON_YS[i]);
        bool moving = i == 0 || Recti(pos, MM8_ICON_SIZE).contains(mousePos);
        int frame = moving ? (platform->tickCount() / 100) % MM8_ICON_FRAMES[i] : 0;
        render->DrawQuad2D(assets->getImage_Solid(fmt::format("IRA-{}_{:02}", i + 1, frame + 1)), pos);
    }

    drawMm8CloseButton();
}

void GUIWindow_Book::createMm8CloseButton(Pointi pos) {
    GraphicsImage *close = assets->getImage_ColorKey("c_close_up");
    CreateButton(pos, close->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE),
                 {assets->getImage_ColorKey("c_close_dn")});
}

void GUIWindow_Book::drawMm8CloseButton(Pointi pos) {
    GraphicsImage *close = assets->getImage_ColorKey("c_close_up");
    bool lit = Recti(pos, close->size()).contains(mouse->position());
    render->DrawQuad2D(lit ? assets->getImage_ColorKey("c_close_ht") : close, pos);
}

GUIButton *GUIWindow_Book::createMm8Ribbon(int ribbon, Pointi pos, BookButtonAction action, InputAction inputAction, std::string_view hint) {
    GraphicsImage *image = assets->getImage_ColorKey(fmt::format("irt{:02}r", ribbon));
    return CreateButton(pos, image->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ClickBooksBtn, std::to_underlying(action), inputAction, hint);
}

void GUIWindow_Book::drawMm8Ribbon(int ribbon, Pointi pos, BookButtonAction action) {
    bool pressed = _bookButtonClicked && _bookButtonAction == action;
    render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("irt{:02}{}", ribbon, pressed ? 'b' : 'r')), pos);
}

Recti GUIWindow_Book::mm8TitleRect() {
    return Recti(100, 60, 360, 40);
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
