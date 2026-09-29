#pragma once

#include <memory>
#include <string>

#include "GUI/GUIWindow.h"

class GraphicsImage;
class GUIButton;

/**
 * Books of the MM8 book screen, in the order of their icons on its right edge.
 */
enum class Mm8BookPage {
    MM8_BOOK_MAP = 0,
    MM8_BOOK_QUESTS = 1,
    MM8_BOOK_HISTORY = 2,
    MM8_BOOK_NOTES = 3,
};
using enum Mm8BookPage;

/**
 * The book the MM8 scroll button on the top bar opens, the one looked at last.
 */
extern Mm8BookPage mm8LastBookPage;

enum class BookButtonAction {
    BOOK_ZOOM_IN = 0,
    BOOK_ZOOM_OUT = 1,
    BOOK_SCROLL_UP = 2,
    BOOK_SCROLL_DOWN = 3,
    BOOK_SCROLL_RIGHT = 4,
    BOOK_SCROLL_LEFT = 5,
    BOOK_NOTES_POTION = 6,
    BOOK_NOTES_FOUNTAIN = 7,
    BOOK_NOTES_OBELISK = 8,
    BOOK_NOTES_SEER = 9,
    BOOK_NOTES_MISC = 10,
    BOOK_NOTES_INSTRUCTORS = 11,
    BOOK_NEXT_PAGE = 12,
    BOOK_PREV_PAGE = 13,

    BOOK_BUTTON_FIRST = BOOK_ZOOM_IN,
    BOOK_BUTTON_LAST = BOOK_PREV_PAGE
};
using enum BookButtonAction;

class GUIWindow_Book : public GUIWindow {
 public:
    GUIWindow_Book();
    virtual ~GUIWindow_Book();

    void bookButtonClicked(BookButtonAction action);

 protected:
    static constexpr Pointi MM6_PAGE_POS = {47, 22};

    /**
     * Draws the book. MM6 draws its open book with `page` laid on it, MM7 books are a single picture.
     *
     * @param page                      MM7 book picture or MM6 page picture, can be null in MM6.
     * @param mm6PagePos                Where MM6 puts the page.
     */
    void drawBackground(GraphicsImage *page, Pointi mm6PagePos = MM6_PAGE_POS);
    void drawExitButton();

    /**
     * @param mm7Pos                    Position of a button on the right edge of the book in MM7, relative to the
     *                                  viewport.
     * @return                          Position of the same button in MM6. MM6 lines its tabs up differently.
     */
    static Pointi buttonPos(Pointi mm7Pos);

    /**
     * @return                          Rectangle that book titles are centered in, the title goes 22 pixels below
     *                                  its top.
     */
    static Recti titleRect();

    // MM8 book layout, MM8.exe 0x4CE59D and the pages after it.
    static constexpr Pointi MM8_CLOSE_POS = {555, 445};
    static constexpr Pointi MM8_PREV_PAGE_POS = {483, 39};
    static constexpr Pointi MM8_NEXT_PAGE_POS = {483, 91};
    static constexpr int MM8_TEXT_X = 90;
    static constexpr int MM8_TEXT_WIDTH = 360;
    static constexpr int MM8_DIV_BAR_X = 150;

    /**
     * @return                          Rectangle that MM8 book titles are centered in.
     */
    static Recti mm8TitleRect();

    /**
     * Creates the MM8 close button and the icons on the right that switch to the other books.
     */
    void createMm8Frame(Mm8BookPage page);

    void createMm8CloseButton(Pointi pos = MM8_CLOSE_POS);
    void drawMm8CloseButton(Pointi pos = MM8_CLOSE_POS);

    /**
     * Draws the MM8 scroll with the decoration of `page` in its top left corner, the book icons and the close button.
     */
    void drawMm8Frame(Mm8BookPage page);

    /**
     * @param ribbon                    Number of the MM8 ribbon picture, `irt01r` is 1.
     */
    GUIButton *createMm8Ribbon(int ribbon, Pointi pos, BookButtonAction action, InputAction inputAction = INPUT_ACTION_INVALID,
                               std::string_view hint = {});
    void drawMm8Ribbon(int ribbon, Pointi pos, BookButtonAction action);


    GraphicsImage *_mm6Book = nullptr;
    GraphicsImage *_mm6Exit = nullptr;

    std::unique_ptr<GUIWindow> pChildBooksOverlay;

    GraphicsImage *ui_book_button8_off{ nullptr };
    GraphicsImage *ui_book_button8_on{ nullptr };
    GraphicsImage *ui_book_button7_off{ nullptr };
    GraphicsImage *ui_book_button7_on{ nullptr };
    GraphicsImage *ui_book_button6_off{ nullptr };
    GraphicsImage *ui_book_button6_on{ nullptr };
    GraphicsImage *ui_book_button5_off{ nullptr };
    GraphicsImage *ui_book_button5_on{ nullptr };
    GraphicsImage *ui_book_button4_off{ nullptr };
    GraphicsImage *ui_book_button4_on{ nullptr };
    GraphicsImage *ui_book_button3_off{ nullptr };
    GraphicsImage *ui_book_button3_on{ nullptr };
    GraphicsImage *ui_book_button2_off{ nullptr };
    GraphicsImage *ui_book_button2_on{ nullptr };
    GraphicsImage *ui_book_button1_off{ nullptr };
    GraphicsImage *ui_book_button1_on{ nullptr };

    GraphicsImage *ui_book_map_frame{ nullptr };
    GraphicsImage *ui_book_quest_div_bar{ nullptr };

    static constexpr int BOOK_BUTTON_PRESSED_FRAMES = 10; // How long a clicked book button stays drawn pressed.

    int _bookButtonClicked = 0; // Frames left to draw the pressed button, counts down from BOOK_BUTTON_PRESSED_FRAMES.
    BookButtonAction _bookButtonAction = BOOK_ZOOM_IN;

 private:
    /**
     * @offset 0x411AAA
     */
    void initializeFonts();
};


class GUIWindow_BooksButtonOverlay : public GUIWindow {
 public:
    GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {});

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};
