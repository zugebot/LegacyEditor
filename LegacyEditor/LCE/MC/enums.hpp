#pragma once

#include "LegacyEditor/utils/processor.hpp"


enum class CONSOLE : i8 {
    NONE = -1,
    XBOX360 = 0,
    PS3 = 1,
    WIIU = 2,
    VITA = 3,
    RPCS3 = 4,
    SWITCH = 5,
};


enum class DIM : i8 {
    NETHER = -1,
    OVERWORLD = 0,
    END = 1,
};


enum STATUS : i8 {
    SUCCESS = 0,
    COMPRESS = -1,
    DECOMPRESS = -2,
    MALLOC_FAILED = -3,
    INVALID_SAVE = -4,
    FILE_NOT_FOUND = -5,
    INVALID_CONSOLE = -6
};


static std::string consoleToStr(const CONSOLE console) {
    switch (console) {
        case CONSOLE::XBOX360:
            return "xbox360";
        case CONSOLE::PS3:
            return "ps3";
        case CONSOLE::RPCS3:
            return "rpcs3";
        case CONSOLE::WIIU:
            return "wiiu";
        case CONSOLE::VITA:
            return "vita";
        case CONSOLE::SWITCH:
            return "switch";
        case CONSOLE::NONE:
        default:
            return "NONE";
    }
}


static bool consoleIsBigEndian(const CONSOLE console) {
    switch (console) {
        case CONSOLE::NONE:
        case CONSOLE::XBOX360:
        case CONSOLE::PS3:
        case CONSOLE::RPCS3:
        case CONSOLE::WIIU:
        default:
            return true;
        case CONSOLE::VITA:
        case CONSOLE::SWITCH:
            return false;
    }
}
