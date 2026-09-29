#include "GUI/UI/UISpellbook.h"

#include <algorithm>
#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Timer.h"
#include "Engine/Mm6ExeData.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/GameVariant.h"

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> spellbook_texture_filename_suffices = {
    {MAGIC_SCHOOL_FIRE,   "f"},
    {MAGIC_SCHOOL_AIR,    "a"},
    {MAGIC_SCHOOL_WATER,  "w"},
    {MAGIC_SCHOOL_EARTH,  "e"},
    {MAGIC_SCHOOL_SPIRIT, "s"},
    {MAGIC_SCHOOL_MIND,   "m"},
    {MAGIC_SCHOOL_BODY,   "b"},
    {MAGIC_SCHOOL_LIGHT,  "l"},
    {MAGIC_SCHOOL_DARK,   "d"}
};

static constexpr IndexedArray<std::array<unsigned char, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> pSpellbookSpellIndices = {{
    {MAGIC_SCHOOL_FIRE,     {0, 3, 1, 8, 11, 7, 4, 10, 6, 2, 5, 9}},
    {MAGIC_SCHOOL_AIR,      {0, 11, 2, 9, 6, 8, 5, 10, 3, 7, 1, 4}},
    {MAGIC_SCHOOL_WATER,    {0, 4, 8, 9, 1, 10, 3, 11, 7, 6, 2, 5}},
    {MAGIC_SCHOOL_EARTH,    {0, 7, 10, 8, 2, 11, 1, 5, 3, 6, 4, 9}},
    {MAGIC_SCHOOL_SPIRIT,   {0, 5, 10, 11, 7, 2, 8, 1, 4, 9, 3, 6}},
    {MAGIC_SCHOOL_MIND,     {0, 5, 9, 8, 3, 7, 6, 4, 1, 11, 2, 10}},
    {MAGIC_SCHOOL_BODY,     {0, 1, 6, 9, 3, 5, 8, 11, 7, 10, 4, 2}},
    {MAGIC_SCHOOL_LIGHT,    {0, 1, 10, 11, 9, 4, 3, 6, 5, 7, 8, 2}},
    {MAGIC_SCHOOL_DARK,     {0, 9, 3, 7, 1, 5, 2, 10, 11, 8, 6, 4}}
}};

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texNames = {
    {MAGIC_SCHOOL_FIRE,   "SBFB00"},
    {MAGIC_SCHOOL_AIR,    "SBAB00"},
    {MAGIC_SCHOOL_WATER,  "SBWB00"},
    {MAGIC_SCHOOL_EARTH,  "SBEB00"},
    {MAGIC_SCHOOL_SPIRIT, "SBSB00"},
    {MAGIC_SCHOOL_MIND,   "SBMB00"},
    {MAGIC_SCHOOL_BODY,   "SBBB00"},
    {MAGIC_SCHOOL_LIGHT,  "SBLB00"},
    {MAGIC_SCHOOL_DARK,   "SBDB00"}
};

