#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Chest : public GUIWindow {
 public:
    explicit GUIWindow_Chest(int chestId);
    virtual ~GUIWindow_Chest();

    virtual void Update() override;

    int chestId() const {
        return _chestId;
    }

 private:
    int _chestId = 0;
    GUIButton *_takeAllButton = nullptr;
    GraphicsImage *_takeAllTexture = nullptr;  // The exit button's plaque with its baked-in caption tiled over.
};
