#pragma once

#include "Engine/Pid.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "GUI/UI/UIBooks.h"

class GUIWindow_TownPortalBook : public GUIWindow_Book {
 public:
    explicit GUIWindow_TownPortalBook(Pid casterPid, SpellCastFlags castFlags);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update() override;

    void clickTown(int townId);
    void hintTown(int townId);

    /**
     * @return                          MM6 town of the last house the party entered, where town portal below master
     *                                  goes. New Sorpigal if there is none.
     */
    static int mm6LastTown();

    /**
     * Remembers the current map as the MM6 town portal destination if it is one of the towns, MM6.exe 0x43C3D8.
     */
    static void mm6RememberTown();

 private:
    const Pid _casterPid;
    const SpellCastFlags _castFlags;
};
