#include "QuestTable.h"

#include <array>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, QBIT_FIRST, QBIT_LAST> pQuestTable;

void initializeQuests(const Blob &quests) {
    // quests.txt table structure: quest bit | text (localized) | dev notes (not used) |
    //                             quest giver name (not localized, not used).
    pQuestTable.fill({});

    for (std::string_view line : split(quests.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        QuestBit qbit = static_cast<QuestBit>(fromString<int>(tokens[0]));
        std::string text = unquote(tokens[1]);
        // Ours: the unused bits 271 and up have the text "0", the quest book showed it as a quest "0" once a script
        // set one of them. They stay empty, so the book leaves them out.
        if (text == "0")
            continue;
        pQuestTable[qbit] = text;
    }
}
