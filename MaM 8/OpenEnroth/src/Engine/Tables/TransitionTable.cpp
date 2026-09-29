#include "TransitionTable.h"

#include <array>
#include <cassert>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<std::string, 501> pTransitionStrings; // MM8 has 500 transitions, MM7 464.

void initializeTransitions(const Blob &transitions) {
    // trans.txt table structure: index | description (localized) | name (not localized, not used).
    pTransitionStrings.fill({});

    for (std::string_view line : split(transitions.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t'); // Some rows have no description, so it defaults to "".
        if (trim(tokens[0]).empty())
            continue; // MM8 trans.txt has rows without an index.
        int i = fromString<int>(tokens[0]);
        pTransitionStrings[i] = unquote(tokens[1]);
    }
}
