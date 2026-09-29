#pragma once

#include <array>
#include <string>

#include "GUI/UI/UIHouses.h"

class GraphicsImage;

/**
 * MM8 Adventurer's Inn, MM8.exe 0x4CAE6B. Lists the roster characters that were offered to join and aren't in the
 * party, and shows the selected one to be hired. Picking a party member shows that one to be dismissed.
 */
class GUIWindow_AdventurersInn : public GUIWindow_House {
 public:
    explicit GUIWindow_AdventurersInn(HouseId houseId);
    virtual ~GUIWindow_AdventurersInn() {}

    virtual void Update() override;
    virtual void houseScreenClick() override;

 private:
    void drawDetails(Character &character, const std::string &blurb) const;
    Character *selectedCharacter() const;

    int _scroll = 0; // First shown row of portraits.
    int _selected = -1; // Roster id of the shown inn character, -1 for none.
    bool _showingParty = false; // Whether the active party member is shown instead.
    int _lastActiveCharacter = -1;
    GraphicsImage *_background = nullptr;
    std::array<GraphicsImage *, 4> _highlight = {};
};
