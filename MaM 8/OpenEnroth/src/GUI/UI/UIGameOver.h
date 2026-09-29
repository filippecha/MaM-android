#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    /**
     * @param releaseEvent              Message sent when the window closes.
     * @param lost                      MM6 only, the ending where the reactor destroys the world.
     */
    explicit GUIWindow_GameOver(UIMessageType releaseEvent = UIMSG_OnGameOverWindowClose, bool lost = false);
    virtual ~GUIWindow_GameOver();

    virtual void Update() override;

    bool toggleAndTestFinished();

 protected:
    UIMessageType _releaseEvent = UIMSG_0;
    bool _showPopUp = false;
    GraphicsImage *_winnerCert = nullptr;
};
