#include "GUI/UI/UISpellbook.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/Mm8Ids.h"
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

namespace mm8_book {

constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> schoolPrefixes = {
    {MAGIC_SCHOOL_FIRE,   "f"},
    {MAGIC_SCHOOL_AIR,    "a"},
    {MAGIC_SCHOOL_WATER,  "w"},
    {MAGIC_SCHOOL_EARTH,  "e"},
    {MAGIC_SCHOOL_SPIRIT, "s"},
    {MAGIC_SCHOOL_MIND,   "m"},
    {MAGIC_SCHOOL_BODY,   "b"},
    {MAGIC_SCHOOL_LIGHT,  "l"},
    {MAGIC_SCHOOL_DARK,   "dk"}
};

constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> tabPrefixes = {
    {MAGIC_SCHOOL_FIRE,   "stf"},
    {MAGIC_SCHOOL_AIR,    "sta"},
    {MAGIC_SCHOOL_WATER,  "stw"},
    {MAGIC_SCHOOL_EARTH,  "ste"},
    {MAGIC_SCHOOL_SPIRIT, "sts"},
    {MAGIC_SCHOOL_MIND,   "stm"},
    {MAGIC_SCHOOL_BODY,   "stb"},
    {MAGIC_SCHOOL_LIGHT,  "stl"},
    {MAGIC_SCHOOL_DARK,   "std"}
};

// Spell pictures of each school by page position, MM8.exe 0x503638. Slot 0 is unused.
constexpr IndexedArray<std::array<Pointi, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> iconPositions = {{
    {MAGIC_SCHOOL_FIRE,   {{{0, 0}, {330, 297}, {190, 165}, {341, 38}, {335, 192}, {47, 324}, {42, 163}, {468, 46}, {478, 176}, {172, 340}, {159, 22}, {442, 333}}}},
    {MAGIC_SCHOOL_AIR,    {{{0, 0}, {42, 149}, {443, 37}, {458, 194}, {173, 168}, {468, 333}, {183, 31}, {38, 268}, {326, 331}, {180, 327}, {322, 175}, {329, 36}}}},
    {MAGIC_SCHOOL_WATER,  {{{0, 0}, {452, 333}, {425, 218}, {186, 39}, {323, 36}, {323, 140}, {33, 249}, {138, 154}, {320, 270}, {40, 369}, {167, 294}, {472, 47}}}},
    {MAGIC_SCHOOL_EARTH,  {{{0, 0}, {43, 152}, {56, 320}, {452, 181}, {472, 334}, {331, 322}, {336, 35}, {183, 73}, {458, 49}, {134, 404}, {338, 163}, {169, 234}}}},
    {MAGIC_SCHOOL_SPIRIT, {{{0, 0}, {327, 37}, {161, 34}, {463, 41}, {328, 183}, {461, 189}, {184, 168}, {498, 324}, {347, 346}, {40, 292}, {41, 149}, {85, 348}}}},
    {MAGIC_SCHOOL_MIND,   {{{0, 0}, {469, 114}, {338, 158}, {52, 162}, {323, 342}, {478, 336}, {176, 37}, {191, 172}, {342, 39}, {175, 313}, {440, 243}, {37, 314}}}},
    {MAGIC_SCHOOL_BODY,   {{{0, 0}, {163, 35}, {178, 180}, {40, 282}, {327, 322}, {139, 340}, {434, 44}, {356, 174}, {329, 37}, {441, 325}, {432, 174}, {39, 141}}}},
    {MAGIC_SCHOOL_LIGHT,  {{{0, 0}, {53, 245}, {340, 142}, {180, 315}, {170, 41}, {334, 291}, {451, 338}, {454, 132}, {382, 35}, {39, 332}, {39, 152}, {462, 214}}}},
    {MAGIC_SCHOOL_DARK,   {{{0, 0}, {408, 33}, {181, 39}, {474, 284}, {328, 324}, {48, 281}, {324, 22}, {323, 223}, {184, 176}, {35, 153}, {446, 142}, {110, 362}}}},
}};

constexpr int TAB_X = 591;
constexpr std::array<int, 9> TAB_Y = {0, 40, 80, 120, 159, 198, 237, 276, 315};
constexpr Pointi SPELL_BUTTON_POS = {0, 380};
constexpr Pointi EXIT_BUTTON_POS = {0, 430};

/**
 * @param school                        Magic school.
 * @param slot                          Spell of the school, 1-11.
 * @return                              Number of the spell picture, MM8.exe 0x503AB8 swaps two fire spells.
 */
int pictureIndex(MagicSchool school, int slot) {
    if (school == MAGIC_SCHOOL_FIRE && (slot == 2 || slot == 4))
        return 6 - slot;
    return slot;
}

Pointi tabPosition(MagicSchool school) {
    return {TAB_X, TAB_Y[std::to_underlying(school)]};
}

/**
 * The page of dark elf, vampire or dragon abilities, MM8.exe 0x4CD2EF.
 */
struct RacialPage {
    const char *prefix;
    const char *tab;
    Pointi tabPosition;
    std::array<Pointi, 5> iconPositions; // MM8.exe 0x503638 after the schools. Slot 0 is unused.
};

const RacialPage *racialPage(const Character &character) {
    static constexpr RacialPage darkElf = {"de", "stde", {TAB_X, 354}, {{{0, 0}, {134, 110}, {355, 292}, {57, 241}, {344, 79}}}};
    static constexpr RacialPage vampire = {"v", "stv", {TAB_X, 393}, {{{0, 0}, {130, 132}, {399, 258}, {54, 318}, {350, 49}}}};
    static constexpr RacialPage dragon = {"d", "stdr", {TAB_X, 432}, {{{0, 0}, {52, 294}, {123, 96}, {363, 261}, {344, 59}}}};
    switch (mm8ClassKind(character.classType)) {
    case MM8_DARK_ELF: return &darkElf;
    case MM8_VAMPIRE: return &vampire;
    case MM8_DRAGON: return &dragon;
    default: return nullptr;
    }
}

bool pageAvailable(const Character &character, MagicSchool page) {
    if (page == MAGIC_SCHOOL_MM8_RACIAL)
        return hasMm8RacialSpellbookPage(character);
    return character.pActiveSkills[skillForMagicSchool(page)] || engine->config->debug.AllMagic.value();
}

std::vector<SpellId> pageSpells(const Character &character, MagicSchool page) {
    if (page == MAGIC_SCHOOL_MM8_RACIAL) {
        std::span<const SpellId> racial = mm8RacialSpells(mm8ClassKind(character.classType));
        return {racial.begin(), racial.end()};
    }
    Segment<SpellId> spells = spellsForMagicSchool(page);
    return {spells.begin(), spells.end()};
}

Pointi iconPosition(const Character &character, MagicSchool page, int slot) {
    if (page == MAGIC_SCHOOL_MM8_RACIAL)
        return racialPage(character)->iconPositions[slot];
    return iconPositions[page][pictureIndex(page, slot)];
}

std::string pictureName(const Character &character, MagicSchool page, int slot, char state) {
    if (page == MAGIC_SCHOOL_MM8_RACIAL)
        return fmt::format("sb{}{:02}{}", racialPage(character)->prefix, slot, state);
    return fmt::format("sb{}{:02}{}", schoolPrefixes[page], pictureIndex(page, slot), state);
}

} // namespace mm8_book

