#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>


enum STATUS : int8_t {
    SUCCESS = 0,
    COMPRESS = -1,
    DECOMPRESS = -2,
    MALLOC_FAILED = -3,
    INVALID_SAVE = -4,
    FILE_ERROR = -5,
    INVALID_CONSOLE = -6,
    INVALID_ARGUMENT = -7,
    NOT_IMPLEMENTED = -8,
};