static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord1 = {{
    {MAGIC_SCHOOL_FIRE,     {406, 9}},
    {MAGIC_SCHOOL_AIR,      {406, 46}},
    {MAGIC_SCHOOL_WATER,    {406, 84}},
    {MAGIC_SCHOOL_EARTH,    {406, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {407, 158}},
    {MAGIC_SCHOOL_MIND,     {405, 196}},
    {MAGIC_SCHOOL_BODY,     {405, 234}},
    {MAGIC_SCHOOL_LIGHT,    {405, 272}},
    {MAGIC_SCHOOL_DARK,     {405, 309}}
}};

static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord0 = {{
    {MAGIC_SCHOOL_FIRE,     {415, 10}},
    {MAGIC_SCHOOL_AIR,      {415, 46}},
    {MAGIC_SCHOOL_WATER,    {415, 83}},
    {MAGIC_SCHOOL_EARTH,    {415, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {415, 158}},
    {MAGIC_SCHOOL_MIND,     {416, 196}},
    {MAGIC_SCHOOL_BODY,     {416, 234}},
    {MAGIC_SCHOOL_LIGHT,    {416, 271}},
    {MAGIC_SCHOOL_DARK,     {416, 307}}
}};

namespace mm6_book {

constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> schoolPrefixes = {
    {MAGIC_SCHOOL_FIRE,   "fire"},
    {MAGIC_SCHOOL_AIR,    "air"},
    {MAGIC_SCHOOL_WATER,  "wtr"},
    {MAGIC_SCHOOL_EARTH,  "earth"},
    {MAGIC_SCHOOL_SPIRIT, "sprt"},
    {MAGIC_SCHOOL_MIND,   "mind"},
    {MAGIC_SCHOOL_BODY,   "body"},
    {MAGIC_SCHOOL_LIGHT,  "lite"},
    {MAGIC_SCHOOL_DARK,   "dark"}
};

constexpr uint32_t ICON_X_ADDRESS = 0x4BC75C; // int32[9][12], slot 0 is the school picture, slots 1-11 the spells.
constexpr uint32_t ICON_Y_ADDRESS = 0x4BC90C;
constexpr uint32_t NAME_X_ADDRESS = 0x4BC3FC; // int32[9][12], left edge of the name box.
constexpr uint32_t NAME_Y_ADDRESS = 0x4BC5AC;
constexpr int NAME_WIDTH = 110;
constexpr Color SELECTED_NAME_COLOR = Color(255, 12, 8);
constexpr int NAME_Y_OFFSET = -5; // MM6 draws the names above the positions in the table.

constexpr Pointi BOOK_POS = {8, 8};
constexpr Pointi SPELL_BUTTON_POS = {301, 332};
constexpr Pointi EXIT_BUTTON_POS = {360, 332};
constexpr int ACTIVE_TAB_X = 414;
constexpr int INACTIVE_TAB_X = 421;
constexpr int TAB_Y = 13;
constexpr int TAB_STEP = 35;

Pointi tabPosition(MagicSchool school, bool active) {
    return {active ? ACTIVE_TAB_X : INACTIVE_TAB_X, TAB_Y + TAB_STEP * std::to_underlying(school)};
}

int exeTableValue(uint32_t address, MagicSchool school, int slot) {
    return static_cast<int32_t>(mm6ExeData.u32(address + 4 * (12 * std::to_underlying(school) + slot)));
}

/**
 * @param school                        Magic school.
 * @param slot                          0 for the school picture, 1-11 for the spells.
 * @return                              Screen position of the picture in the slot.
 */
Pointi iconPosition(MagicSchool school, int slot) {
    if (mm6ExeData.isLoaded())
        return {exeTableValue(ICON_X_ADDRESS, school, slot), exeTableValue(ICON_Y_ADDRESS, school, slot)};
    if (slot == 0)
        return {48, 18};
    return {68 + 130 * (slot % 3), 32 + 75 * (slot / 3)};
}

/**
 * @param school                        Magic school.
 * @param slot                          Spell slot, 1-11.
 * @return                              Top left corner of the spell name box.
 */
Pointi namePosition(MagicSchool school, int slot) {
    if (mm6ExeData.isLoaded())
        return {exeTableValue(NAME_X_ADDRESS, school, slot), exeTableValue(NAME_Y_ADDRESS, school, slot)};
    return {46 + 130 * (slot % 3), 89 + 75 * (slot / 3)};
}

GUIFont *nameFont() {
    static std::unique_ptr<GUIFont> font = GUIFont::LoadFont("spell.fnt");
    return font.get();
}

} // namespace mm6_book

SpellId spellbookSelectedSpell;

GUIWindow_Spellbook::GUIWindow_Spellbook() : GUIWindow(WINDOW_SpellBook, {0, 0}, render->GetRenderDimensions()) {
    current_screen_type = SCREEN_SPELL_BOOK;
    gameTimer->setPaused(true);

    initializeTextures();
    openSpellbook();

    // Sound 48 is absent in MM7
    pAudioPlayer->playUISound(SOUND_48);
}

void GUIWindow_Spellbook::openSpellbookPage(MagicSchool page) {
    onCloseSpellBookPage();
    pParty->activeCharacter().lastOpenedSpellbookPage = page;
    openSpellbook();
    pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_Spellbook::openSpellbook() {
    if (isMm6()) {
        openSpellbookMm6();
        return;
    }

    int pageSpells = 0;
    const Character &player = pParty->activeCharacter();

    loadSpellbook();

    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        if (!player.bHaveSpell[spell] && !engine->config->debug.AllMagic.value())
            continue;

        int index = spellIndexInMagicSchool(spell);
        CreateButton(fmt::format("SpellBook_Spell{}", index),
                     {pViewport.x + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Xpos,
                     pViewport.y + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Ypos},
                     SBPageSSpellsTextureList[index + 1]->size(), BUTTON_TYPE_NORMAL, UIMSG_Spellbook_ShowHightlightedSpellInfo,
                     UIMSG_SelectSpell, std::to_underlying(spell));
        pageSpells++;
    }

    CreateButton({0, 0}, {0, 0}, BUTTON_TYPE_NORMAL, 0, UIMSG_SpellBook_PressTab, 0, INPUT_ACTION_NEXT_CHAR);
    if (pageSpells) {
        setKeyboardControlGroup(pageSpells, true, 0, 0);
    }

    static constexpr IndexedArray<Pointi, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> buttonPositions = {
        {MAGIC_SCHOOL_FIRE,     {399, 10}},
        {MAGIC_SCHOOL_AIR,      {399, 46}},
        {MAGIC_SCHOOL_WATER,    {399, 83}},
        {MAGIC_SCHOOL_EARTH,    {399, 121}},
        {MAGIC_SCHOOL_SPIRIT,   {399, 158}},
        {MAGIC_SCHOOL_MIND,     {400, 196}},
        {MAGIC_SCHOOL_BODY,     {400, 234}},
        {MAGIC_SCHOOL_LIGHT,    {400, 271}},
        {MAGIC_SCHOOL_DARK,     {400, 307}},
    };

    for (MagicSchool school : allMagicSchools())
        if (player.pActiveSkills[skillForMagicSchool(school)] || engine->config->debug.AllMagic.value())
            CreateButton(fmt::format("SpellBook_School{}", std::to_underlying(school)), buttonPositions[school],
                         {50, 36}, BUTTON_TYPE_NORMAL, 0, UIMSG_OpenSpellbookPage, std::to_underlying(school), INPUT_ACTION_INVALID,
                         localization->spellSchoolName(school));

    pBtn_InstallRemoveSpell = CreateButton({476, 450}, ui_spellbook_btn_quckspell->size(), BUTTON_TYPE_NORMAL, UIMSG_HintSelectRemoveQuickSpellBtn,
                                           UIMSG_ClickInstallRemoveQuickSpellBtn, 0, INPUT_ACTION_INVALID, "", {ui_spellbook_btn_quckspell_click});
    pBtn_CloseBook = CreateButton({561, 450}, ui_spellbook_btn_close->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
                                  localization->str(LSTR_EXIT_DIALOGUE), {ui_spellbook_btn_close_click});
}

void GUIWindow_Spellbook::Update() {
    if (isMm6()) {
        updateMm6();
        return;
    }

    const Character &player = pParty->activeCharacter();
    int pX_coord, pY_coord;

    drawCurrentSchoolBackground();

    for (MagicSchool page : allMagicSchools()) {
        Skill skill = skillForMagicSchool(page);

        if (player.pActiveSkills[skill] || engine->config->debug.AllMagic.value()) {
            auto pPageTexture = ui_spellbook_school_tabs[page][0];
            if (player.lastOpenedSpellbookPage == page) {
                pPageTexture = ui_spellbook_school_tabs[page][1];
                pX_coord = texture_tab_coord1[page][0];
                pY_coord = texture_tab_coord1[page][1];
            } else {
                pPageTexture = ui_spellbook_school_tabs[page][0];
                pX_coord = texture_tab_coord0[page][0];
                pY_coord = texture_tab_coord0[page][1];
            }
            render->DrawQuad2D(pPageTexture, {pX_coord, pY_coord});

            Pointi mousePos = mouse->position();

            for (SpellId spell : spellsForMagicSchool(player.lastOpenedSpellbookPage)) {
                int index = spellIndexInMagicSchool(spell);
                if (player.bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
                    // this should check if player knows spell
                    if (SBPageSSpellsTextureList[index + 1]) {
                        GraphicsImage *pTexture = (spellbookSelectedSpell == spell) ? SBPageCSpellsTextureList[index + 1] : SBPageSSpellsTextureList[index + 1];
                        if (pTexture) {
                            SpellBookIconPos &iconPos = pIconPos[player.lastOpenedSpellbookPage][pSpellbookSpellIndices[player.lastOpenedSpellbookPage][index + 1]];

                            pX_coord = pViewport.x + iconPos.Xpos;
                            pY_coord = pViewport.y + iconPos.Ypos;

                            Recti iconRect = Recti(pX_coord, pY_coord, pTexture->width(), pTexture->height());
                            if (iconRect.contains(mousePos)) { // mouseover highlight
                                if (SBPageCSpellsTextureList[index + 1]) {
                                    render->DrawQuad2D(SBPageCSpellsTextureList[index + 1], {pX_coord, pY_coord});
                                }
                            } else {
                                render->DrawQuad2D(pTexture, {pX_coord, pY_coord});
                            }
                        }
                    }
                }
            }
        }
    }
}

GUIWindow_Spellbook::~GUIWindow_Spellbook() {
    onCloseSpellBookPage();
    onCloseSpellBook();
}

void GUIWindow_Spellbook::loadSpellbook() {
    MagicSchool page = pParty->activeCharacter().lastOpenedSpellbookPage;
    if (pParty->activeCharacter().uQuickSpell != SPELL_NONE && magicSchoolForSpell(pParty->activeCharacter().uQuickSpell) == page)
        spellbookSelectedSpell = pParty->activeCharacter().uQuickSpell;
    else
        spellbookSelectedSpell = SPELL_NONE;

    for (SpellId spell : spellsForMagicSchool(page)) {
        if (pParty->activeCharacter().bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
            int index = spellIndexInMagicSchool(spell);
            if (isMm6()) {
                SBPageSSpellsTextureList[index + 1] = assets->getImage_ColorKey(fmt::format("{}{:03}", mm6_book::schoolPrefixes[page], index + 1));
                continue;
            }

            std::string pContainer;

            pContainer = fmt::format("SB{}S{:02}", spellbook_texture_filename_suffices[page], pSpellbookSpellIndices[page][index + 1]);
            SBPageSSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);

            pContainer = fmt::format("SB{}C{:02}", spellbook_texture_filename_suffices[page], pSpellbookSpellIndices[page][index + 1]);
            SBPageCSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);
        }
    }
}

