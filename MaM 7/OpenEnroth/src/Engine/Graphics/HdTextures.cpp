#include "HdTextures.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

#ifdef __ANDROID__
#include <sys/sysinfo.h>
#endif

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Graphics/Renderer/Renderer.h"

#include "Library/Image/Jpeg.h"
#include "Library/Image/Png.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"

/** Area filter: every target pixel averages the source pixels it covers, with fractional weights at the edges. */
static RgbaImage areaResize(const RgbaImage &image, int width, int height) {
    RgbaImage result = RgbaImage::uninitialized(width, height);
    float sx = float(image.width()) / width, sy = float(image.height()) / height;
    for (int y = 0; y < height; y++) {
        float y0 = y * sy, y1 = (y + 1) * sy;
        for (int x = 0; x < width; x++) {
            float x0 = x * sx, x1 = (x + 1) * sx;
            float r = 0, g = 0, b = 0, a = 0, total = 0;
            for (int py = static_cast<int>(y0); py < image.height() && py < y1; py++) {
                float wy = std::min(y1, py + 1.0f) - std::max(y0, float(py));
                for (int px = static_cast<int>(x0); px < image.width() && px < x1; px++) {
                    float w = wy * (std::min(x1, px + 1.0f) - std::max(x0, float(px)));
                    Color c = image[py][px];
                    r += c.r * w;
                    g += c.g * w;
                    b += c.b * w;
                    a += c.a * w;
                    total += w;
                }
            }
            result[y][x] = Color(r / total + 0.5f, g / total + 0.5f, b / total + 0.5f, a / total + 0.5f);
        }
    }
    return result;
}

static bool hdEnabled() {
    if (!engine || !dfs)
        return false;
    int scale = engine->config->graphics.HdTextures.value();
    return scale < 0 || scale >= 2;
}

static std::optional<RgbaImage> readHdFile(std::string_view folder, std::string_view name) {
    std::string path = fmt::format("hd/{}/{}.png", folder, ascii::toLower(name));
    if (dfs->exists(path))
        return png::decode(dfs->read(path));
    path = fmt::format("hd/{}/{}.jpg", folder, ascii::toLower(name));
    if (dfs->exists(path))
        return jpeg::decode(dfs->read(path));
    return std::nullopt;
}

int defaultHdScale() {
#ifdef __ANDROID__
    // Phones with less memory keep half the pack's size, the outdoor maps hold a few hundred textures.
    struct sysinfo info = {};
    if (sysinfo(&info) == 0 && static_cast<uint64_t>(info.totalram) * info.mem_unit < 6ull * 1024 * 1024 * 1024)
        return 2;
#endif
    return 4;
}

int spriteScale() {
    int scale = engine ? engine->config->graphics.SpriteScale.value() : 1;
    if (scale == 0) {
        scale = render ? std::min(render->renderScale(), 3) : 1;
#ifdef __ANDROID__
        scale = std::min(scale, 2);  // Scale3x sprites are nine times the texture memory, too much for a phone's GPU.
#endif
    }
    return scale;
}

std::optional<RgbaImage> loadHdBitmap(std::string_view name, int originalWidth, int originalHeight, bool desaturate) {
    if (!hdEnabled() || originalWidth <= 0 || originalHeight <= 0)
        return std::nullopt;
    int scale = engine->config->graphics.HdTextures.value();
    if (scale < 0)
        scale = defaultHdScale();

    std::optional<RgbaImage> hd = readHdFile("bitmaps", name);
    if (!hd)
        return std::nullopt;
    RgbaImage image = std::move(*hd);
    int factor = image.width() / originalWidth;
    if (factor < 1 || image.width() != originalWidth * factor || image.height() != originalHeight * factor) {
        MM_WARNING("HD texture {} is {}x{}, not a multiple of {}x{}", name, image.width(), image.height(), originalWidth, originalHeight);
        return std::nullopt;
    }

    if (scale < factor)
        image = areaResize(image, originalWidth * scale, originalHeight * scale);

    MM_INFO("HD texture {} {}x{}", name, image.width(), image.height());
    if (desaturate) {
        float xs = engine->config->graphics.Saturation.value();
        float xv = engine->config->graphics.Lightness.value();
        for (Color &pixel : image.pixels())
            pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();
    }
    return image;
}

void applyHdIcon(std::string_view name, RgbaImage *image, int *scale) {
    *scale = 1;
    // UI images are drawn at their logical size, more texture pixels than the framebuffer has per logical pixel
    // would only be averaged away.
    int wanted = render ? render->renderScale() : 1;
    if (wanted < 2 || !hdEnabled() || image->width() <= 0 || image->height() <= 0)
        return;
    std::optional<RgbaImage> hd = readHdFile("icons", name);
    if (!hd)
        return;
    int factor = hd->width() / image->width();
    if (factor < 2 || hd->width() != image->width() * factor || hd->height() != image->height() * factor) {
        MM_WARNING("HD icon {} is {}x{}, not a multiple of {}x{}", name, hd->width(), hd->height(), image->width(), image->height());
        return;
    }
    if (wanted < factor) {
        hd = areaResize(*hd, image->width() * wanted, image->height() * wanted);
        factor = wanted;
    }
    for (int y = 0; y < hd->height(); y++) {
        for (int x = 0; x < hd->width(); x++) {
            Color &pixel = (*hd)[y][x];
            uint8_t alpha = (*image)[y / factor][x / factor].a;
            pixel = alpha == 0 ? Color() : Color(pixel.r, pixel.g, pixel.b, alpha);
        }
    }
    *image = std::move(*hd);
    *scale = factor;
}
