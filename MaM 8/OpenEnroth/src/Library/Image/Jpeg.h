#pragma once

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace jpeg {
/** Ours: decodes a baseline or progressive JPEG into an opaque RGBA image. Throws on a broken file. */
RgbaImage decode(const Blob &data);
bool detect(const Blob &data);
} // namespace jpeg