bool hasMm8RacialSpellbookPage(const Character &character) {
    return isMm8() && mm8_book::racialPage(character) && character.pActiveSkills[SKILL_MM8_RACIAL];
}

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
    if (isMm8()) {
        openSpellbookMm8();
        return;
    }

    int pageSpells = 0;
    const Character &player = pParty->activeCharacter();

    loadSpellbook();

    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        if (!player.knowsSpell(spell) && !engine->config->debug.AllMagic.value())
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
    if (isMm8()) {
        updateMm8();
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
                if (player.knowsSpell(spell) || engine->config->debug.AllMagic.value()) {
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

    if (isMm8() && page == MAGIC_SCHOOL_MM8_RACIAL) {
        const Character &character = pParty->activeCharacter();
        for (SpellId spell : mm8_book::pageSpells(character, page)) {
            int slot = spellIndexInMagicSchool(spell) + 1;
            SBPageSSpellsTextureList[slot] = assets->getImage_ColorKey(mm8_book::pictureName(character, page, slot, 'a'));
            SBPageCSpellsTextureList[slot] = assets->getImage_ColorKey(mm8_book::pictureName(character, page, slot, 'b'));
        }
        return;
    }

    for (SpellId spell : spellsForMagicSchool(page)) {
        if (pParty->activeCharacter().knowsSpell(spell) || engine->config->debug.AllMagic.value()) {
            int index = spellIndexInMagicSchool(spell);
            if (isMm6()) {
                SBPageSSpellsTextureList[index + 1] = assets->getImage_ColorKey(fmt::format("{}{:03}", mm6_book::schoolPrefixes[page], index + 1));
                continue;
            }
            if (isMm8()) {
                const Character &character = pParty->activeCharacter();
                SBPageSSpellsTextureList[index + 1] = assets->getImage_ColorKey(mm8_book::pictureName(character, page, index + 1, 'a'));
                SBPageCSpellsTextureList[index + 1] = assets->getImage_ColorKey(mm8_book::pictureName(character, page, index + 1, 'b'));
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
    if (isMm8()) {
        initializeTexturesMm8();
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
        if (!player.knowsSpell(spell) && !engine->config->debug.AllMagic.value())
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
        if (!SBPageSSpellsTextureList[slot] || (!player.knowsSpell(spell) && !engine->config->debug.AllMagic.value()))
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

void GUIWindow_Spellbook::initializeTexturesMm8() {
    ui_spellbook_btn_quckspell = assets->getImage_ColorKey("stssu");
    ui_spellbook_btn_quckspell_click = assets->getImage_ColorKey("stssd");
    ui_spellbook_btn_close = assets->getImage_ColorKey("stxtu");
    ui_spellbook_btn_close_click = assets->getImage_ColorKey("stxtd");

    for (MagicSchool page : allMagicSchools()) {
        ui_spellbook_school_backgrounds[page] = assets->getImage_Solid(fmt::format("sb{}000", mm8_book::schoolPrefixes[page]));
        ui_spellbook_school_tabs[page][0] = assets->getImage_ColorKey(fmt::format("{}u", mm8_book::tabPrefixes[page]));
        ui_spellbook_school_tabs[page][1] = assets->getImage_ColorKey(fmt::format("{}d", mm8_book::tabPrefixes[page]));
    }
}

void GUIWindow_Spellbook::openSpellbookMm8() {
    Character &player = pParty->activeCharacter();
    if (!mm8_book::pageAvailable(player, player.lastOpenedSpellbookPage) && hasMm8RacialSpellbookPage(player))
        player.lastOpenedSpellbookPage = MAGIC_SCHOOL_MM8_RACIAL; // Dragons have no school of magic.
    if (player.lastOpenedSpellbookPage == MAGIC_SCHOOL_MM8_RACIAL && !hasMm8RacialSpellbookPage(player))
        player.lastOpenedSpellbookPage = MAGIC_SCHOOL_FIRE;
    loadSpellbook();

    int pageSpells = 0;
    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : mm8_book::pageSpells(player, chapter)) {
        if (!player.knowsSpell(spell) && !engine->config->debug.AllMagic.value())
            continue;

        int slot = spellIndexInMagicSchool(spell) + 1;
        if (!SBPageSSpellsTextureList[slot])
            continue;
        CreateButton(fmt::format("SpellBook_Spell{}", slot - 1), mm8_book::iconPosition(player, chapter, slot),
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
        CreateButton(fmt::format("SpellBook_School{}", std::to_underlying(school)), mm8_book::tabPosition(school),
                     ui_spellbook_school_tabs[school][0]->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_OpenSpellbookPage, std::to_underlying(school),
                     INPUT_ACTION_INVALID, localization->spellSchoolName(school));
    }
    if (hasMm8RacialSpellbookPage(player)) {
        const mm8_book::RacialPage *page = mm8_book::racialPage(player);
        CreateButton("SpellBook_SchoolRacial", page->tabPosition, assets->getImage_ColorKey(fmt::format("{}u", page->tab))->size(),
                     BUTTON_TYPE_NORMAL, 0, UIMSG_OpenSpellbookPage, std::to_underlying(MAGIC_SCHOOL_MM8_RACIAL), INPUT_ACTION_INVALID,
                     characterSkillName(player, SKILL_MM8_RACIAL));
    }

    pBtn_InstallRemoveSpell = CreateButton(mm8_book::SPELL_BUTTON_POS, ui_spellbook_btn_quckspell->size(), BUTTON_TYPE_NORMAL,
                                           UIMSG_HintSelectRemoveQuickSpellBtn, UIMSG_ClickInstallRemoveQuickSpellBtn, 0,
                                           INPUT_ACTION_INVALID, "", {ui_spellbook_btn_quckspell_click});
    pBtn_CloseBook = CreateButton(mm8_book::EXIT_BUTTON_POS, ui_spellbook_btn_close->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0,
                                  INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE), {ui_spellbook_btn_close_click});
}

void GUIWindow_Spellbook::updateMm8() {
    const Character &player = pParty->activeCharacter();
    MagicSchool chapter = player.lastOpenedSpellbookPage;

    const mm8_book::RacialPage *racialPage = hasMm8RacialSpellbookPage(player) ? mm8_book::racialPage(player) : nullptr;
    if (chapter == MAGIC_SCHOOL_MM8_RACIAL)
        render->DrawQuad2D(assets->getImage_Solid(fmt::format("sb{}000", racialPage->prefix)), {0, 0});
    else
        render->DrawQuad2D(ui_spellbook_school_backgrounds[chapter], {0, 0});
    for (MagicSchool school : allMagicSchools())
        if (player.pActiveSkills[skillForMagicSchool(school)] || engine->config->debug.AllMagic.value())
            render->DrawQuad2D(ui_spellbook_school_tabs[school][school == chapter], mm8_book::tabPosition(school));
    if (racialPage)
        render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("{}{}", racialPage->tab, chapter == MAGIC_SCHOOL_MM8_RACIAL ? 'd' : 'u')),
                           racialPage->tabPosition);

    Pointi mousePos = mouse->position();
    for (SpellId spell : mm8_book::pageSpells(player, chapter)) {
        int slot = spellIndexInMagicSchool(spell) + 1;
        if (!SBPageSSpellsTextureList[slot] || (!player.knowsSpell(spell) && !engine->config->debug.AllMagic.value()))
            continue;

        Pointi position = mm8_book::iconPosition(player, chapter, slot);
        bool lit = spellbookSelectedSpell == spell || Recti(position, SBPageSSpellsTextureList[slot]->size()).contains(mousePos);
        render->DrawQuad2D(lit && SBPageCSpellsTextureList[slot] ? SBPageCSpellsTextureList[slot] : SBPageSSpellsTextureList[slot], position);
    }

    render->DrawQuad2D(ui_spellbook_btn_quckspell, mm8_book::SPELL_BUTTON_POS);
    render->DrawQuad2D(ui_spellbook_btn_close, mm8_book::EXIT_BUTTON_POS);
}
