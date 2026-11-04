#pragma once

#include <optional>
#include <string_view>

#include "Library/Image/Image.h"

/** The scale `graphics.hd_textures = -1` resolves to: 4, on Android phones under 6 GB of memory 2. */
int defaultHdScale();

/** The sprite scale `graphics.sprite_scale` resolves to: 1 (off), 2 or 3, by default the renderer's scale. */
int spriteScale();

/**
 * Ours: a replacement for a bitmaps.lod texture from `hd/bitmaps/<name>.png` or `.jpg` in the game data. The pack
 * holds the textures upscaled 4x, `graphics.hd_textures` says how much of that to keep in memory.
 *
 * @param name                          Texture name as in bitmaps.lod, any case.
 * @param originalWidth                 Size of the texture in bitmaps.lod.
 * @param originalHeight
 * @param desaturate                    Apply the same saturation and lightness adjustment as the loaded palettes get.
 * @param[out] scaleOut                 Texture pixels per pixel of the original, if given.
 * @return                              The replacement, or nothing if there is none or it is switched off.
 */
std::optional<RgbaImage> loadHdBitmap(std::string_view name, int originalWidth, int originalHeight, bool desaturate,
                                      int *scaleOut = nullptr);

/**
 * Ours: `hd/icons/<name>.png` or `.jpg` replaces the colours of a UI image at a whole multiple of its size. The
 * transparency stays the original's, upscaled, so colour keys and alpha behave exactly as before.
 *
 * @param name                          Image name as in icons.lod.
 * @param[in,out] image                 The image as its loader made it, replaced on success.
 * @param[out] scale                    Texture pixels per logical pixel afterwards.
 */
void applyHdIcon(std::string_view name, RgbaImage *image, int *scale);
