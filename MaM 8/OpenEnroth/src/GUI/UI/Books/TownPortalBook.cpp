#include "TownPortalBook.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/PartyPlacement.h"
#include "Engine/Pid.h"
#include "Engine/SaveLoad.h"
#include "Engine/Objects/Actor.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/MapEnumFunctions.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIGame.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/Mouse.h"

#include "Library/Geometry/Rect.h"

#include "Utility/GameVariant.h"

struct TownPortalData {
    Vec3f pos;
    int viewYaw;
    int viewPitch;
    MapId mapInfoID;
    QuestBit qBit;
};

static const int TOWN_PORTAL_DESTINATION_COUNT = 6;
static const int TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS = 6 + 14;

std::array<TownPortalData, TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS> townPortalList = {{
    {Vec3f( -5121,   2107,    1), 1536, 0, MAP_CASTLE_HARMONDALE,    QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED},
    {Vec3f(-15148, -10240, 1473),    0, 0, MAP_TULAREAN_FOREST,      QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED},
    {Vec3f(-10519,   5375,  753),  512, 0, MAP_ERATHIA,              QBIT_FOUNTAIN_IN_STEADWICK_ACTIVATED},
    {Vec3f(  3114, -11055,  513),    0, 0, MAP_MOUNT_NIGHON,         QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED},
    {Vec3f(  -158,   7624,    1),  512, 0, MAP_CELESTE,              QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED},
    {Vec3f( -1837,  -4247,   65),   65, 0, MAP_PIT,                  QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED},
    // cheats
    {Vec3f( 14500,  20300,   450), 1800, 0, MAP_AVLEE,               QBIT_INVALID},
    {Vec3f( -2500,   3000,  2100), 1024, 0, MAP_BARROW_DOWNS,        QBIT_INVALID},
    {Vec3f( 10000,  18000,    50), 1024, 0, MAP_BRACADA_DESERT,      QBIT_INVALID},
    {Vec3f(-17000,  18000,    50),    0, 0, MAP_DEYJA,               QBIT_INVALID},
    {Vec3f( 12000, -10000,   400), 1024, 0, MAP_TATALIA,             QBIT_INVALID}, // near swamps
    //{Vec3f(16000, 15000,  3100),  300, 0, MAP_TATALIA,             QBIT_INVALID}, // alternative location - near mercenary guild
    {Vec3f( -5000,     25,   600),    0, 0, MAP_EVENMORN_ISLAND,     QBIT_INVALID}, // in a center of map :)
    //{Vec3f( -4868,   25,   100),    0, 0, MAP_EVENMORN_ISLAND,     QBIT_INVALID}, // alternative location - near ship
    {Vec3f( 10600,   9900,   100),    0, 0, MAP_EMERALD_ISLAND,      QBIT_INVALID},
    {Vec3f( 15000,   4500,  1000), 1400, 0, MAP_LAND_OF_THE_GIANTS,  QBIT_INVALID},
    {Vec3f(     0,  16000,  2500), 1000, 0, MAP_SHOALS,              QBIT_INVALID},
    {Vec3f( -2500,   3400,    50),    0, 0, MAP_CELESTE,             QBIT_INVALID},
    {Vec3f( -3642,  10830, -1550),    0, 0, MAP_CASTLE_NAVAN,        QBIT_INVALID},
    {Vec3f( -5640,   -253,   550),  600, 0, MAP_CASTLE_GRYPHONHEART, QBIT_INVALID},
    {Vec3f( -3492,  12500,  1121),  512, 0, MAP_CASTLE_LAMBENT,      QBIT_INVALID},
    {Vec3f( -9916, -18482, -2800), 1024, 0, MAP_CASTLE_GLOAMING,     QBIT_INVALID},
}};

