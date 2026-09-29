#pragma once
#include <vector>

#include "GUI/UI/UIBooks.h"

struct GUIWindow_JournalBook : public GUIWindow_Book {
    GUIWindow_JournalBook();
    virtual ~GUIWindow_JournalBook() {}

    virtual void Update() override;

 private:
    /**
     * @return                          Where the text goes.
     */
    static Recti journalWindow();

    int _currentIdx = 0;
    std::vector<int> _journalIdx;
    std::vector<int> _journalEntryPage;
};
