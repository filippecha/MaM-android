#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/Timer.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIRest.h"

#include "Io/Mouse.h"

#include "Utility/GameVariant.h"

GraphicsImage *rest_ui_btn_4 = nullptr;
GraphicsImage *rest_ui_btn_exit = nullptr;
GraphicsImage *rest_ui_btn_3 = nullptr;
GraphicsImage *rest_ui_btn_1 = nullptr;
GraphicsImage *rest_ui_btn_2 = nullptr;
GraphicsImage *rest_ui_restmain = nullptr;

GraphicsImage *rest_ui_sky_frame_current = nullptr;
GraphicsImage *rest_ui_hourglass_frame_current = nullptr;

namespace {

/**
 * MM8 rest screen, MM8.exe 0x41F370. The button pictures carry their labels, "u" is the normal picture and "h" the
 * lit one.
 */
struct Mm8RestButton {
    const char *picture;
    Pointi position;
};

constexpr Mm8RestButton MM8_REST_HEAL = {"R8h", {60, 156}};
constexpr Mm8RestButton MM8_WAIT_TILL_DAWN = {"Rdawn", {99, 240}};
constexpr Mm8RestButton MM8_WAIT_1_HOUR = {"R1h", {99, 272}};
constexpr Mm8RestButton MM8_WAIT_5_MINUTES = {"R5m", {99, 304}};
constexpr Mm8RestButton MM8_EXIT = {"Rexit", {362, 300}};

GraphicsImage *mm8ButtonPicture(const Mm8RestButton &button, bool lit) {
    return assets->getImage_ColorKey(fmt::format("{}{}", button.picture, lit ? "H" : "U"));
}

} // namespace

int foodRequiredToRest;
Duration remainingRestTime;
RestType currentRestType;

static void prepareToLoadRestUI() {
    if (current_screen_type != SCREEN_GAME) {
        pGUIWindow_CurrentMenu = nullptr;
        current_screen_type = SCREEN_GAME;
    }
    gameTimer->setPaused(true);
    if (currentRestType != REST_HEAL) {
        new OnButtonClick({518, 450}, {0, 0}, pBtn_Rest);
    }
    remainingRestTime = Duration();
    currentRestType = REST_NONE;
}

static void calculateRequiredFood() {
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        foodRequiredToRest = pOutdoor->getNumFoodRequiredToRestInCurrentPos(pParty->pos);
    } else {
        foodRequiredToRest = 2;
    }

    if (PartyHasDragon()) {
        ++foodRequiredToRest;
    }

    if (CheckHiredNPCSpeciality(Porter)) {
        --foodRequiredToRest;
    }
    if (CheckHiredNPCSpeciality(QuarterMaster)) {
        foodRequiredToRest -= 2;
    }
    if (CheckHiredNPCSpeciality(Gypsy)) {
        --foodRequiredToRest;
    }
    if (foodRequiredToRest < 1) {
        foodRequiredToRest = 1;
    }
    if (engine->_currentLoadedMapId == MAP_CASTLE_HARMONDALE && pParty->_questBits[QBIT_HARMONDALE_REBUILT]) {
        foodRequiredToRest = 0;
    }
}