void GUIWindow_Spellbook::drawCurrentSchoolBackground() {
    MagicSchool page = MAGIC_SCHOOL_FIRE;
    if (pParty->hasActiveCharacter()) {
        page = pParty->activeCharacter().lastOpenedSpellbookPage;
    }
    render->DrawQuad2D(ui_spellbook_school_backgrounds[page], {8, 8});

    render->DrawQuad2D(ui_spellbook_btn_quckspell, {476, 450});
    render->DrawQuad2D(ui_spellbook_btn_close, {561, 450});
}

void GUIWindow_Spellbook::initializeTextures() {
    pAudioPlayer->playUISound(SOUND_openbook);

    if (isMm6()) {
        initializeTexturesMm6();
        return;
    }

    ui_spellbook_btn_close = assets->getImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->getImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->getImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->getImage_Solid("ib-m6-d");

    for (MagicSchool page : allMagicSchools()) {
        ui_spellbook_school_backgrounds[page] = assets->getImage_ColorKey(texNames[page]);
        ui_spellbook_school_tabs[page][0] = assets->getImage_Alpha(fmt::format("tab{}a", std::to_underlying(page) + 1));
        ui_spellbook_school_tabs[page][1] = assets->getImage_Alpha(fmt::format("tab{}b", std::to_underlying(page) + 1));
    }
}

