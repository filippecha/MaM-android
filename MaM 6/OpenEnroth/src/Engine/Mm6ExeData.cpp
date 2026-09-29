#include "Mm6ExeData.h"

#include <cstring>
#include <string>
#include <vector>

#include "Engine/Resources/EngineFileSystem.h"

#include "Library/Logger/Logger.h"

Mm6ExeData mm6ExeData;

template<class T>
static T readAt(const Blob &blob, size_t offset) {
    T result{};
    if (offset + sizeof(T) <= blob.size())
        memcpy(&result, static_cast<const char *>(blob.data()) + offset, sizeof(T));
    return result;
}

bool Mm6ExeData::load() {
    _loaded = false;
    if (!dfs->exists("mm6.exe")) {
        MM_WARNING("MM6.exe not found, tables stored in the executable are not available");
        return false;
    }
    _exe = dfs->read("mm6.exe");

    uint32_t pe = readAt<uint32_t>(_exe, 0x3C);
    uint16_t sectionCount = readAt<uint16_t>(_exe, pe + 6);
    uint16_t optionalHeaderSize = readAt<uint16_t>(_exe, pe + 20);
    _imageBase = readAt<uint32_t>(_exe, pe + 24 + 28);

    _sections.clear();
    size_t sectionHeader = pe + 24 + optionalHeaderSize;
    for (int i = 0; i < sectionCount; i++, sectionHeader += 40) {
        Section section;
        section.virtualAddress = readAt<uint32_t>(_exe, sectionHeader + 12);
        section.rawSize = readAt<uint32_t>(_exe, sectionHeader + 16);
        section.rawOffset = readAt<uint32_t>(_exe, sectionHeader + 20);
        _sections.push_back(section);
    }

    _loaded = true;

    // House movie #1 is the Castle Ironfist blacksmith in every MM6 1.1 build.
    if (string(u32(0x4BE888 + 16)) != "blcksrch") {
        MM_WARNING("Unsupported MM6.exe version, tables stored in the executable are not available");
        _loaded = false;
    }
    return _loaded;
}

int64_t Mm6ExeData::fileOffset(uint32_t address) const {
    uint32_t rva = address - _imageBase;
    for (const Section &section : _sections)
        if (rva >= section.virtualAddress && rva < section.virtualAddress + section.rawSize)
            return static_cast<int64_t>(rva - section.virtualAddress) + section.rawOffset;
    return -1;
}

std::vector<uint8_t> Mm6ExeData::bytes(uint32_t address, size_t size) const {
    int64_t offset = fileOffset(address);
    if (offset < 0 || offset + size > _exe.size())
        return {};
    const uint8_t *data = static_cast<const uint8_t *>(_exe.data()) + offset;
    return std::vector<uint8_t>(data, data + size);
}

uint32_t Mm6ExeData::u32(uint32_t address) const {
    std::vector<uint8_t> data = bytes(address, 4);
    uint32_t result = 0;
    if (data.size() == 4)
        memcpy(&result, data.data(), 4);
    return result;
}

std::string Mm6ExeData::string(uint32_t address) const {
    int64_t offset = fileOffset(address);
    if (offset < 0)
        return {};
    const char *begin = static_cast<const char *>(_exe.data()) + offset;
    size_t maxLength = _exe.size() - offset;
    return std::string(begin, strnlen(begin, maxLength));
}
