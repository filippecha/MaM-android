#include "PixelScale.h"

#include <cstdint>

namespace {
struct Neighbours {
    uint8_t a, b, c, d, e, f, g, h, i;  // a b c / d e f / g h i, e is the pixel itself.
};

Neighbours neighbours(GrayscaleImageView image, int x, int y) {
    int w = image.width(), h = image.height();
    auto at = [&](int px, int py) {
        px = px < 0 ? 0 : px >= w ? w - 1 : px;
        py = py < 0 ? 0 : py >= h ? h - 1 : py;
        return image[py][px];
    };
    return {at(x - 1, y - 1), at(x, y - 1), at(x + 1, y - 1),
            at(x - 1, y),     at(x, y),     at(x + 1, y),
            at(x - 1, y + 1), at(x, y + 1), at(x + 1, y + 1)};
}
}  // namespace

GrayscaleImage scale2x(GrayscaleImageView image) {
    int w = image.width(), h = image.height();
    GrayscaleImage result = GrayscaleImage::uninitialized(w * 2, h * 2);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Neighbours n = neighbours(image, x, y);
            uint8_t e0 = n.e, e1 = n.e, e2 = n.e, e3 = n.e;
            if (n.b != n.h && n.d != n.f) {
                e0 = n.d == n.b ? n.d : n.e;
                e1 = n.b == n.f ? n.f : n.e;
                e2 = n.d == n.h ? n.d : n.e;
                e3 = n.h == n.f ? n.f : n.e;
            }
            result[2 * y][2 * x] = e0;
            result[2 * y][2 * x + 1] = e1;
            result[2 * y + 1][2 * x] = e2;
            result[2 * y + 1][2 * x + 1] = e3;
        }
    }
    return result;
}

GrayscaleImage scale3x(GrayscaleImageView image) {
    int w = image.width(), h = image.height();
    GrayscaleImage result = GrayscaleImage::uninitialized(w * 3, h * 3);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Neighbours n = neighbours(image, x, y);
            uint8_t e0 = n.e, e1 = n.e, e2 = n.e, e3 = n.e, e4 = n.e, e5 = n.e, e6 = n.e, e7 = n.e, e8 = n.e;
            if (n.b != n.h && n.d != n.f) {
                e0 = n.d == n.b ? n.d : n.e;
                e1 = (n.d == n.b && n.e != n.c) || (n.b == n.f && n.e != n.a) ? n.b : n.e;
                e2 = n.b == n.f ? n.f : n.e;
                e3 = (n.d == n.b && n.e != n.g) || (n.d == n.h && n.e != n.a) ? n.d : n.e;
                e5 = (n.b == n.f && n.e != n.i) || (n.h == n.f && n.e != n.c) ? n.f : n.e;
                e6 = n.d == n.h ? n.d : n.e;
                e7 = (n.d == n.h && n.e != n.i) || (n.h == n.f && n.e != n.g) ? n.h : n.e;
                e8 = n.h == n.f ? n.f : n.e;
            }
            uint8_t cells[3][3] = {{e0, e1, e2}, {e3, e4, e5}, {e6, e7, e8}};
            for (int dy = 0; dy < 3; dy++)
                for (int dx = 0; dx < 3; dx++)
                    result[3 * y + dy][3 * x + dx] = cells[dy][dx];
        }
    }
    return result;
}