// MM6 towns: party placement from MM6.exe 0x4C1F98, map from mapstats.txt and the button on the townport picture from
// 0x4BCAE4. Listed in the order of the buttons.
static const std::array<TownPortalData, 6> mm6TownPortalList = {{
    {Vec3f( -9705,  -6858, 161), 1536, 0, mapIdFromMm6(15), QBIT_INVALID}, // New Sorpigal.
    {Vec3f(  3489, -14582, 257),    0, 0, mapIdFromMm6(14), QBIT_INVALID}, // Mist.
    {Vec3f( 13146,  -9194,   1),    0, 0, mapIdFromMm6(10), QBIT_INVALID}, // Silver Cove.
    {Vec3f(  6991,  13438,  97),    0, 0, mapIdFromMm6(8),  QBIT_INVALID}, // Free Haven.
    {Vec3f(-15079,  12878, 161), 1536, 0, mapIdFromMm6(5),  QBIT_INVALID}, // Blackshire.
    {Vec3f( -9138,  14518,  97),    0, 0, mapIdFromMm6(7),  QBIT_INVALID}, // White Cap.
}};

// MM8 towns: party placement and map from MM8.exe 0x4FEA98, the qbits from 0x431BA4, the highlights and their places
// from 0x4D3DB1. Listed in the order of the highlights.
struct Mm8Town {
    TownPortalData data;
    const char *highlight;
    Pointi pos;
};
static const std::array<Mm8Town, 6> mm8Towns = {{
    {{Vec3f(   -8,  15447,   448), 512, 0, mapIdFromMm8(3),  static_cast<QuestBit>(181)}, "tphell",    {269,  36}}, // Alvar.
    {{Vec3f(10296, -12283,     1),   0, 0, mapIdFromMm8(2),  static_cast<QuestBit>(180)}, "tpheaven",  {274, 174}}, // Ravenshore.
    {{Vec3f(-1632,  -1630, -2239), 776, 0, mapIdFromMm8(35), static_cast<QuestBit>(184)}, "tpisland",  { 14, 298}}, // Balthazar Lair.
    {{Vec3f( 2579,  -1597,   384), 512, 0, mapIdFromMm8(13), static_cast<QuestBit>(183)}, "tpwarlock", {204, 348}}, // Regna.
    {{Vec3f(-7088, -15860,    97),   0, 0, mapIdFromMm8(6),  static_cast<QuestBit>(182)}, "tpelf",     {444, 127}}, // Shadowspire.
    {{Vec3f( 7924,   5335,   737), 897, 0, mapIdFromMm8(1),  static_cast<QuestBit>(185)}, "tpharmndy", {460, 303}}, // Dagger Wound.
}};
static constexpr Pointi MM8_TOWN_PORTAL_CLOSE_POS = {553, 448};

static constexpr std::array<Recti, 6> mm6TownPortalButtonsPos = {{
    {346, 280, 62, 31}, {360, 186, 46, 42}, {318, 121, 52, 26}, {223, 156, 51, 30}, {113, 150, 51, 33}, {192, 81, 54, 30}
}};

static const TownPortalData &townPortalData(int townId) {
    if (isMm8())
        return mm8Towns[townId].data;
    return isMm6() ? mm6TownPortalList[townId] : townPortalList[townId];
}

static bool isTownUnlocked(int townId) {
    return isMm6() || engine->config->debug.TownPortal.value() || pParty->_questBits[townPortalData(townId).qBit];
}

static std::array<Recti, TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS> townPortalButtonsPos = {{
    {260, 206, 80, 55}, // Harmondale
    {324,  84, 66, 56}, // Tularean forest
    {147, 182, 68, 65}, // Erathia
    {385, 239, 72, 67}, // Nighon
    {390,  17, 67, 67}, // Celeste
    { 19, 283, 74, 59}, // Pit

    {220,  17, 39, 36}, // Avlee, above Tularean forest
    {260, 283, 39, 36}, // Barrow downs, below Harmondale
    {147, 262, 39, 36}, // Bracada, below Erathia
    {224, 104, 39, 36}, // Dejya, to the left of Tularean forest
    { 67, 182, 39, 36}, // Tatalia, to the left of Erathia
    { 10, 182, 39, 36}, // Evenmourn island, to the left of Tatalia
    { 19, 123, 39, 36}, // Emerald island, above Pit
    {355, 159, 39, 36}, // Eofol above of Nighon
    {150,  17, 39, 36}, // Shoals, to the NW of Avlee
    {340,  17, 39, 36}, // Celeste training
    {284,  84, 39, 36}, // Navan throne room
    {147, 142, 39, 36}, // Gryphonheart throne room
    {400,  87, 39, 36}, // Lambent throne room
    { 19, 243, 39, 36}, // Gloaming throne room
}};

