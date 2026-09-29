#include "GUI/UI/UIMainMenu.h"

#include <array>

#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

/**
 * @param index                         0 for new game, 1 for load, 2 for credits, 3 for quit.
 * @return                              Position of the button painted into MM6 or MM8 title.pcx.
 */
static Pointi paintedButtonPosition(int index) {
    return isMm6() ? Pointi(484, 10 + 62 * index) : Pointi(515, 203 + 38 * index);
}

GUIWindow_MainMenu::GUIWindow_MainMenu() :
    GUIWindow(WINDOW_MainMenu, {0, 0}, render->GetRenderDimensions()) {
    main_menu_background = assets->getImage_PCXFromIconsLOD("title.pcx");

    if (isMm8()) {
        // MM8 draws localized buttons over the ones painted into title.pcx, "up" and "ht" when hovered.
        static constexpr std::array<const char *, 4> names = {"t_new", "t_load", "t_cred", "t_quit"};
        for (int i = 0; i < 4; i++) {
            _mm8Buttons[i][0] = assets->getImage_ColorKey(fmt::format("{}_up", names[i]));
            _mm8Buttons[i][1] = assets->getImage_ColorKey(fmt::format("{}_ht", names[i]));
        }
    }

    if (isMm6() || isMm8()) {
        // MM6 has the buttons painted into title.pcx and no highlight images.
        Sizei size = isMm6() ? Sizei(130, 42) : Sizei(106, 32);
        pBtnNew = CreateButton("MainMenu_NewGame", paintedButtonPosition(0), size, BUTTON_TYPE_NORMAL, 0,
                               UIMSG_MainMenu_ShowPartyCreationWnd, 0, INPUT_ACTION_NEW_GAME, "");
        pBtnLoad = CreateButton("MainMenu_LoadGame", paintedButtonPosition(1), size, BUTTON_TYPE_NORMAL, 0,
                                UIMSG_MainMenu_ShowLoadWindow, 1, INPUT_ACTION_LOAD_GAME, "");
        pBtnCredits = CreateButton("MainMenu_Credits", paintedButtonPosition(2), size, BUTTON_TYPE_NORMAL, 0,
                                   UIMSG_ShowCredits, 2, INPUT_ACTION_SHOW_CREDITS, "");
        pBtnExit = CreateButton("MainMenu_ExitGame", paintedButtonPosition(3), size, BUTTON_TYPE_NORMAL, 0,
                                UIMSG_ExitToWindows, 3, INPUT_ACTION_EXIT_GAME, "");
        return;
    }

    ui_mainmenu_new = assets->getImage_ColorKey("title_new");
    ui_mainmenu_load = assets->getImage_ColorKey("title_load");
    ui_mainmenu_credits = assets->getImage_ColorKey("title_cred");
    ui_mainmenu_exit = assets->getImage_ColorKey("title_exit");

    pBtnNew = CreateButton("MainMenu_NewGame", {495, 172}, ui_mainmenu_new->size(), BUTTON_TYPE_NORMAL, 0,
                           UIMSG_MainMenu_ShowPartyCreationWnd, 0, INPUT_ACTION_NEW_GAME, "", {ui_mainmenu_new});
    pBtnLoad = CreateButton("MainMenu_LoadGame", {495, 227}, ui_mainmenu_load->size(), BUTTON_TYPE_NORMAL, 0,
                            UIMSG_MainMenu_ShowLoadWindow, 1, INPUT_ACTION_LOAD_GAME, "", {ui_mainmenu_load});
    pBtnCredits = CreateButton("MainMenu_Credits", {495, 282}, ui_mainmenu_credits->size(), BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ShowCredits, 2, INPUT_ACTION_SHOW_CREDITS, "", {ui_mainmenu_credits});
    pBtnExit = CreateButton("MainMenu_ExitGame", {495, 337}, ui_mainmenu_exit->size(), BUTTON_TYPE_NORMAL, 0,
                            UIMSG_ExitToWindows, 3, INPUT_ACTION_EXIT_GAME, "", {ui_mainmenu_exit});
}

GUIWindow_MainMenu::~GUIWindow_MainMenu() {
    for (GraphicsImage *image : {ui_mainmenu_new, ui_mainmenu_load, ui_mainmenu_credits, ui_mainmenu_exit})
        if (image)
            image->release();
    for (auto &images : _mm8Buttons)
        for (GraphicsImage *image : images)
            if (image)
                image->release();
    main_menu_background->release();
}

void GUIWindow_MainMenu::Update() {
    render->DrawQuad2D(main_menu_background, {0, 0});
    if (isMm8()) {
        Pointi mousePos = mouse->position();
        for (int i = 0; i < 4; i++) {
            bool hovered = vButtons[i]->Contains(mousePos.x, mousePos.y);
            render->DrawQuad2D(_mm8Buttons[i][hovered ? 1 : 0], paintedButtonPosition(i));
        }
        return;
    }
    if (isMm6())
        return;

    Pointi pt = mouse->position();

    GraphicsImage *pTexture = nullptr;
    for (GUIButton *pButton : vButtons) {
        if (pButton->Contains(pt.x, pt.y)) {
            auto pControlParam = pButton->msg_param;
            int pY = 0;
            switch (pControlParam) {  // backlight for buttons
                case 0:
                    pTexture = assets->getImage_ColorKey("title_new");
                    pY = 172;
                    break;
                case 1:
                    pTexture = assets->getImage_ColorKey("title_load");
                    pY = 227;
                    break;
                case 2:
                    pTexture = assets->getImage_ColorKey("title_cred");
                    pY = 282;
                    break;
                case 3:
                    pTexture = assets->getImage_ColorKey("title_exit");
                    pY = 337;
                    break;
            }
            render->DrawQuad2D(pTexture, {495, pY});
        }
    }
}

void GUIWindow_MainMenu::processMessage(UIMessageType message) {
    if (isMm6() || isMm8()) {
        switch (message) {
        case UIMSG_MainMenu_ShowPartyCreationWnd:
        case UIMSG_MainMenu_ShowLoadWindow:
        case UIMSG_ShowCredits:
        case UIMSG_ExitToWindows:
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            break;
        default:
            break;
        }
        return;
    }

    // Play the sound and change visual connected to the related button
    switch (message) {
    case UIMSG_MainMenu_ShowPartyCreationWnd:
        new OnButtonClick({ 495, 172 }, { 0, 0 }, pBtnNew);
        break;
    case UIMSG_MainMenu_ShowLoadWindow:
        new OnButtonClick({ 495, 227 }, { 0, 0 }, pBtnLoad);
        break;
    case UIMSG_ShowCredits:
        new OnButtonClick({ 495, 282 }, { 0, 0 }, pBtnCredits);
        break;
    case UIMSG_ExitToWindows:
        new OnButtonClick({ 495, 337 }, { 0, 0 }, pBtnExit);
        break;
    default:
        break;
    }
}