void GUIWindow_Spellbook::onCloseSpellBook() {
    for (GraphicsImage **image : {&_mm6Book, &_mm6PageMask}) {
        if (*image) {
            (*image)->release();
            *image = nullptr;
        }
    }

    if (ui_spellbook_btn_close) {
        ui_spellbook_btn_close->release();
        ui_spellbook_btn_close = nullptr;
    }
    if (ui_spellbook_btn_close_click) {
        ui_spellbook_btn_close_click->release();
        ui_spellbook_btn_close_click = nullptr;
    }

    if (ui_spellbook_btn_quckspell) {
        ui_spellbook_btn_quckspell->release();
        ui_spellbook_btn_quckspell = nullptr;
    }
    if (ui_spellbook_btn_quckspell_click) {
        ui_spellbook_btn_quckspell_click->release();
        ui_spellbook_btn_quckspell_click = nullptr;
    }

    for (MagicSchool page : allMagicSchools()) {
        if (ui_spellbook_school_backgrounds[page]) {
            ui_spellbook_school_backgrounds[page]->release();
            ui_spellbook_school_backgrounds[page] = nullptr;
        }

        if (ui_spellbook_school_tabs[page][0]) {
            ui_spellbook_school_tabs[page][0]->release();
            ui_spellbook_school_tabs[page][0] = nullptr;
        }
        if (ui_spellbook_school_tabs[page][1]) {
            ui_spellbook_school_tabs[page][1]->release();
            ui_spellbook_school_tabs[page][1] = nullptr;
        }
    }

    pAudioPlayer->playUISound(SOUND_closebook);
}

