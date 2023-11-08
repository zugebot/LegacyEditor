#pragma once

#include "processor.hpp"


enum class CONSOLE : i8 {
    NONE = -1,
    XBOX360 = 0,
    PS3 = 1,
    WIIU = 2,
    VITA = 3
};


enum class DIM : i8 {
    NETHER = -1,
    HELL = -1,
    OVERWORLD = 0,
    END = 1,
};
