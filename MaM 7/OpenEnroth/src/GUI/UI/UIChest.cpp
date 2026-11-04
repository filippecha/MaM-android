#include "UIChest.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Timer.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"
#include "Engine/Tables/ChestTable.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/ItemGrid.h"

GUIWindow_Chest::GUIWindow_Chest(int chestId) : GUIWindow(WINDOW_Chest, {0, 0}, render->GetRenderDimensions()), _chestId(chestId) {
    CreateCharacterButtons();

    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
                                   localization->str(LSTR_EXIT_DIALOGUE), {ui_exit_cancel_button_background});
    CreateButton({7, 8}, {460, 343}, BUTTON_TYPE_NORMAL, 0, UIMSG_CHEST_ClickItem, 0);
    // Ours: everything out of the chest at once, above the exit button.
    _takeAllButton = CreateButton({471, 400}, {169, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_CHEST_TakeAll, 0, INPUT_ACTION_INVALID,
                                  "Vz\xedt v\x9a" "e", {ui_exit_cancel_button_background});
    // The texture may be an HD replacement, s texture pixels per logical pixel.
    int s = ui_exit_cancel_button_background->scale();
    RgbaImage plaque = RgbaImage::copy(ui_exit_cancel_button_background->rgba());
    if (plaque.width() == 169 * s && plaque.height() == 35 * s) {
        // The caption sits on the plaque at x 53..118, y 8..27. The 14 blank columns left of it repeat over it.
        for (int y = 8 * s; y < 28 * s; y++)
            for (int x = 53 * s; x < 119 * s; x++)
                plaque[y][x] = plaque[y][53 * s + (x - 53 * s) % (14 * s)];
    }
    // The caption in the typeface of the baked ones (create.fnt): a one pixel stroke with the same light
    // anti-aliasing halo as "Zavřít" below it, drawn in logical pixels and stretched over the texture.
    GUIFont *font = assets->pFontCreate.get();
    int logicalW = plaque.width() / s, logicalH = plaque.height() / s;
    RgbaImage text = RgbaImage::solid(Color(), logicalW, logicalH + font->GetHeight() + 2);
    int textX = std::max(0, (logicalW - font->GetLineWidth(_takeAllButton->label)) / 2);
    font->DrawTextLineToBuff(colorTable.White, Color(), &text[10][textX], _takeAllButton->label, text.width());
    for (int y = s; y < plaque.height() - s; y++) {
        for (int x = s; x < plaque.width() - s; x++) {
            int lx = x / s, ly = y / s;
            int coverage = 0;  // Of 16: the stroke is black, its four neighbours a quarter.
            if (text[ly][lx].a) {
                coverage = 16;
            } else {
                for (auto [dx, dy] : {std::pair(-1, 0), std::pair(1, 0), std::pair(0, -1), std::pair(0, 1)})
                    if (text[ly + dy][lx + dx].a)
                        coverage = std::max(coverage, 4);
            }
            if (!coverage)
                continue;
            Color &pixel = plaque[y][x];
            pixel.r = pixel.r * (16 - coverage) / 16;
            pixel.g = pixel.g * (16 - coverage) / 16;
            pixel.b = pixel.b * (16 - coverage) / 16;
        }
    }
    // Ours: between this button (400..435) and the exit one (445..480) ten rows showed the dark bar of the main UI
    // behind them. The texture grows by these rows, filled with the plain stone (or ice) of the plaque around its
    // button: the bottom rows 31..34 and the top rows 0..4, both mirrored so that the joins stay smooth.
    if (plaque.width() == 169 * s && plaque.height() == 35 * s) {
        std::vector<int> fill;
        for (int y = 35 * s - 1; y >= 31 * s; y--)
            fill.push_back(y);
        for (int y = 31 * s; y < 32 * s; y++)
            fill.push_back(y);
        for (int y = 5 * s - 1; y >= 0; y--)
            fill.push_back(y);
        RgbaImage tall = RgbaImage::uninitialized(plaque.width(), 35 * s + static_cast<int>(fill.size()));
        for (int y = 0; y < tall.height(); y++) {
            int from = y < 35 * s ? y : fill[y - 35 * s];
            for (int x = 0; x < plaque.width(); x++)
                tall[y][x] = plaque[from][x];
        }
        plaque = std::move(tall);
    }
    _takeAllTexture = GraphicsImage::Create(std::move(plaque), s);
    current_screen_type = SCREEN_CHEST;
    gameTimer->setPaused(true);
}

GUIWindow_Chest::~GUIWindow_Chest() {
    if (_takeAllTexture)
        _takeAllTexture->release();
}

void GUIWindow_Chest::Update() {
    if (current_screen_type == SCREEN_CHEST_INVENTORY && pParty->hasActiveCharacter()) {
        draw_leather();
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
    } else if (current_screen_type == SCREEN_CHEST) {
        int uChestID = _chestId;
        int chestDescId = vChests[uChestID].chestTypeId;
        int chest_offs_x = chestTable[chestDescId].inventoryOffset.x;
        int chest_offs_y = chestTable[chestDescId].inventoryOffset.y;
        int chestWidthCells = chestTable[chestDescId].size.w;
        int chestHeghtCells = chestTable[chestDescId].size.h;

        GraphicsImage *chest_background = assets->getImage_ColorKey(chestTable[chestDescId].textureName);
        render->DrawQuad2D(chest_background, {8, 8});

        render->SetUIClipRect({ chest_offs_x, chest_offs_y, 32 * chestWidthCells, 32 * chestHeghtCells });
        CharacterUI_DrawPickedItemUnderlay({ chest_offs_x, chest_offs_y });
        render->ResetUIClipRect();

        for (InventoryEntry entry : vChests[uChestID].inventory.entries()) {
            if (entry.zone() != INVENTORY_ZONE_GRID)
                continue;

            auto item_texture = assets->getImage_ColorKey(entry->GetIconName());
            int X_offset = itemOffset(item_texture->width());
            int Y_offset = itemOffset(item_texture->height());
            int itemPixelPosX = chest_offs_x + 32 * entry.geometry().x + X_offset;
            int itemPixelPosY = chest_offs_y + 32 * entry.geometry().y + Y_offset;

            assert(0 < itemPixelPosX && itemPixelPosX < 640);
            assert(0 < itemPixelPosY && itemPixelPosY < 480);
            render->DrawQuad2D(item_texture, {itemPixelPosX, itemPixelPosY});
        }

        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
        if (_takeAllButton)
            render->DrawQuad2D(_takeAllTexture, _takeAllButton->rect.topLeft());
    }
}
