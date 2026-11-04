#include "UIMessageScroll.h"

#include <memory>
#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Party.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Tables/ItemTable.h"

#include "Engine/Graphics/Renderer/Renderer.h"

#include "GUI/GUIFont.h"

#include "Library/Color/ColorTable.h"

#include "Utility/String/Split.h"

/**
 * Ours: the bill of sale of the golem quest ends with the lines saying where the golem parts were delivered. A line
 * turns green once the party has all its parts, in the inventory or already built into the golem, and yellow when it
 * has one of two. One head is all the golem needs, so the Bracada line is green as soon as a head is built in.
 *
 * @param text                          Scroll text from scroll.txt.
 * @return                              The text with the delivery lines coloured.
 */
static std::string colorGolemDeliveries(const std::string &text) {
    struct Part {
        ItemId item;
        QuestBit placed; // Set by the golem in the castle once the part is built in.
    };
    // In the order of the lines: Barrow Downs, Tatalia, Avlee, Deyja and Bracada.
    static const std::vector<std::vector<Part>> deliveries = {
        {{ITEM_QUEST_GOLEM_CHEST, static_cast<QuestBit>(66)}},
        {{ITEM_QUEST_GOLEM_RIGHT_ARM, static_cast<QuestBit>(70)}},
        {{ITEM_QUEST_GOLEM_LEFT_ARM, static_cast<QuestBit>(69)}},
        {{ITEM_QUEST_GOLEM_LEFT_LEG, static_cast<QuestBit>(67)}, {ITEM_QUEST_GOLEM_RIGHT_LEG, static_cast<QuestBit>(68)}},
        {{ITEM_QUEST_GOLEM_HEAD, static_cast<QuestBit>(71)}, {ITEM_QUEST_ABBEY_NORMAL_GOLEM_HEAD, static_cast<QuestBit>(72)}},
    };

    std::vector<std::string> lines;
    for (std::string_view line : split(text).by('\n'))
        lines.emplace_back(line);
    std::vector<size_t> deliveryLines;
    for (size_t i = lines.size(); i-- > 0 && deliveryLines.size() < deliveries.size();)
        if (lines[i].find_first_not_of(" \r") != std::string::npos)
            deliveryLines.insert(deliveryLines.begin(), i);
    if (deliveryLines.size() != deliveries.size())
        return text;

    for (size_t i = 0; i < deliveries.size(); i++) {
        int found = 0;
        bool builtIn = false;
        for (const Part &part : deliveries[i]) {
            builtIn |= pParty->_questBits[part.placed];
            if (pParty->_questBits[part.placed] || pParty->hasItem(part.item) || pParty->pPickedItem.itemId == part.item)
                found++;
        }
        bool headBuiltIn = deliveries[i][0].item == ITEM_QUEST_GOLEM_HEAD && builtIn;
        if (found == 0 && !headBuiltIn)
            continue;
        Color color = found == static_cast<int>(deliveries[i].size()) || headBuiltIn ? colorTable.Green : colorTable.Yellow;
        lines[deliveryLines[i]] = fmt::format("{::}{}\f00000", color.tag(), lines[deliveryLines[i]]);
    }

    std::string result;
    for (size_t i = 0; i < lines.size(); i++)
        result += (i ? "\n" : "") + lines[i];
    return result;
}

void CreateMsgScrollWindow(ItemId mscroll_id) {
    if (!pGUIWindow_ScrollWindow && isMessageScroll(mscroll_id)) {
        pGUIWindow_ScrollWindow = std::make_unique<GUIWindow_MessageScroll>(Pointi(0, 0), render->GetRenderDimensions(), mscroll_id, "");
    }
}

void GUIWindow_MessageScroll::Update() {
    GUIWindow_MessageScroll &a1 = *pGUIWindow_ScrollWindow;
    a1.sHint.clear();
    a1.frameRect.x = 1;
    a1.frameRect.y = 1;
    a1.frameRect.w = 468;
    std::string text = pMessageScrolls[pGUIWindow_ScrollWindow->scroll_type];
    if (pGUIWindow_ScrollWindow->scroll_type == ITEM_MESSAGE_BILL_OF_SALE)
        text = colorGolemDeliveries(text);
    int textHeight = assets->pFontSmallnum->CalcTextHeight(text, a1.frameRect.w, 0);
    unsigned int v0 = textHeight + 2 * (unsigned char)assets->pFontCreate->GetHeight() + 24;
    a1.frameRect.h = v0;
    if ((v0 + a1.frameRect.y) > 479) {
        v0 = 479 - a1.frameRect.y;
        a1.frameRect.h = 479 - a1.frameRect.y;
    }
    GUIWindow::DrawMessageBox(a1.frameRect, a1.sHint);
    a1.frameRect.x += 12;
    a1.frameRect.w -= 28;
    a1.frameRect.y += 12;
    a1.frameRect.h -= 12;

    const std::string &name = pItemTable->items[pGUIWindow_ScrollWindow->scroll_type].name;

    DrawTitleText(assets->pFontCreate.get(), 0, 0, colorTable.White, fmt::format("{::}{}\f00000\n", colorTable.PaleCanary.tag(), name), 3, a1.frameRect);
    DrawText(assets->pFontSmallnum.get(), {1, assets->pFontCreate->GetHeight() - 3}, colorTable.White, text, a1.frameRect);
}
