#pragma once

#include <cassert>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Engine/Graphics/Image.h"

/**
 * A canvas-like container that allows adding images with associated values, then querying pixels to get the value of
 * the topmost non-transparent image at that position.
 *
 * @tparam T                            The type of values associated with images.
 */
template<class T>
class HitMap {
 public:
    /**
     * Add an image with associated value to the canvas.
     *
     * @param position                  Top-left position where the image should be placed.
     * @param image                     Pointer to the graphics image.
     * @param value                     Value to associate with this image.
     */
    void add(Pointi position, GraphicsImage *image, T value) {
        assert(image);

        _entries.emplace_back(position, image, std::move(value));

        Recti imageRect = Recti(position, image->size());
        if (_entries.size() == 1) {
            _bounds = imageRect;
        } else {
            _bounds |= imageRect;
        }
    }

    /**
     * Clear all images from the canvas.
     */
    void clear() {
        _entries.clear();
        _bounds = Recti();
    }

    /**
     * Query a pixel position to get the value from the topmost non-transparent image.
     *
     * @param point                     Position to query.
     * @param defaultValue              Value to return if no non-transparent image is found at the position.
     * @return                          Value associated with the topmost non-transparent image at the given position,
     *                                  or `defaultValue` if no such image exists.
     */
    T query(Pointi point, T defaultValue = {}) const {
        if (!Recti(_bounds.x - kSlack, _bounds.y - kSlack, _bounds.w + 2 * kSlack, _bounds.h + 2 * kSlack).contains(point))
            return defaultValue;

        // Iterate entries in reverse order (topmost first).
        for (auto it = _entries.rbegin(); it != _entries.rend(); ++it)
            if (opaque(*it, point))
                return it->value;

        // Ours: a finger or a click just beside an item still takes it, the nearest item within a few pixels wins.
        for (int r = 1; r <= kSlack; r++)
            for (int dy = -r; dy <= r; dy++)
                for (int dx = -r; dx <= r; dx++)
                    if (std::max(std::abs(dx), std::abs(dy)) == r)
                        for (auto it = _entries.rbegin(); it != _entries.rend(); ++it)
                            if (opaque(*it, point + Pointi(dx, dy)))
                                return it->value;

        return defaultValue;
    }

 private:
    static constexpr int kSlack = 4; // Logical pixels.

    struct Entry {
        Pointi position;
        GraphicsImage *image = nullptr;
        T value;

        Entry(Pointi pos, GraphicsImage *img, T val) : position(pos), image(img), value(val) {}
    };

    // Ours: the point is in logical pixels, an HD texture has scale() texture pixels per logical one. Reading it
    // unscaled tested only the top left third of every item on the paperdoll, it was hard to take anything off.
    static bool opaque(const Entry &entry, Pointi point) {
        Pointi relativePos = point - entry.position;
        if (!Recti(Pointi(0, 0), entry.image->size()).contains(relativePos))
            return false;
        int s = entry.image->scale();
        RgbaImage &rgba = entry.image->rgba();
        Pointi pixel = relativePos * s + Pointi(s / 2, s / 2);
        return rgba.geometry().contains(pixel) && rgba[pixel].a > 0;
    }

    std::vector<Entry> _entries;
    Recti _bounds; // Bounding rectangle of all images.
};