GUIWindow_Rest::GUIWindow_Rest()
    : GUIWindow(WINDOW_Rest, {0, 0}, render->GetRenderDimensions()) {
    prepareToLoadRestUI();
    calculateRequiredFood();

    current_screen_type = SCREEN_REST;

    hourglassLoopTimer = 0_ticks;
    rest_ui_restmain = assets->getImage_Alpha("restmain");
    rest_ui_btn_1 = assets->getImage_Alpha("restb1");
    rest_ui_btn_2 = assets->getImage_Alpha("restb2");
    rest_ui_btn_3 = assets->getImage_Alpha("restb3");
    rest_ui_btn_4 = assets->getImage_Alpha("restb4");
    rest_ui_btn_exit = assets->getImage_Alpha("restexit");

    OutdoorLocation::LoadActualSkyFrame();

    if (isMm8()) {
        auto create = [&](const Mm8RestButton &button, UIMessageType message, InputAction action, std::string_view id) {
            GraphicsImage *picture = mm8ButtonPicture(button, false);
            return CreateButton(std::string(id), button.position, picture->size(), BUTTON_TYPE_NORMAL, 0, message, 0, action, "",
                                {mm8ButtonPicture(button, true)});
        };
        pButton_RestUI_Exit = create(MM8_EXIT, UIMSG_ExitRest, INPUT_ACTION_INVALID, "Rest_Exit");
        pButton_RestUI_Main = create(MM8_REST_HEAL, UIMSG_Rest8Hour, INPUT_ACTION_REST_HEAL, "Rest_RestAndHeal");
        pButton_RestUI_WaitUntilDawn = create(MM8_WAIT_TILL_DAWN, UIMSG_WaitTillDawn, INPUT_ACTION_REST_WAIT_TILL_DAWN, "Rest_WaitTillDawn");
        pButton_RestUI_Wait1Hour = create(MM8_WAIT_1_HOUR, UIMSG_Wait1Hour, INPUT_ACTION_REST_WAIT_1_HOUR, "Rest_Wait1Hour");
        pButton_RestUI_Wait5Minutes = create(MM8_WAIT_5_MINUTES, UIMSG_Wait5Minutes, INPUT_ACTION_REST_WAIT_5_MINUTES, "Rest_Wait5Minutes");
        return;
    }

    pButton_RestUI_Exit = CreateButton({280, 297}, {154, 37}, BUTTON_TYPE_NORMAL, 0, UIMSG_ExitRest, 0, INPUT_ACTION_INVALID, "", {rest_ui_btn_exit});
    pButton_RestUI_Main = CreateButton("Rest_RestAndHeal", {24, 154}, {225, 37}, BUTTON_TYPE_NORMAL, 0, UIMSG_Rest8Hour, 0, INPUT_ACTION_REST_HEAL, "", {rest_ui_btn_4});
    pButton_RestUI_WaitUntilDawn = CreateButton("Rest_WaitTillDawn", {61, 232}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_WaitTillDawn, 0, INPUT_ACTION_REST_WAIT_TILL_DAWN, "", {rest_ui_btn_1});
    pButton_RestUI_Wait1Hour = CreateButton("Rest_Wait1Hour", {61, 264}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait1Hour, 0, INPUT_ACTION_REST_WAIT_1_HOUR, "", {rest_ui_btn_2});
    pButton_RestUI_Wait5Minutes = CreateButton({61, 296}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait5Minutes, 0, INPUT_ACTION_REST_WAIT_5_MINUTES, "", {rest_ui_btn_3});
}

