#pragma once

#include <string>
#include <functional>

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"

class LodSpriteCache;
class LodTextureCache;
class LodReader;

class ImageLoader {
 public:
    virtual ~ImageLoader() = default;
    const std::string &GetResourceName() const { return this->resource_name; }

    virtual bool Load(RgbaImage *rgbaImage) = 0;
    int scale() const { return _scale; }
    /** Ours: the texture holds palette indices, not colours (sprites), its transparent texels must stay as they are. */
    virtual bool paletted() const { return false; }

 protected:
    std::string resource_name;
    int _scale = 1;  // Ours: texture pixels per logical pixel, 2 for HD replacements and scaled sprites.
};

class Paletted_Img_Loader : public ImageLoader {
 public:
    inline Paletted_Img_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
};

class ColorKey_LOD_Loader : public ImageLoader {
 public:
    inline ColorKey_LOD_Loader(LodTextureCache *lod, std::string_view filename, Color colorkey) {
        this->resource_name = filename;
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    Color colorkey;
    LodTextureCache *lod = nullptr;
};

class Image16bit_LOD_Loader : public ImageLoader {
 public:
    /**
     * @param allowHd                   Ours: whether an HD replacement may be used. Minimaps are read pixel by pixel
     *                                  at their original size, they keep it.
     */
    inline Image16bit_LOD_Loader(LodTextureCache *lod, std::string_view filename, bool allowHd = true) {
        this->resource_name = filename;
        this->lod = lod;
        this->allowHd = allowHd;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
    bool allowHd = true;
};

class Alpha_LOD_Loader : public ImageLoader {
 public:
    inline Alpha_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    inline Buff_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
};

class PCX_Loader : public ImageLoader {
 protected:
    bool InternalLoad(const Blob &data, RgbaImage *rgbaImage);
};

class PCX_LOD_Raw_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Raw_Loader(LodReader *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodReader *lod = nullptr;
};

class PCX_LOD_Compressed_Loader : public PCX_Loader {
 public:
    // TODO(captainurist): this is the next level of ugly, redo.
    template<class Lod>
    inline PCX_LOD_Compressed_Loader(Lod *lod, std::string_view filename, Color colorkey) {
        this->colorkey = colorkey;
        resource_name = filename;
        blob_func = [this, lod] {
            return lod->LoadCompressedTexture(resource_name);
        };
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    std::function<Blob()> blob_func;
    Color colorkey;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    explicit inline Bitmaps_GEN_Loader(std::string_view filename) {
        this->resource_name = filename;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(LodSpriteCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;
    virtual bool paletted() const override { return true; }

 protected:
    LodSpriteCache *lod = nullptr;
};
