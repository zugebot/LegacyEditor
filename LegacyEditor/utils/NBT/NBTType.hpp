#pragma once

#include "LegacyEditor/utils/processor.hpp"


enum NBTType : u8 {
    NBT_NONE = 0,
    NBT_INT8 = 1,
    NBT_INT16 = 2,
    NBT_INT32 = 3,
    NBT_INT64 = 4,
    NBT_FLOAT = 5,
    NBT_DOUBLE = 6,
    TAG_BYTE_ARRAY = 7,
    TAG_STRING = 8,
    TAG_LIST = 9,
    TAG_COMPOUND = 10,
    TAG_INT_ARRAY = 11,
    TAG_LONG_ARRAY = 12,
    TAG_PRIMITIVE = 99
};
