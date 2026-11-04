#pragma once

#include <string>
#include <memory>

#include "Engine/Graphics/Renderer/TextureRenderId.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class ImageLoader;

class GraphicsImage {
 public:
    static GraphicsImage *Create(RgbaImage image);
    /** Ours: an image whose texture has `scale` pixels per logical pixel. */
    static GraphicsImage *Create(RgbaImage image, int scale);
    static GraphicsImage *Create(int width, int height);
    static GraphicsImage *Create(Sizei size);
    static GraphicsImage *Create(std::unique_ptr<ImageLoader> loader);

    int width();
    int height();
    Sizei size();

    RgbaImage &rgba();
    // Texture pixels per logical pixel. Loads the image, the scale is known once the HD replacement is read: the
    // chest asked a not yet loaded exit button and its "Take all" copy came out three times too big on the phone.
    int scale() {
        initialize();
        return _scale;
    }

    const std::string &name();

    void release(); // TODO(captainurist): drop

    [[nodiscard]] TextureRenderId renderId();
    void releaseRenderId();

 private:
    GraphicsImage();
    ~GraphicsImage(); // Call Release() instead.

    bool initialize();

 private:
    bool _initialized = false;
    std::string _name;
    std::unique_ptr<ImageLoader> _loader;
    RgbaImage _rgba;
    TextureRenderId _renderId;
    int _scale = 1;
};

struct GraphicsImageDeleter {
    void operator()(GraphicsImage *image) const {
        image->release();
    }
};

using GraphicsImagePtr = std::unique_ptr<GraphicsImage, GraphicsImageDeleter>;
