#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Utility/Memory/Blob.h"

/**
 * Read-only access to the game tables that MM6 keeps inside MM6.exe, e.g. house movies or class skills. The tables
 * are read from the player's own MM6.exe, addresses are those of the GOG release (version 1.1, same as GrayFace's
 * MMExtension uses). `mm8ExeData` reads MM8-Rel.exe of the GOG release the same way.
 */
class Mm6ExeData {
 public:
    /**
     * Loads MM6.exe or MM8-Rel.exe from the game data folder.
     *
     * @param mm8                       Whether to load MM8-Rel.exe.
     * @return                          Whether the file was found and looks like the supported version.
     */
    bool load(bool mm8 = false);

    [[nodiscard]] bool isLoaded() const { return _loaded; }

    /**
     * @param address                   Virtual address in MM6.exe.
     * @param size                      Number of bytes to read.
     * @return                          Bytes at the given address, empty if the range is outside of the file.
     */
    [[nodiscard]] std::vector<uint8_t> bytes(uint32_t address, size_t size) const;

    [[nodiscard]] uint32_t u32(uint32_t address) const;

    /**
     * @param address                   Virtual address of a zero-terminated string.
     * @return                          The string, empty if the address is outside of the file.
     */
    [[nodiscard]] std::string string(uint32_t address) const;

 private:
    [[nodiscard]] int64_t fileOffset(uint32_t address) const;

    struct Section {
        uint32_t virtualAddress;
        uint32_t rawSize;
        uint32_t rawOffset;
    };

    bool _loaded = false;
    Blob _exe;
    uint32_t _imageBase = 0;
    std::vector<Section> _sections;
};

extern Mm6ExeData mm6ExeData;
extern Mm6ExeData mm8ExeData;
