#pragma once

#include "processor.hpp"


enum class CONSOLE : i8 {
    NONE = -1,
    XBOX360 = 0,
    PS3 = 1,
    WIIU = 2,
    VITA = 3,
    RPCS3 = 4
};


enum class DIM : i8 {
    NETHER = -1,
    OVERWORLD = 0,
    END = 1,
};


static std::string consoleToStr(CONSOLE console) {
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
        case CONSOLE::NONE:
        default:
            return "NONE";
    }
}