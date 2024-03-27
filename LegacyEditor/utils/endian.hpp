#pragma once

#include "processor.hpp"


static bool isSystemLittleEndian() {
    static constexpr int num = 1;
    static const bool isLittle = *reinterpret_cast<const char*>(&num) == 1;
    return isLittle;
}

static u16 swapEndian16(const u16 value) {
    return value << 8 | value >> 8;
}

static u32 swapEndian32(const u32 value) {
    return (value & 0xFF000000U) >> 24 |
           (value & 0x00FF0000U) >>  8 |
           (value & 0x0000FF00U) <<  8 |
           (value & 0x000000FFU) << 24;
}

// FIXME: this supposedly does not work?
static u64 swapEndian64(u64 value) {
    value = (value & 0x00000000FFFFFFFFLL) << 32 | (value & 0xFFFFFFFF00000000LL) >> 32;
    value = (value & 0x0000FFFF0000FFFFLL) << 16 | (value & 0xFFFF0000FFFF0000LL) >> 16;
    value = (value & 0x00FF00FF00FF00FFLL) <<  8 | (value & 0xFF00FF00FF00FF00LL) >>  8;
    return value;
}