#include "MessageScrollTable.h"

#include <array>
#include <string>

#include "Engine/Objects/Mm6Ids.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/GameVariant.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, ITEM_FIRST_MESSAGE_SCROLL, ITEM_LAST_MESSAGE_SCROLL> pMessageScrolls;

void initializeMessageScrolls(const Blob &scrolls) {
    // scroll.txt table structure: item index | message text (localized) | scroll title (localized, not used) | (empty).
    for (std::string_view line : split(scrolls.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Skip tab-only trailing lines.

        int id = fromString<int>(tokens[0]);
        ItemId i = isMm6() ? itemIdFromMm6(id) : static_cast<ItemId>(id);
        if (i < ITEM_FIRST_MESSAGE_SCROLL || i > ITEM_LAST_MESSAGE_SCROLL)
            continue;
        pMessageScrolls[i] = unquote(tokens[1]);
    }
}