static std::array<GraphicsImage *, TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS> ui_book_townportal_icons;

GraphicsImage *ui_book_townportal_background = nullptr;
GraphicsImage *ui_townportal_cheat_destination_icon = nullptr;

GUIWindow_TownPortalBook::GUIWindow_TownPortalBook(Pid casterPid, SpellCastFlags castFlags)
        : _casterPid(casterPid), _castFlags(castFlags) {
    this->eWindowType = WindowType::WINDOW_TownPortal;

    ui_book_townportal_background = assets->getImage_Solid("townport");

    if (isMm8()) {
        createMm8CloseButton(MM8_TOWN_PORTAL_CLOSE_POS);
        for (int i = 0; i < mm8Towns.size(); ++i)
            CreateButton(fmt::format("TownPortalBook_Marker{}", i), mm8Towns[i].pos, assets->getImage_ColorKey(mm8Towns[i].highlight)->size(),
                         BUTTON_TYPE_NORMAL, UIMSG_HintTownPortal, UIMSG_ClickTownInTP, i);
        return;
    }

    ui_book_townportal_icons[0] = assets->getImage_ColorKey("tpharmndy");
    ui_book_townportal_icons[1] = assets->getImage_ColorKey("tpelf");
    ui_book_townportal_icons[2] = assets->getImage_ColorKey("tpwarlock");
    ui_book_townportal_icons[3] = assets->getImage_ColorKey("tpisland");
    ui_book_townportal_icons[4] = assets->getImage_ColorKey("tpheaven");
    ui_book_townportal_icons[5] = assets->getImage_ColorKey("tphell");

    // cheat locations
    ui_townportal_cheat_destination_icon = assets->getImage_ColorKey("tab-an-2a");
    for (int i = TOWN_PORTAL_DESTINATION_COUNT; i < TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS; ++i) {
        ui_book_townportal_icons[i] = assets->getImage_ColorKey("tab-an-2b");
    }

    int count = TOWN_PORTAL_DESTINATION_COUNT;
    if (engine->config->debug.TownPortal.value()) {
        count = TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS;
    }
    if (isMm6()) {
        for (int i = 0; i < mm6TownPortalButtonsPos.size(); ++i) {
            Recti rect = mm6TownPortalButtonsPos[i];
            CreateButton(fmt::format("TownPortalBook_Marker{}", i), rect.topLeft(), rect.size(), BUTTON_TYPE_NORMAL, UIMSG_HintTownPortal, UIMSG_ClickTownInTP, i);
        }
        return;
    }
    for (int i = 0; i < count; ++i) {
        CreateButton(fmt::format("TownPortalBook_Marker{}", i), townPortalButtonsPos[i].topLeft(), townPortalButtonsPos[i].size(), BUTTON_TYPE_NORMAL, UIMSG_HintTownPortal, UIMSG_ClickTownInTP, i);
    }
}

int GUIWindow_TownPortalBook::mm6LastTown() {
    for (int i = 0; i < mm6TownPortalList.size(); i++)
        if (mm6TownPortalList[i].mapInfoID == pParty->mm6LastTown)
            return i;
    return 0;
}

void GUIWindow_TownPortalBook::mm6RememberTown() {
    for (const TownPortalData &town : mm6TownPortalList)
        if (town.mapInfoID == engine->_currentLoadedMapId)
            pParty->mm6LastTown = town.mapInfoID;
}

