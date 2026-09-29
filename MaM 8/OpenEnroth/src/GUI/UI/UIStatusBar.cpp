#include "UIStatusBar.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"

#include "Engine/Graphics/Renderer/Renderer.h"

#include "GUI/GUIFont.h"

#include "GUI/UI/UIGame.h"

#include "Utility/GameVariant.h"

const std::string &StatusBar::get() {
    if (_eventStatusExpireTime) {
        return _eventStatusString;
    } else {
        return _statusString;
    }
}

void StatusBar::draw() {
    if (isMm8()) {
        const std::string &status = get();
        if (status.empty() || current_screen_type == SCREEN_SAVEGAME || current_screen_type == SCREEN_LOADGAME ||
            current_screen_type == SCREEN_OPTIONS || current_screen_type == SCREEN_VIDEO_OPTIONS)
            return; // These menus have no status line.
        if (current_screen_type == SCREEN_SPELL_BOOK || current_screen_type == SCREEN_MENU) { // These cover the bottom bar.
            int y = current_screen_type == SCREEN_MENU ? 444 : 458; // The game menu has a line of its own.
            GUIWindow::DrawText(assets->pFontLucida.get(), {assets->pFontLucida->AlignText_Center(640, status), y}, uGameUIFontMain, status, pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
            return;
        }
        GUIWindow::DrawText(assets->pFontLucida.get(), {assets->pFontLucida->AlignText_Center(468, status), 370}, uGameUIFontMain, status, pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
        return;
    }
    render->DrawQuad2D(game_ui_statusbar, {0, 352});

    const std::string &status = get();
    if (status.length() > 0) {
        GUIWindow::DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, status) + 11, 357}, uGameUIFontMain, status, pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
    }
}

void StatusBar::drawForced(std::string_view str, Color color) {
    if (isMm8()) {
        GUIWindow::DrawText(assets->pFontLucida.get(), {assets->pFontLucida->AlignText_Center(468, str), 370}, color, str, pPrimaryWindow->frameRect);
        return;
    }
    render->DrawQuad2D(game_ui_statusbar, {0, 352});
    GUIWindow::DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, str) + 11, 357}, color, str, pPrimaryWindow->frameRect);
}

void StatusBar::update() {
    // Was also checking that event timer is not stopped
    if (_eventStatusExpireTime && platform->tickCount() >= _eventStatusExpireTime) {
        _eventStatusExpireTime = 0;
    }
}

void StatusBar::setPermanent(std::string_view str) {
    if (str.length() > 0) {
        if (_eventStatusExpireTime == 0) {
            _statusString = str;
        }
    }
}

void StatusBar::clearPermanent() {
    _statusString.clear();
}

void StatusBar::clearAll() {
    _statusString.clear();
    clearEvent();
}

void StatusBar::setEvent(std::string_view str) {
    _eventStatusString = str;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION;
}

void StatusBar::setEventShort(std::string_view str) {
    _eventStatusString = str;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION_SHORT;
}

void StatusBar::clearEvent() {
    _eventStatusExpireTime = 0;
}

void StatusBar::nothingHere() {
    if (_eventStatusExpireTime == 0) {
        setEvent(LSTR_NOTHING_HERE);
    }
}
