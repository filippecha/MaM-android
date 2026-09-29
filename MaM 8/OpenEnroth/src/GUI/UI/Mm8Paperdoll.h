#pragma once

#include <vector>

#include "Engine/Objects/Inventory.h"

#include "Library/Geometry/Point.h"

class Character;
class GraphicsImage;

/**
 * One picture of an MM8 paper doll.
 */
struct Mm8DollPiece {
    GraphicsImage *image = nullptr;
    Pointi position;
    InventoryEntry item; // The equipped item the picture shows, empty for the body.
};

/**
 * Lays out an MM8 paper doll: background, body, arms and the equipped items, in drawing order. The layout follows
 * MMExtension's Misc/Paper Doll/mm8/PaperDol.txt and Scripts/Modules/PaperDoll.lua, which describe what MM8.exe does.
 *
 * @param character                     Character to show.
 * @param origin                        Top left corner of the doll background, everything else is relative to it.
 * @return                              Pictures to draw, from the back to the front.
 */
std::vector<Mm8DollPiece> mm8PaperdollPieces(Character &character, Pointi origin);
