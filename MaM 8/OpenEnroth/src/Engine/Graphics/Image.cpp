#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>
#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/AssetsManager.h"

GraphicsImage::GraphicsImage() = default;
GraphicsImage::~GraphicsImage() = default;

GraphicsImage *GraphicsImage::Create(RgbaImage image) {
    GraphicsImage *result = new GraphicsImage();
    result->_initialized = true;
    result->_rgba = std::move(image);
    result->_renderId = render->CreateTexture(result->_rgba);
    return result;
}

GraphicsImage *GraphicsImage::Create(RgbaImage image, int scale) {
    GraphicsImage *result = Create(std::move(image));
    result->_scale = scale;
    return result;
}

GraphicsImage *GraphicsImage::Create(int width, int height) {
    assert(width > 0 && height > 0);
    return Create(RgbaImage::solid(Color(), width, height));
}

GraphicsImage *GraphicsImage::Create(Sizei size) {
    return Create(size.w, size.h);
}

GraphicsImage *GraphicsImage::Create(std::unique_ptr<ImageLoader> loader) {
    GraphicsImage *result = new GraphicsImage();
    result->_name = loader->GetResourceName();
    result->_loader = std::move(loader);
    return result;
}

int GraphicsImage::width() {
    return rgba().width() / _scale;
}

int GraphicsImage::height() {
    return rgba().height() / _scale;
}

Sizei GraphicsImage::size() {
    return {width(), height()};
}

RgbaImage &GraphicsImage::rgba() {
    initialize();
    return _rgba;
}

const std::string &GraphicsImage::name() {
    return _name;
}

void GraphicsImage::release() {
    if (_loader) {
        if (!assets->releaseSprite(_loader->GetResourceName()))
            if (!assets->releaseImage(_loader->GetResourceName()))
                assets->releaseBitmap(_loader->GetResourceName());
    }

    releaseRenderId();

    delete this;
}

/**
 * Transparent texels are black, linear filtering of a magnified texture mixes that black into the edge of the opaque
 * part and draws a dark outline (2D images on the scaled framebuffer). The uploaded copy gives every transparent
 * texel next to an opaque one the average colour of its opaque neighbours, two texels deep, alpha stays 0.
 */
static RgbaImage bleedTransparentColours(const RgbaImage &image) {
    RgbaImage result = RgbaImage::copy(image);
    int w = result.width(), h = result.height();
    for (int pass = 0; pass < 2; pass++) {
        RgbaImage source = RgbaImage::copy(result);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                if (source[y][x].a != 0 || (source[y][x].r | source[y][x].g | source[y][x].b) != 0)
                    continue;
                int r = 0, g = 0, b = 0, n = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx, ny = y + dy;
                        if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                            continue;
                        const Color &c = source[ny][nx];
                        if (c.a == 0 && pass == 0)
                            continue;
                        if (c.a == 0 && (c.r | c.g | c.b) == 0)
                            continue;
                        r += c.r;
                        g += c.g;
                        b += c.b;
                        n++;
                    }
                }
                if (n)
                    result[y][x] = Color(r / n, g / n, b / n, 0);
            }
        }
    }
    return result;
}

[[nodiscard]] TextureRenderId GraphicsImage::renderId() {
    if (!_renderId) {
        initialize();
        bool hasTransparency = false;
        if (!_loader || !_loader->paletted()) {
            for (const Color &c : _rgba.pixels()) {
                if (c.a == 0) {
                    hasTransparency = true;
                    break;
                }
            }
        }
        _renderId = hasTransparency ? render->CreateTexture(bleedTransparentColours(_rgba)) : render->CreateTexture(_rgba);
    }

    return _renderId;
}

void GraphicsImage::releaseRenderId() {
    if (!_renderId)
        return;

    render->DeleteTexture(_renderId);
    _renderId = TextureRenderId();
}

bool GraphicsImage::initialize() {
    if (_initialized)
        return true;

    assert(_loader);
    _initialized = _loader->Load(&_rgba);
    _scale = _loader->scale();
    // TODO(captainurist): _initialized == false happens, investigate

    return _initialized;
}
