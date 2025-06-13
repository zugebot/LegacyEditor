#pragma once

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "code/ConsoleParser/ConsoleParser.hpp"
#include "code/ConsoleParser/headerUnion.hpp"
#include "code/SaveFile/stateSettings.hpp"

#include "common/error_status.hpp"
#include "lce/enums.hpp"

#include "code/ConsoleParser/consoles/PS3.hpp"
#include "code/ConsoleParser/consoles/PS4.hpp"
#include "code/ConsoleParser/consoles/Rpcs3.hpp"
#include "code/ConsoleParser/consoles/Switch.hpp"
#include "code/ConsoleParser/consoles/Vita.hpp"
#include "code/ConsoleParser/consoles/WiiU.hpp"
#include "code/ConsoleParser/consoles/Xbox1.hpp"
#include "code/ConsoleParser/consoles/Xbox360BIN.hpp"
#include "code/ConsoleParser/consoles/Xbox360DAT.hpp"
#include "code/ConsoleParser/consoles/Windurango.hpp"

namespace editor {


    static std::unique_ptr<ConsoleParser> makeParserForConsole(lce::CONSOLE console, bool isXbox360Bin=false) {
        switch (console) {
            case lce::CONSOLE::NONE:
                return nullptr;
            case lce::CONSOLE::XBOX360:
                if (isXbox360Bin) {
                    return std::make_unique<Xbox360BIN>();
                } else {
                    return std::make_unique<Xbox360DAT>();
                }
            case lce::CONSOLE::XBOX1:
                return std::make_unique<Xbox1>();
            case lce::CONSOLE::PS3:
                return std::make_unique<PS3>();
            case lce::CONSOLE::RPCS3:
                return std::make_unique<RPCS3>();
            case lce::CONSOLE::VITA:
                return std::make_unique<Vita>();
            case lce::CONSOLE::PS4:
                return std::make_unique<PS4>();
            case lce::CONSOLE::WIIU:
                return std::make_unique<WiiU>();
            case lce::CONSOLE::SWITCH:
                return std::make_unique<Switch>();
            case lce::CONSOLE::WINDURANGO:
                return std::make_unique<Windurango>();
            default:
                return nullptr;
        }
    }



}