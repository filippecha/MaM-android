#include "Mm6Dialogue.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Utility/String/Format.h"

namespace mm6_dialogue {

std::vector<int> optionPositions(const std::vector<int> &textHeights, int areaTop) {
    constexpr int areaBottom = 282;
    int areaHeight = areaBottom - areaTop;
    constexpr int textToGap = 5; // Every option's button extends this far below its text.

    int count = textHeights.size();
    int totalHeight = 0;
    for (int height : textHeights)
        totalHeight += height;

    int spacing = std::max(0, (areaHeight - totalHeight) / (count + 1));
    int offset = areaTop + std::max(0, areaHeight - totalHeight - spacing * count) / 2;

    std::vector<int> result;
    for (int height : textHeights) {
        int y = offset + spacing;
        result.push_back(y);
        offset = y + height + textToGap;
    }
    return result;
}

std::string panelName(int panel) {
    return fmt::format("evpan{:03}", panel);
}

} // namespace mm6_dialogue
