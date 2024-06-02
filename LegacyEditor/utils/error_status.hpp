#pragma once

#include "LegacyEditor/utils/processor.hpp"
#include "lce/enums.hpp"

#include <algorithm>
#include <cctype>


enum STATUS : i8 {
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