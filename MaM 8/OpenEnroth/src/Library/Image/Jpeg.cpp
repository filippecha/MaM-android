#include "Jpeg.h"

#include <cstdint>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#include <stb_image.h> // NOLINT: not a C system header.

#include "Utility/Exception.h"

RgbaImage jpeg::decode(const Blob &data) {
    int width = 0, height = 0, channels = 0;
    stbi_uc *pixels = stbi_load_from_memory(static_cast<const stbi_uc *>(data.data()), static_cast<int>(data.size()),
                                            &width, &height, &channels, 4);
    if (!pixels)
        throw Exception("Could not decode JPEG '{}': {}", data.displayPath(), stbi_failure_reason());

    RgbaImage result = RgbaImage::uninitialized(width, height);
    std::memcpy(result.pixels().data(), pixels, static_cast<size_t>(width) * height * 4);
    stbi_image_free(pixels);
    return result;
}

bool jpeg::detect(const Blob &data) {
    if (data.size() < 3)
        return false;
    const uint8_t *bytes = static_cast<const uint8_t *>(data.data());
    return bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF;
}