void GUIWindow_TownPortalBook::Update() {
    Pointi cursorPos = mouse->position();
    if (isMm8()) {
        // The map covers the screen, the towns light up under the mouse once they are open.
        render->DrawQuad2D(ui_book_townportal_background, {0, 0});
        for (int i = 0; i < mm8Towns.size(); ++i) {
            GraphicsImage *highlight = assets->getImage_ColorKey(mm8Towns[i].highlight);
            if (isTownUnlocked(i) && Recti(mm8Towns[i].pos, highlight->size()).contains(cursorPos))
                render->DrawQuad2D(highlight, mm8Towns[i].pos);
        }
        drawMm8CloseButton(MM8_TOWN_PORTAL_CLOSE_POS);
        return;
    }

    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    bool townPortalCheats = engine->config->debug.TownPortal.value();
    int count = townPortalCheats ? TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS : TOWN_PORTAL_DESTINATION_COUNT;

    render->DrawQuad2D(ui_book_townportal_background, {8, 8});
    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    if (townPortalCheats) {
        // draw grey icons for cheat locations
        // ordinary locations are in the background image already
        for (int i = TOWN_PORTAL_DESTINATION_COUNT; i < TOWN_PORTAL_DESTINATION_COUNT_WITH_CHEATS; ++i) {
            render->DrawQuad2D(ui_townportal_cheat_destination_icon, townPortalButtonsPos[i].topLeft());
        }
    }

    if (isMm6()) { // MM6 has no highlight pictures, the towns are on the map.
        DrawTitleText(assets->pFontBookTitle.get(), 0, 22, colorTable.White, localization->str(LSTR_TOWN_PORTAL), 3, pViewport);
        return;
    }

    // highlight the covered location
    for (int i = 0; i < count; ++i) {
        if (townPortalCheats || pParty->_questBits[townPortalList[i].qBit]) {
            if (townPortalButtonsPos[i].contains(cursorPos)) {
                render->DrawQuad2D(ui_book_townportal_icons[i], townPortalButtonsPos[i].topLeft());
            }
        }
    }

    DrawTitleText(assets->pFontBookTitle.get(), 0, 22, colorTable.White, localization->str(LSTR_TOWN_PORTAL), 3, pViewport);
}

void GUIWindow_TownPortalBook::clickTown(int townId) {
    // check if tp location is unlocked
    if (!isTownUnlocked(townId)) {
        return;
    }
    const TownPortalData &town = townPortalData(townId);

    // begin TP
    autoSave();
    // if in current map
    // TODO(Nik-RE-dev): need separate function for teleportation to other maps
    if (engine->_currentLoadedMapId == town.mapInfoID) {
        pParty->pos = town.pos;
        pParty->uFallStartZ = pParty->pos.z;
        pParty->_viewYaw = town.viewYaw;
        pParty->_viewPitch = town.viewPitch;
    } else {  // if change map
        onMapLeave();
        dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
        uGameState = GAME_STATE_CHANGE_LOCATION;
        engine->_pendingTransition = MapDestination(town.mapInfoID,
                                                    PartyPlacement(town.pos, town.viewYaw, town.viewPitch, 0));
        Actor::InitializeActors();
    }

    assert(_casterPid.type() == OBJECT_Character);

    int casterId = _casterPid.id();
    if (casterId < pParty->pCharacters.size()) {
        // Town portal cast by character
        Mastery mastery;
        Character &character = pParty->pCharacters[casterId];
        if (engine->config->debug.AllMagic.value()) {
            mastery = MASTERY_GRANDMASTER;
        } else if (_castFlags & ON_CAST_CastViaScroll) {
            // Cast from scroll
            mastery = SCROLL_OR_NPC_SPELL_SKILL_VALUE.mastery();
        } else {
            mastery = character.getActualSkillValue(SKILL_WATER).mastery();
            character.SpendMana(spellManaCost(SPELL_WATER_TOWN_PORTAL, mastery));
        }

        Duration sRecoveryTime = pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[mastery];
        if (pParty->bTurnBasedModeOn) {
            pParty->pTurnBasedCharacterRecoveryTimes[casterId] = sRecoveryTime;
            character.SetRecoveryTime(sRecoveryTime);
            pTurnEngine->ApplyPlayerAction();
        } else {
            character.SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * sRecoveryTime);
        }
    } else {
        // Town portal cast by hireling
        pParty->pHirelings[casterId - pParty->pCharacters.size()].hasUsedAbility = 1;
    }

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_TownPortalBook::hintTown(int townId) {
    if (!isTownUnlocked(townId)) {
        render->DrawQuad2D(game_ui_statusbar, {0, 352}); // TODO(captainurist): engine->_statusBar->smthSmth()???
        return;
    }

    engine->_statusBar->setPermanent(LSTR_TOWN_PORTAL_TO_S, pMapTable->pInfos[townPortalData(townId).mapInfoID].name);
}