void GUIWindow_Spellbook::onCloseSpellBookPage() {
    for (unsigned int i = 1; i <= 11; i++) {
        if (SBPageCSpellsTextureList[i]) {
            SBPageCSpellsTextureList[i]->release();
            SBPageCSpellsTextureList[i] = nullptr;
        }
        if (SBPageSSpellsTextureList[i]) {
            SBPageSSpellsTextureList[i]->release();
            SBPageSSpellsTextureList[i] = nullptr;
        }
    }

    if (pGUIWindow_CurrentMenu)
        pGUIWindow_CurrentMenu->DeleteButtons();
}

void GUIWindow_Spellbook::initializeTexturesMm6() {
    _mm6Book = assets->getImage_Solid("book");
    _mm6PageMask = assets->getImage_ColorKey("pagemask", Color(252, 0, 252));
    ui_spellbook_btn_quckspell = assets->getImage_Solid("tabspell");
    ui_spellbook_btn_close = assets->getImage_Solid("tabexit");

    for (MagicSchool page : allMagicSchools()) {
        ui_spellbook_school_backgrounds[page] = assets->getImage_ColorKey(fmt::format("{}000", mm6_book::schoolPrefixes[page]));
        ui_spellbook_school_tabs[page][0] = assets->getImage_ColorKey(fmt::format("tab{}a", std::to_underlying(page) + 1));
        ui_spellbook_school_tabs[page][1] = assets->getImage_ColorKey(fmt::format("tab{}b", std::to_underlying(page) + 1));
    }
}

