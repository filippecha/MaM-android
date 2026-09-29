#pragma once

#include <vector>
#include <unordered_map>

#include "Engine/Data/AutonoteEnums.h"

#include "GUI/UI/UIBooks.h"

struct GUIWindow_AutonotesBook : public GUIWindow_Book {
    GUIWindow_AutonotesBook();
    virtual ~GUIWindow_AutonotesBook() {}

    virtual void Update() override;

 protected:
    void recalculateCurrentNotesTypePages();

 private:
    void updateMm8();

    /**
     * Turns the pages and draws the notes of the page.
     *
     * @param autonotes_frameRect       Where the notes go, its height is the bottom of the text.
     * @param divBarX                   Where the bars between the notes go.
     */
    void drawNotes(Recti autonotes_frameRect, int divBarX);

    int _startingNotesIdx = 0;
    int _currentPage = 0;
    int _currentPageNotes = 0;
    std::vector<int> _activeNotesIdx;
    std::unordered_map<int, int> _notesPerPage;
};

extern AutonoteType autonoteBookDisplayType;
