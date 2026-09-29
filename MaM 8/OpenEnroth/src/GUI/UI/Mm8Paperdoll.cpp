#include "Mm8Paperdoll.h"

#include <array>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/Mm8Ids.h"
#include "Engine/Tables/ItemTable.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"

namespace {

enum Mm8DollBody {
    MM8_DOLL_MALE,
    MM8_DOLL_FEMALE,
    MM8_DOLL_MINOTAUR,
    MM8_DOLL_TROLL,
    MM8_DOLL_DRAGON,
    MM8_DOLL_BASE, // Male or female, only in the item table.
};

enum Piece {
    PIECE_MAIN,
    PIECE_ARM1, // Armor sleeve when holding a weapon.
    PIECE_ARM1_FREE, // Armor sleeve when not holding a weapon.
    PIECE_SCARF, // Front part of a cloak.
    PIECE_SCARF2,
};

struct DollItem {
    int picture; // Number in the item picture name, "item084" is 84.
    Mm8DollBody body;
    Piece piece;
    const char *image; // Empty for nothing.
    Pointi offset;
};

constexpr std::array<DollItem, 243> dollItems = {{
    {122, MM8_DOLL_MALE, PIECE_MAIN, "item122v1a", {51, 79}},
    {122, MM8_DOLL_MALE, PIECE_SCARF, "item122v1b", {84, 80}},
    {122, MM8_DOLL_FEMALE, PIECE_MAIN, "item122v2a", {47, 90}},
    {122, MM8_DOLL_FEMALE, PIECE_SCARF, "item122v2b", {75, 90}},
    {122, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item122v1a", {40, 120}},
    {122, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item122v1b", {68, 120}},
    {122, MM8_DOLL_TROLL, PIECE_MAIN, "item122v1a", {41, 94}},
    {122, MM8_DOLL_TROLL, PIECE_SCARF, "item122v1b", {61, 99}},
    {123, MM8_DOLL_MALE, PIECE_MAIN, "item123v1a", {54, 81}},
    {123, MM8_DOLL_MALE, PIECE_SCARF, "item123v1b", {77, 81}},
    {123, MM8_DOLL_FEMALE, PIECE_MAIN, "item123v2a", {54, 91}},
    {123, MM8_DOLL_FEMALE, PIECE_SCARF, "item123v2b", {67, 91}},
    {123, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item123v1a", {40, 119}},
    {123, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item123v1b", {64, 116}},
    {123, MM8_DOLL_TROLL, PIECE_MAIN, "item123v1a", {39, 94}},
    {123, MM8_DOLL_TROLL, PIECE_SCARF, "item123v1b", {61, 99}},
    {124, MM8_DOLL_MALE, PIECE_MAIN, "item124v1a", {56, 82}},
    {124, MM8_DOLL_MALE, PIECE_SCARF, "item124v1b", {76, 76}},
    {124, MM8_DOLL_FEMALE, PIECE_MAIN, "item124v2a", {60, 91}},
    {124, MM8_DOLL_FEMALE, PIECE_SCARF, "item124v2b", {69, 91}},
    {124, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item124v1a", {40, 115}},
    {124, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item124v1b", {65, 113}},
    {124, MM8_DOLL_TROLL, PIECE_MAIN, "item124v1a", {44, 103}},
    {124, MM8_DOLL_TROLL, PIECE_SCARF, "item124v1b", {65, 99}},
    {125, MM8_DOLL_MALE, PIECE_MAIN, "item125v1a", {49, 80}},
    {125, MM8_DOLL_MALE, PIECE_SCARF, "item125v1b", {80, 73}},
    {125, MM8_DOLL_FEMALE, PIECE_MAIN, "item125v2a", {55, 93}},
    {125, MM8_DOLL_FEMALE, PIECE_SCARF, "item125v2b", {72, 83}},
    {125, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item125v1a", {40, 119}},
    {125, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item125v1b", {65, 114}},
    {125, MM8_DOLL_TROLL, PIECE_MAIN, "item125v1a", {44, 97}},
    {125, MM8_DOLL_TROLL, PIECE_SCARF, "item125v1b", {65, 95}},
    {126, MM8_DOLL_MALE, PIECE_MAIN, "item126v1a", {57, 82}},
    {126, MM8_DOLL_MALE, PIECE_SCARF, "item126v1b", {64, 57}},
    {126, MM8_DOLL_MALE, PIECE_SCARF2, "item126v1c", {64, 57}},
    {126, MM8_DOLL_FEMALE, PIECE_MAIN, "item126v2a", {54, 95}},
    {126, MM8_DOLL_FEMALE, PIECE_SCARF, "item126v2b", {51, 64}},
    {126, MM8_DOLL_FEMALE, PIECE_SCARF2, "item126v2c", {51, 64}},
    {126, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item126v1a", {40, 123}},
    {126, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item126v1b", {48, 104}},
    {126, MM8_DOLL_MINOTAUR, PIECE_SCARF2, "item126v1c", {48, 104}},
    {126, MM8_DOLL_TROLL, PIECE_MAIN, "item126v1a", {44, 88}},
    {126, MM8_DOLL_TROLL, PIECE_SCARF, "item126v1b", {47, 78}},
    {126, MM8_DOLL_TROLL, PIECE_SCARF2, "item126v1c", {47, 78}},
    {260, MM8_DOLL_MALE, PIECE_MAIN, "item260v1a", {51, 78}},
    {260, MM8_DOLL_MALE, PIECE_SCARF, "item260v1b", {81, 64}},
    {260, MM8_DOLL_FEMALE, PIECE_MAIN, "item260v2a", {53, 94}},
    {260, MM8_DOLL_FEMALE, PIECE_SCARF, "item260v2b", {72, 76}},
    {260, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item260v1a", {40, 120}},
    {260, MM8_DOLL_MINOTAUR, PIECE_SCARF, "item260v1b", {68, 105}},
    {260, MM8_DOLL_TROLL, PIECE_MAIN, "item260v1a", {41, 94}},
    {260, MM8_DOLL_TROLL, PIECE_SCARF, "item260v1b", {61, 84}},
    {84, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {84, MM8_DOLL_MALE, PIECE_ARM1, "item084v1", {47, 78}},
    {84, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item084v1a", {47, 78}},
    {84, MM8_DOLL_FEMALE, PIECE_ARM1, "item084v2", {50, 93}},
    {84, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item084v2a", {50, 93}},
    {84, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item084v3", {44, 108}},
    {84, MM8_DOLL_TROLL, PIECE_MAIN, "item084v4", {0, 0}},
    {85, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {85, MM8_DOLL_MALE, PIECE_ARM1, "item085v1", {30, 84}},
    {85, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item085v1a", {30, 84}},
    {85, MM8_DOLL_FEMALE, PIECE_ARM1, "item085v2", {60, 95}},
    {85, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item085v2a", {60, 95}},
    {85, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item085v3", {45, 111}},
    {85, MM8_DOLL_TROLL, PIECE_MAIN, "item085v4", {0, 0}},
    {86, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {86, MM8_DOLL_MALE, PIECE_ARM1, "item086v1", {29, 72}},
    {86, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item086v1a", {29, 72}},
    {86, MM8_DOLL_FEMALE, PIECE_ARM1, "item086v2", {57, 86}},
    {86, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item086v2a", {57, 86}},
    {86, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item086v3", {40, 101}},
    {86, MM8_DOLL_TROLL, PIECE_MAIN, "item086v4", {0, 0}},
    {87, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {87, MM8_DOLL_MALE, PIECE_ARM1, "item087v1", {29, 78}},
    {87, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item087v1a", {29, 78}},
    {87, MM8_DOLL_FEMALE, PIECE_ARM1, "item087v2", {47, 90}},
    {87, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item087v2a", {47, 90}},
    {87, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item087v3", {29, 107}},
    {87, MM8_DOLL_TROLL, PIECE_MAIN, "item087v4", {0, 0}},
    {88, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {88, MM8_DOLL_MALE, PIECE_ARM1, "item088v1", {46, 78}},
    {88, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item088v1a", {46, 78}},
    {88, MM8_DOLL_FEMALE, PIECE_ARM1, "item088v2", {42, 88}},
    {88, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item088v2a", {42, 88}},
    {88, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item088v3", {38, 103}},
    {88, MM8_DOLL_TROLL, PIECE_MAIN, "item088v4", {0, 0}},
    {89, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {89, MM8_DOLL_MALE, PIECE_ARM1, "item089v1", {61, 82}},
    {89, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item089v1a", {61, 82}},
    {89, MM8_DOLL_FEMALE, PIECE_ARM1, "item089v2", {61, 98}},
    {89, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item089v2a", {61, 98}},
    {89, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item089v3", {46, 111}},
    {89, MM8_DOLL_TROLL, PIECE_MAIN, "item089v4", {0, 0}},
    {90, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {90, MM8_DOLL_MALE, PIECE_ARM1, "item090v1", {55, 82}},
    {90, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item090v1a", {55, 82}},
    {90, MM8_DOLL_FEMALE, PIECE_ARM1, "item090v2", {60, 98}},
    {90, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item090v2a", {60, 98}},
    {90, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item090v3", {45, 116}},
    {90, MM8_DOLL_TROLL, PIECE_MAIN, "item090v4", {0, 0}},
    {91, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {91, MM8_DOLL_MALE, PIECE_ARM1, "item091v1", {45, 76}},
    {91, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item091v1a", {45, 76}},
    {91, MM8_DOLL_FEMALE, PIECE_ARM1, "item091v2", {56, 89}},
    {91, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item091v2a", {56, 89}},
    {91, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item091v3", {46, 107}},
    {91, MM8_DOLL_TROLL, PIECE_MAIN, "item091v4", {0, 0}},
    {92, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {92, MM8_DOLL_MALE, PIECE_ARM1, "item092v1", {49, 76}},
    {92, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item092v1a", {49, 76}},
    {92, MM8_DOLL_FEMALE, PIECE_ARM1, "item092v2", {52, 90}},
    {92, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item092v2a", {52, 90}},
    {92, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item092v3", {36, 100}},
    {92, MM8_DOLL_TROLL, PIECE_MAIN, "item092v4", {0, 0}},
    {93, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {93, MM8_DOLL_MALE, PIECE_ARM1, "item093v1", {49, 76}},
    {93, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item093v1a", {49, 76}},
    {93, MM8_DOLL_FEMALE, PIECE_ARM1, "item093v2", {43, 84}},
    {93, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item093v2a", {43, 84}},
    {93, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item093v3", {28, 105}},
    {93, MM8_DOLL_TROLL, PIECE_MAIN, "item093v4", {0, 0}},
    {94, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {94, MM8_DOLL_MALE, PIECE_ARM1, "item094v1", {62, 81}},
    {94, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item094v1a", {62, 81}},
    {94, MM8_DOLL_FEMALE, PIECE_ARM1, "item094v2", {52, 94}},
    {94, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item094v2a", {52, 94}},
    {94, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item094v3", {41, 115}},
    {95, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {95, MM8_DOLL_MALE, PIECE_ARM1, "item095v1", {55, 79}},
    {95, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item095v1a", {55, 79}},
    {95, MM8_DOLL_FEMALE, PIECE_ARM1, "item095v2", {51, 91}},
    {95, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item095v2a", {51, 91}},
    {95, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item095v3", {39, 109}},
    {96, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {96, MM8_DOLL_MALE, PIECE_ARM1, "item096v1", {46, 73}},
    {96, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item096v1a", {46, 73}},
    {96, MM8_DOLL_FEMALE, PIECE_ARM1, "item096v2", {47, 90}},
    {96, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item096v2a", {47, 90}},
    {96, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item096v3", {34, 109}},
    {97, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {97, MM8_DOLL_MALE, PIECE_ARM1, "item097v1", {50, 60}},
    {97, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item097v1a", {50, 60}},
    {97, MM8_DOLL_FEMALE, PIECE_ARM1, "item097v2", {54, 65}},
    {97, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item097v2a", {54, 65}},
    {97, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item097v3", {39, 96}},
    {98, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {98, MM8_DOLL_MALE, PIECE_ARM1, "item098v1", {53, 70}},
    {98, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item098v1a", {53, 70}},
    {98, MM8_DOLL_FEMALE, PIECE_ARM1, "item098v2", {48, 82}},
    {98, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item098v2a", {48, 82}},
    {98, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item098v3", {39, 100}},
    {251, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {251, MM8_DOLL_MALE, PIECE_ARM1, "item251v1", {29, 57}},
    {251, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item251v1a", {29, 57}},
    {251, MM8_DOLL_FEMALE, PIECE_ARM1, "item251v2", {47, 70}},
    {251, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item251v2a", {47, 70}},
    {251, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item251v3", {27, 100}},
    {251, MM8_DOLL_TROLL, PIECE_MAIN, "item251v4", {0, 0}},
    {252, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {252, MM8_DOLL_MALE, PIECE_ARM1, "item252v1", {29, 68}},
    {252, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item252v1a", {29, 68}},
    {252, MM8_DOLL_FEMALE, PIECE_ARM1, "item252v2", {49, 86}},
    {252, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item252v2a", {49, 86}},
    {252, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item252v3", {36, 103}},
    {252, MM8_DOLL_TROLL, PIECE_MAIN, "item252v4", {0, 0}},
    {253, MM8_DOLL_BASE, PIECE_MAIN, "", {0, 0}},
    {253, MM8_DOLL_MALE, PIECE_ARM1, "item253v1", {30, 69}},
    {253, MM8_DOLL_MALE, PIECE_ARM1_FREE, "item253v1a", {30, 69}},
    {253, MM8_DOLL_FEMALE, PIECE_ARM1, "item253v2", {52, 82}},
    {253, MM8_DOLL_FEMALE, PIECE_ARM1_FREE, "item253v2a", {52, 82}},
    {253, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item253v3", {32, 107}},
    {117, MM8_DOLL_MALE, PIECE_MAIN, "item117v1", {64, 147}},
    {117, MM8_DOLL_FEMALE, PIECE_MAIN, "item117v2", {68, 151}},
    {117, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item117v1", {55, 194}},
    {117, MM8_DOLL_TROLL, PIECE_MAIN, "item117v1", {58, 182}},
    {118, MM8_DOLL_MALE, PIECE_MAIN, "item118v1", {66, 147}},
    {118, MM8_DOLL_FEMALE, PIECE_MAIN, "item118v2", {70, 154}},
    {118, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item118v1", {57, 194}},
    {118, MM8_DOLL_TROLL, PIECE_MAIN, "item118v1", {55, 178}},
    {119, MM8_DOLL_MALE, PIECE_MAIN, "item119v1", {63, 146}},
    {119, MM8_DOLL_FEMALE, PIECE_MAIN, "item119v2", {71, 155}},
    {119, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item119v1", {54, 193}},
    {119, MM8_DOLL_TROLL, PIECE_MAIN, "item119v1", {52, 185}},
    {120, MM8_DOLL_MALE, PIECE_MAIN, "item120v1", {67, 138}},
    {120, MM8_DOLL_FEMALE, PIECE_MAIN, "item120v2", {70, 154}},
    {120, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item120v1", {58, 190}},
    {120, MM8_DOLL_TROLL, PIECE_MAIN, "item120v1", {58, 175}},
    {121, MM8_DOLL_MALE, PIECE_MAIN, "item121v1", {65, 151}},
    {121, MM8_DOLL_FEMALE, PIECE_MAIN, "item121v2", {72, 153}},
    {121, MM8_DOLL_MINOTAUR, PIECE_MAIN, "item121v1", {58, 198}},
    {121, MM8_DOLL_TROLL, PIECE_MAIN, "item121v1", {55, 183}},
    {109, MM8_DOLL_MALE, PIECE_MAIN, "item109v1", {87, 28}},
    {109, MM8_DOLL_FEMALE, PIECE_MAIN, "item109v2", {72, 41}},
    {109, MM8_DOLL_TROLL, PIECE_MAIN, "item109v4", {77, 42}},
    {110, MM8_DOLL_MALE, PIECE_MAIN, "item110v1", {79, 24}},
    {110, MM8_DOLL_FEMALE, PIECE_MAIN, "item110v2", {65, 35}},
    {110, MM8_DOLL_TROLL, PIECE_MAIN, "item110v4", {64, 30}},
    {111, MM8_DOLL_MALE, PIECE_MAIN, "item111v1", {85, 27}},
    {111, MM8_DOLL_FEMALE, PIECE_MAIN, "item111v2", {74, 45}},
    {111, MM8_DOLL_TROLL, PIECE_MAIN, "item111v4", {68, 41}},
    {112, MM8_DOLL_MALE, PIECE_MAIN, "item112v1", {57, 8}},
    {112, MM8_DOLL_FEMALE, PIECE_MAIN, "item112v2", {49, 25}},
    {112, MM8_DOLL_TROLL, PIECE_MAIN, "item112v4", {43, 13}},
    {113, MM8_DOLL_MALE, PIECE_MAIN, "item113v1", {77, 18}},
    {113, MM8_DOLL_FEMALE, PIECE_MAIN, "item113v2", {62, 28}},
    {113, MM8_DOLL_TROLL, PIECE_MAIN, "item113v4", {50, 18}},
    {114, MM8_DOLL_MALE, PIECE_MAIN, "item114v1", {85, 26}},
    {114, MM8_DOLL_FEMALE, PIECE_MAIN, "item114v1", {72, 42}},
    {114, MM8_DOLL_TROLL, PIECE_MAIN, "item114v4", {78, 40}},
    {115, MM8_DOLL_MALE, PIECE_MAIN, "item115v1", {78, 14}},
    {115, MM8_DOLL_FEMALE, PIECE_MAIN, "item115v1", {66, 30}},
    {115, MM8_DOLL_TROLL, PIECE_MAIN, "item115v4", {74, 35}},
    {116, MM8_DOLL_MALE, PIECE_MAIN, "item116v1", {88, 30}},
    {116, MM8_DOLL_FEMALE, PIECE_MAIN, "item116v1", {72, 42}},
    {116, MM8_DOLL_TROLL, PIECE_MAIN, "item116v4", {78, 38}},
    {256, MM8_DOLL_MALE, PIECE_MAIN, "item256v1", {77, 6}},
    {256, MM8_DOLL_FEMALE, PIECE_MAIN, "item256v2", {68, 26}},
    {256, MM8_DOLL_TROLL, PIECE_MAIN, "item256v4", {61, 17}},
    {257, MM8_DOLL_MALE, PIECE_MAIN, "item257v1", {83, 30}},
    {257, MM8_DOLL_FEMALE, PIECE_MAIN, "item257v1", {68, 47}},
    {257, MM8_DOLL_TROLL, PIECE_MAIN, "item257v4", {70, 50}},
    {258, MM8_DOLL_MALE, PIECE_MAIN, "item258v1", {81, 0}},
    {258, MM8_DOLL_FEMALE, PIECE_MAIN, "item258v2", {67, -5}},
    {258, MM8_DOLL_TROLL, PIECE_MAIN, "item258v4", {77, 12}},
    {132, MM8_DOLL_MALE, PIECE_MAIN, "item132v1", {46, 261}},
    {132, MM8_DOLL_FEMALE, PIECE_MAIN, "item132v2", {75, 260}},
    {132, MM8_DOLL_TROLL, PIECE_MAIN, "item132v4", {46, 290}},
    {133, MM8_DOLL_MALE, PIECE_MAIN, "item133v1", {40, 255}},
    {133, MM8_DOLL_FEMALE, PIECE_MAIN, "item133v2", {73, 260}},
    {133, MM8_DOLL_TROLL, PIECE_MAIN, "item133v4", {44, 278}},
    {134, MM8_DOLL_MALE, PIECE_MAIN, "item134v1", {31, 250}},
    {134, MM8_DOLL_FEMALE, PIECE_MAIN, "item134v2", {64, 255}},
    {134, MM8_DOLL_TROLL, PIECE_MAIN, "item134v4", {45, 269}},
    {135, MM8_DOLL_MALE, PIECE_MAIN, "item135v1", {30, 250}},
    {135, MM8_DOLL_FEMALE, PIECE_MAIN, "item135v2", {64, 257}},
    {135, MM8_DOLL_TROLL, PIECE_MAIN, "item135v4", {45, 268}},
    {136, MM8_DOLL_MALE, PIECE_MAIN, "item136v1", {26, 241}},
    {136, MM8_DOLL_FEMALE, PIECE_MAIN, "item136v2", {57, 242}},
    {136, MM8_DOLL_TROLL, PIECE_MAIN, "item136v4", {45, 250}},
    {262, MM8_DOLL_MALE, PIECE_MAIN, "item262v1", {41, 254}},
    {262, MM8_DOLL_FEMALE, PIECE_MAIN, "item262v2", {70, 250}},
    {262, MM8_DOLL_TROLL, PIECE_MAIN, "item262v4", {47, 272}},
}};

// Body offsets by face, MM8.exe 0x4F8120.
constexpr std::array<Pointi, 28> bodyOffsets = {{
    {30, 33}, {60, 47}, {30, 31}, {60, 47}, {29, 34}, {60, 47}, {29, 34}, {60, 47}, {29, 34}, {60, 54},
    {29, 38}, {60, 48}, {30, 33}, {60, 47}, {30, 34}, {60, 47}, {30, 38}, {60, 47}, {30, 36}, {60, 47},
    {47, 67}, {48, 54}, {43, 43}, {41, 51}, {0, 0}, {0, 0}, {29, 33}, {61, 45},
}};

/**
 * Arms and hands of one body. The first arm holds the main hand weapon, the second the shield or the second weapon.
 */
struct BodyParts {
    Pointi arm1; // RHu, holding a weapon.
    Pointi arm1Free; // RHd.
    Pointi hand1; // RHb, over the weapon grip.
    Pointi arm2Free; // Empty second hand.
    const char *arm2FreeImage;
    Pointi arm2TwoHanded; // Holding the other end of a two-handed weapon.
    const char *arm2TwoHandedImage;
    Pointi hand2; // LHu, over a shield or a second weapon.
    Pointi extraHand; // Where the shield or second weapon goes.
    Pointi mainHand; // Where the main hand weapon goes.
};

constexpr std::array<BodyParts, 4> bodyParts = {{
    {{27, 117}, {29, 110}, {27, 118}, {132, 150}, "LHo", {138, 150}, "LHu", {138, 150}, {144, 194}, {37, 125}},
    {{43, 142}, {25, 132}, {43, 142}, {129, 148}, "LHd", {117, 148}, "LHo", {129, 148}, {133, 200}, {51, 149}},
    {{12, 166}, {14, 168}, {12, 170}, {122, 162}, "LHd", {119, 161}, "LHo", {122, 162}, {145, 232}, {20, 182}},
    {{3, 156}, {8, 159}, {3, 157}, {131, 159}, "LHd", {131, 159}, "LHo", {131, 159}, {149, 209}, {13, 169}},
}};

constexpr Pointi BOW_OFFSET = {98, 117};

Mm8DollBody dollBody(int face) {
    if (face == 20 || face == 21)
        return MM8_DOLL_MINOTAUR;
    if (face == 22 || face == 23)
        return MM8_DOLL_TROLL;
    if (face == 24 || face == 25)
        return MM8_DOLL_DRAGON;
    return mm8FaceIsFemale(face) ? MM8_DOLL_FEMALE : MM8_DOLL_MALE;
}

std::string backDollName(int face) {
    switch (dollBody(face)) {
    case MM8_DOLL_MINOTAUR: return "backdol3";
    case MM8_DOLL_TROLL: return "backdol4";
    case MM8_DOLL_DRAGON: return fmt::format("pc{:02}bod", face + 1);
    default: return mm8FaceIsFemale(face) ? "backdoll" : "backdolm";
    }
}

/**
 * @return                              The table line for the item picture, body and piece, nullptr if there is none.
 */
const DollItem *findDollItem(const Item &item, Mm8DollBody body, Piece piece) {
    std::string icon = ascii::toLower(item.GetIconName());
    const DollItem *result = nullptr;
    for (const DollItem &entry : dollItems) {
        if (entry.piece != piece || fmt::format("item{:03}", entry.picture) != icon)
            continue;
        if (entry.body == body || (entry.body == MM8_DOLL_BASE && (body == MM8_DOLL_MALE || body == MM8_DOLL_FEMALE)))
            result = &entry; // Later lines take precedence.
    }
    return result;
}

/**
 * @param name                          Icon name.
 * @return                              The icon turned by 90 degrees counterclockwise.
 */
GraphicsImage *rotatedIcon(std::string_view name) {
    static std::map<std::string, GraphicsImage *, std::less<>> cache;
    if (auto pos = cache.find(name); pos != cache.end())
        return pos->second;
    const RgbaImage &source = assets->getImage_ColorKey(name)->rgba();
    RgbaImage rotated = RgbaImage::uninitialized(source.height(), source.width());
    for (int y = 0; y < rotated.height(); y++)
        for (int x = 0; x < rotated.width(); x++)
            rotated[y][x] = source[x][source.width() - 1 - y];
    GraphicsImage *result = GraphicsImage::Create(std::move(rotated));
    cache.emplace(std::string(name), result);
    return result;
}

} // namespace

std::vector<Mm8DollPiece> mm8PaperdollPieces(Character &character, Pointi origin) {
    std::vector<Mm8DollPiece> result;
    int face = character.uCurrentFace;
    Mm8DollBody body = dollBody(face);
    auto add = [&](std::string_view image, Pointi offset, InventoryEntry item = {}) {
        result.push_back({assets->getImage_ColorKey(image), origin + offset, item});
    };
    auto facePiece = [&](std::string_view suffix, Pointi offset) {
        add(fmt::format("pc{:02}{}", face + 1, suffix), offset);
    };
    // Items without a line in the table show their inventory picture, placed by the Equip X and Y of items.txt.
    auto addTableItem = [&](InventoryEntry item, Piece piece) {
        if (!item)
            return;
        if (const DollItem *entry = findDollItem(*item, body, piece)) {
            if (*entry->image)
                add(entry->image, entry->offset, item);
        } else if (piece == PIECE_MAIN) {
            add(item->GetIconName(), pItemTable->items[item->itemId].paperdollAnchorOffset, item);
        }
    };
    auto addHeldItem = [&](InventoryEntry item, Pointi anchor) {
        if (item) // Equip X and Y count from the hand here.
            add(item->GetIconName(), anchor - pItemTable->items[item->itemId].paperdollAnchorOffset, item);
    };

    result.push_back({assets->getImage_Paletted(backDollName(face)), origin, {}}); // Flagged transparent, but MM8 draws its zero color.
    if (body == MM8_DOLL_DRAGON || face >= bodyOffsets.size())
        return result; // Dragons are their own background and wear nothing.

    const BodyParts &parts = bodyParts[body];
    InventoryEntry mainHand = character.inventory.entry(ITEM_SLOT_MAIN_HAND);
    InventoryEntry offHand = character.inventory.entry(ITEM_SLOT_OFF_HAND);
    InventoryEntry armor = character.inventory.entry(ITEM_SLOT_ARMOUR);
    InventoryEntry cloak = character.inventory.entry(ITEM_SLOT_CLOAK);
    bool twoHanded = mainHand && mainHand->type() == ITEM_TYPE_TWO_HANDED && !offHand;
    bool shield = offHand && offHand->isShield();

    addHeldItem(character.inventory.entry(ITEM_SLOT_BOW), BOW_OFFSET);
    addTableItem(cloak, PIECE_MAIN);
    facePiece("bod", bodyOffsets[face]);
    if (mainHand) {
        facePiece("RHu", parts.arm1);
    } else {
        facePiece("RHd", parts.arm1Free);
    }
    if (twoHanded) {
        facePiece(parts.arm2TwoHandedImage, parts.arm2TwoHanded);
    } else if (!offHand) {
        facePiece(parts.arm2FreeImage, parts.arm2Free);
    } else if (shield) {
        facePiece("LHu", parts.hand2);
    }
    addTableItem(armor, PIECE_MAIN);
    addTableItem(character.inventory.entry(ITEM_SLOT_BOOTS), PIECE_MAIN);
    addTableItem(armor, mainHand ? PIECE_ARM1 : PIECE_ARM1_FREE);
    addTableItem(character.inventory.entry(ITEM_SLOT_BELT), PIECE_MAIN);
    addTableItem(cloak, PIECE_SCARF);
    addTableItem(character.inventory.entry(ITEM_SLOT_HELMET), PIECE_MAIN);
    addTableItem(cloak, PIECE_SCARF2);
    addHeldItem(mainHand, parts.mainHand);
    if (mainHand)
        facePiece("RHb", parts.hand1);
    if (offHand && !shield) {
        // MM8 turns a second weapon by 90 degrees, PaperDol.txt has the matrix 0,-1,1,0 for it. The corner that was
        // top left ends up bottom left.
        GraphicsImage *image = rotatedIcon(offHand->GetIconName());
        Pointi equip = pItemTable->items[offHand->itemId].paperdollAnchorOffset;
        result.push_back({image, origin + parts.extraHand + Pointi(-equip.y, equip.x - image->height()), offHand});
    } else {
        addHeldItem(offHand, parts.extraHand);
    }
    if (offHand && !shield)
        facePiece("LHu", parts.hand2);
    return result;
}