void GUIWindow_Rest::Update() {
    GUIButton tmp_button;

    int liveCharacters = 0;
    for (Character &player : pParty->pCharacters) {
        if (!player.IsDead() && !player.IsEradicated() && player.health > 0) {
            ++liveCharacters;
        }
    }

    if (liveCharacters && isMm8()) {
        updateMm8();
    } else if (liveCharacters) {
        render->DrawQuad2D(rest_ui_restmain, {8, 8});
        render->DrawQuad2D(rest_ui_sky_frame_current, {16, 26});
        if (rest_ui_hourglass_frame_current) {
            rest_ui_hourglass_frame_current->release();
            rest_ui_hourglass_frame_current = nullptr;
        }

        hourglassLoopTimer += gameTimer->dt();
        if (hourglassLoopTimer >= Duration::fromRealtimeSeconds(4)) {
            hourglassLoopTimer = 0_ticks;
        }

        int hourglass_icon_idx = (int)floorf(((double)hourglassLoopTimer.ticks() / 512.0 * 120.0) + 0.5f) % 256 + 1;
        if (hourglass_icon_idx >= 120) {
            hourglass_icon_idx = 1;
        }

        rest_ui_hourglass_frame_current = assets->getImage_ColorKey(fmt::format("hglas{:03}", hourglass_icon_idx));
        render->DrawQuad2D(rest_ui_hourglass_frame_current, {267, 159});

        tmp_button.rect = Recti(24, 154, 171, 37);
        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->str(LSTR_REST_HEAL_8_HOURS), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        tmp_button.pParent = 0;

        auto str1 = fmt::format("\r408{}", foodRequiredToRest);
        GUIWindow::DrawText(assets->pFontCreate.get(), {0, 164}, colorTable.Diesel, str1, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);

        pButton_RestUI_WaitUntilDawn->DrawLabel(localization->str(LSTR_WAIT_UNTIL_DAWN), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        pButton_RestUI_Wait1Hour->DrawLabel(localization->str(LSTR_WAIT_1_HOUR), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        pButton_RestUI_Wait5Minutes->DrawLabel(localization->str(LSTR_WAIT_5_MINUTES), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        pButton_RestUI_Exit->DrawLabel(localization->str(LSTR_EXIT_REST), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        tmp_button.rect = Recti(45, 199, 185, 30);

        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->str(LSTR_WAIT_WITHOUT_HEALING), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
        tmp_button.pParent = 0;

        CivilTime time = pParty->GetPlayingTime().toCivilTime();

        std::string str2 = fmt::format("{}:{:02} {}", time.hourAmPm, time.minute, localization->amPm(time.isPm));
        DrawText(assets->pFontCreate.get(), {368, 168}, colorTable.Diesel, str2, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str3 = fmt::format("{}\r190{}", localization->str(LSTR_DAY_CAPITALIZED), time.day);
        DrawText(assets->pFontCreate.get(), {350, 190}, colorTable.Diesel, str3, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str4 = fmt::format("{}\r190{}", localization->str(LSTR_MONTH), time.month);
        DrawText(assets->pFontCreate.get(), {350, 222}, colorTable.Diesel, str4, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str5 = fmt::format("{}\r190{}", localization->str(LSTR_YEAR), time.year);
        DrawText(assets->pFontCreate.get(), {350, 254}, colorTable.Diesel, str5, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        if (currentRestType != REST_NONE) {
            Party::restOneFrame();
        }
    } else {
        new OnCancel(pButton_RestUI_Exit->rect.topLeft(), {0, 0}, pButton_RestUI_Exit, localization->str(LSTR_EXIT_REST));
    }
}

void GUIWindow_Rest::updateMm8() {
    render->DrawQuad2D(assets->getImage_Solid("restmain"), {0, 0});
    render->DrawQuad2D(rest_ui_sky_frame_current, {96, 25});

    hourglassLoopTimer += gameTimer->dt();
    if (hourglassLoopTimer >= Duration::fromRealtimeSeconds(4))
        hourglassLoopTimer = 0_ticks;
    int hourglassFrame = (int)floorf(((double)hourglassLoopTimer.ticks() / 512.0 * 120.0) + 0.5f) % 256 + 1;
    if (hourglassFrame >= 120)
        hourglassFrame = 1;
    render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("hglas{:03}", hourglassFrame)), {384, 164});

    Pointi mousePos = mouse->position();
    for (const Mm8RestButton *button : {&MM8_REST_HEAL, &MM8_WAIT_TILL_DAWN, &MM8_WAIT_1_HOUR, &MM8_WAIT_5_MINUTES, &MM8_EXIT}) {
        GraphicsImage *picture = mm8ButtonPicture(*button, false);
        bool lit = Recti(button->position, picture->size()).contains(mousePos);
        render->DrawQuad2D(lit ? mm8ButtonPicture(*button, true) : picture, button->position);
    }

    GUIFont *font = assets->pFontCreate.get();
    Recti screen(0, 0, 640, 480);
    DrawText(font, {0, 168}, colorTable.White, fmt::format("\r315{}", foodRequiredToRest), screen); // Right aligned at x 315.
    CivilTime time = pParty->GetPlayingTime().toCivilTime();
    DrawText(font, {486, 168}, colorTable.White, fmt::format("{}:{:02} {}", time.hourAmPm, time.minute, localization->amPm(time.isPm)), screen);
    DrawText(font, {464, 190}, colorTable.White, fmt::format("{}\r078{}", localization->str(LSTR_DAY_CAPITALIZED), time.day), screen);
    DrawText(font, {464, 222}, colorTable.White, fmt::format("{}\r078{}", localization->str(LSTR_MONTH), time.month), screen);
    DrawText(font, {464, 254}, colorTable.White, fmt::format("{}\r078{}", localization->str(LSTR_YEAR), time.year), screen);

    if (currentRestType != REST_NONE)
        Party::restOneFrame();
}
