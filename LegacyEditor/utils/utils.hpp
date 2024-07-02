#pragma once

#include <string>
#include <vector>
#include <cstdint>


std::vector<std::string> split(const std::string &s, char delimiter);

std::wstring stringToWstring(const std::string& str);

static bool isSystemLittleEndian() {
    static constexpr int num = 1;
    static const bool isLittle = *reinterpret_cast<const char*>(&num) == 1;
    return isLittle;
}

static uint16_t swapEndian16(const uint16_t value) {
    return value << 8 | value >> 8;
}

static uint32_t swapEndian32(const uint32_t value) {
    return (value & 0xFF000000U) >> 24 |
           (value & 0x00FF0000U) >>  8 |
           (value & 0x0000FF00U) <<  8 |
           (value & 0x000000FFU) << 24;
}

// FIXME: this supposedly does not work?
static uint64_t swapEndian64(uint64_t value) {
    value = (value & 0x00000000FFFFFFFFLL) << 32 | (value & 0xFFFFFFFF00000000LL) >> 32;
    value = (value & 0x0000FFFF0000FFFFLL) << 16 | (value & 0xFFFF0000FFFF0000LL) >> 16;
    value = (value & 0x00FF00FF00FF00FFLL) <<  8 | (value & 0xFF00FF00FF00FF00LL) >>  8;
    return value;
}