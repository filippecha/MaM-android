#pragma once

#include <memory>
#include <string>

#include "GUI/GUIWindow.h"

class GraphicsImage;

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