void GUIWindow_Spellbook::openSpellbookMm6() {
    const Character &player = pParty->activeCharacter();
    loadSpellbook();

    int pageSpells = 0;
    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        if (!player.bHaveSpell[spell] && !engine->config->debug.AllMagic.value())
            continue;

        int slot = spellIndexInMagicSchool(spell) + 1;
        if (!SBPageSSpellsTextureList[slot])
            continue;
        CreateButton(fmt::format("SpellBook_Spell{}", slot - 1), mm6_book::iconPosition(chapter, slot),
                     SBPageSSpellsTextureList[slot]->size(), BUTTON_TYPE_NORMAL, UIMSG_Spellbook_ShowHightlightedSpellInfo,
                     UIMSG_SelectSpell, std::to_underlying(spell));
        pageSpells++;
    }

    CreateButton({0, 0}, {0, 0}, BUTTON_TYPE_NORMAL, 0, UIMSG_SpellBook_PressTab, 0, INPUT_ACTION_NEXT_CHAR);
    if (pageSpells)
        setKeyboardControlGroup(pageSpells, true, 0, 0);

    for (MagicSchool school : allMagicSchools()) {
        if (!player.pActiveSkills[skillForMagicSchool(school)] && !engine->config->debug.AllMagic.value())
            continue;
        CreateButton(fmt::format("SpellBook_School{}", std::to_underlying(school)), mm6_book::tabPosition(school, school == chapter),
                     {39, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_OpenSpellbookPage, std::to_underlying(school),
                     INPUT_ACTION_INVALID, localization->spellSchoolName(school));
    }

    pBtn_InstallRemoveSpell = CreateButton(mm6_book::SPELL_BUTTON_POS, ui_spellbook_btn_quckspell->size(), BUTTON_TYPE_NORMAL,
                                           UIMSG_HintSelectRemoveQuickSpellBtn, UIMSG_ClickInstallRemoveQuickSpellBtn, 0,
                                           INPUT_ACTION_INVALID, "", {ui_spellbook_btn_quckspell});
    pBtn_CloseBook = CreateButton(mm6_book::EXIT_BUTTON_POS, ui_spellbook_btn_close->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0,
                                  INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE), {ui_spellbook_btn_close});
}

void GUIWindow_Spellbook::updateMm6() {
    const Character &player = pParty->activeCharacter();
    MagicSchool chapter = player.lastOpenedSpellbookPage;
    auto hasSchool = [&](MagicSchool school) {
        return player.pActiveSkills[skillForMagicSchool(school)] || engine->config->debug.AllMagic.value();
    };

    render->DrawQuad2D(_mm6Book, mm6_book::BOOK_POS);
    render->DrawQuad2D(_mm6PageMask, mm6_book::BOOK_POS);
    for (MagicSchool school : allMagicSchools())
        if (school != chapter && hasSchool(school))
            render->DrawQuad2D(ui_spellbook_school_tabs[school][0], mm6_book::tabPosition(school, false));
    render->DrawQuad2D(ui_spellbook_school_backgrounds[chapter], mm6_book::iconPosition(chapter, 0));
    if (hasSchool(chapter))
        render->DrawQuad2D(ui_spellbook_school_tabs[chapter][1], mm6_book::tabPosition(chapter, true));

    GUIFont *font = mm6_book::nameFont();
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        int slot = spellIndexInMagicSchool(spell) + 1;
        if (!SBPageSSpellsTextureList[slot] || (!player.bHaveSpell[spell] && !engine->config->debug.AllMagic.value()))
            continue;

        render->DrawQuad2D(SBPageSSpellsTextureList[slot], mm6_book::iconPosition(chapter, slot));

        if (font) {
            const std::string &name = pSpellStats->pInfos[spell].name;
            Pointi namePos = mm6_book::namePosition(chapter, slot);
            namePos.x += std::max(0, (mm6_book::NAME_WIDTH - font->GetLineWidth(name)) / 2);
            Color color = spellbookSelectedSpell == spell ? mm6_book::SELECTED_NAME_COLOR : colorTable.Black;
            namePos.y += mm6_book::NAME_Y_OFFSET;
            font->DrawText(Recti(0, 0, 640, 480), namePos, color, name, 0, color);
        }
    }

    render->DrawQuad2D(ui_spellbook_btn_quckspell, mm6_book::SPELL_BUTTON_POS);
    render->DrawQuad2D(ui_spellbook_btn_close, mm6_book::EXIT_BUTTON_POS);
}
