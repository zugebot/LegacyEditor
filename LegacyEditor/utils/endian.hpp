#pragma once

#include "processor.hpp"


static bool isSystemLittleEndian() {
    static int num = 1;
    static bool isLittle = *reinterpret_cast<char*>(&num) == 1;
    return isLittle;
}

static u16 swapEndian16(const u16 value) {
    return (value << 8) | (value >> 8);
}

static u32 swapEndian32(const u32 value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >>  8) |
           ((value & 0x0000FF00) <<  8) |
           ((value & 0x000000FF) << 24);
}

// FIXME: this supposedly does not work?
static u64 swapEndian64(u64 value) {
    value = (value & 0x00000000FFFFFFFF) << 32 | (value & 0xFFFFFFFF00000000) >> 32;
    value = (value & 0x0000FFFF0000FFFF) << 16 | (value & 0xFFFF0000FFFF0000) >> 16;
    value = (value & 0x00FF00FF00FF00FF) <<  8 | (value & 0xFF00FF00FF00FF00) >>  8;
    return value;
}