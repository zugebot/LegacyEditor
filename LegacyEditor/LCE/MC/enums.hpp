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
    PS4 = 6,
    XBOX1 = 7,
};


enum class DIM : i8 {
    NETHER = -1,
    OVERWORLD = 0,
    END = 1,
    NONE = 2
};


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


/**
 * \brief Used for extracting the correct dimension
 * from the GAMEDATA_0000000 filenames.
 * \param number
 * \return
 */
static DIM gamedataIntToDim(const char number) {
    switch(number) {
        case 0:
            return DIM::NETHER;
        case 1:
            return DIM::OVERWORLD;
        case 2:
            return DIM::END;
        default:
            return DIM::NONE;
    }
}


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
        case CONSOLE::PS4:
            return "ps4";
        case CONSOLE::XBOX1:
            return "xbox1";
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
        case CONSOLE::PS4:
        case CONSOLE::SWITCH:
            return false;
    }
}
