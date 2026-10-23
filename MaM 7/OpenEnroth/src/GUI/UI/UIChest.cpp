#include "UIChest.h"

#include <algorithm>
#include <utility>

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
    RgbaImage plaque = RgbaImage::copy(ui_exit_cancel_button_background->rgba());
    if (plaque.width() == 169 && plaque.height() == 35) {
        // The caption sits on the plaque at x 53..118, y 8..27. The 14 blank columns left of it repeat over it.
        for (int y = 8; y < 28; y++)
            for (int x = 53; x < 119; x++)
                plaque[y][x] = plaque[y][53 + (x - 53) % 14];
    }
    // The caption goes into the texture like the baked ones: two pixels wide and blurred, so that the phone's
    // upscaling leaves it as dark and smooth as "Zavřít" below it.
    GUIFont *font = assets->pFontArrus.get();
    RgbaImage text = RgbaImage::solid(Color(), plaque.width(), plaque.height() + font->GetHeight() + 2);
    int textX = std::max(0, (plaque.width() - font->GetLineWidth(_takeAllButton->label) - 1) / 2);
    font->DrawTextLineToBuff(colorTable.White, Color(), &text[10][textX], _takeAllButton->label, text.width());
    for (int y = 0; y < text.height(); y++)
        for (int x = text.width() - 1; x > 0; x--)
            if (text[y][x - 1].a)
                text[y][x] = text[y][x - 1];
    static const int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    for (int y = 1; y < plaque.height() - 1; y++) {
        for (int x = 1; x < plaque.width() - 1; x++) {
            int coverage = 16;  // The stroke itself is black, its surroundings only darken to half the stroke's weight.
            if (!text[y][x].a) {
                coverage = 0;
                for (int ky = -1; ky <= 1; ky++)
                    for (int kx = -1; kx <= 1; kx++)
                        if (text[y + ky][x + kx].a)
                            coverage += kernel[ky + 1][kx + 1];
                coverage /= 2;
            }
            Color &pixel = plaque[y][x];
            pixel.r = pixel.r * (16 - coverage) / 16;
            pixel.g = pixel.g * (16 - coverage) / 16;
            pixel.b = pixel.b * (16 - coverage) / 16;
        }
    }
    _takeAllTexture = GraphicsImage::Create(std::move(plaque));
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
