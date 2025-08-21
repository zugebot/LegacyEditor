#pragma once

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "headerUnion.hpp"

#include "common/data/buffer.hpp"
#include "common/error_status.hpp"
#include "lce/enums.hpp"
#include "sfo/sfo.hpp"


namespace editor {
    
    static int detectConsole(const fs::path& inFilePath, StateSettings& stateSettings) {
        static constexpr u32 CON_MAGIC = 0x434F4E20;
        static constexpr u32 ZLIB_MAGIC = 0x789C;

        FILE* f_in = fopen(inFilePath.string().c_str(), "rb");
        if (f_in == nullptr) {
            return printf_err(FILE_ERROR, ERROR_4, inFilePath.string().c_str());
        }
    
        fseek(f_in, 0, SEEK_END);
        c_u64 input_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);
        if (input_size < 12) {
            return printf_err(FILE_ERROR, ERROR_5);
        }
        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, 12, f_in);
        fclose(f_in);
    
        Buffer data;
        if (headerUnion.getInt1() <= 2) {
            if (headerUnion.getShort5() == ZLIB_MAGIC) {
                if (headerUnion.getInt2Swap() >= headerUnion.getDestSize()) {
                    stateSettings.setConsole(lce::CONSOLE::WIIU);
                } else {
                    const std::string parentDir = stateSettings.filePath().parent_path().filename().string();
                    stateSettings.setConsole(lce::CONSOLE::SWITCH);
                    // TODO: this code sucks
                    if (parentDir == "savedata0") {
                        stateSettings.setConsole(lce::CONSOLE::PS4);
                    }
                    if (fs::exists(stateSettings.filePath().parent_path() / "wd_displayname.txt")) {
                        stateSettings.setConsole(lce::CONSOLE::WINDURANGO);
                    }
                }
            } else {
                // TODO: change this to write custom checker for FILE_COUNT * 144 == diff. with
                // TODO: with custom vitaRLE decompress checker
                c_u32 indexFromSF = headerUnion.getInt2Swap() - headerUnion.getInt3Swap();
                if (indexFromSF > 0 && indexFromSF < 65536) {
                    stateSettings.setConsole(lce::CONSOLE::VITA);
                } else { // compressed ps3
                    stateSettings.setConsole(lce::CONSOLE::PS3);
                }
            }
        } else if (headerUnion.getInt2() <= 2) {
            /// if (int2 == 0) it is an xbox savefile unless it's a massive
            /// file, but there won't be 2 files in a savegame file for PS3
            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(false);
            // TODO: don't use arbitrary guess for a value
        } else if (headerUnion.getInt1() == CON_MAGIC) {
            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(true);
        } else if (headerUnion.getInt2() % 136 == 0) {
            // This is here as a gag, but it will work!
            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(false);
            stateSettings.setCompressed(false);
        } else if (headerUnion.getInt2() < 1000) { // uncompressed PS3 / RPCS3
            if (fs::path sfoPath = inFilePath.parent_path() / "param.sfo";
                fs::exists(sfoPath)) {
                SFOManager mainSFO(sfoPath.string());
                auto testAttr = mainSFO.getAttribute("RPCS3_BLIST");
                if (testAttr) {
                    stateSettings.setConsole(lce::CONSOLE::RPCS3);
                } else {
                    stateSettings.setConsole(lce::CONSOLE::PS3);
                    stateSettings.setCompressed(false);
                }
            } else {
                // fallback, because RPCS3 saves are easier to use than PS3, could be config?
                stateSettings.setConsole(lce::CONSOLE::RPCS3);
            }

        } else {
            return printf_err(INVALID_SAVE, ERROR_3);
        }

        // Lastly, check for other files existence to differentiate
        if (stateSettings.console() == lce::CONSOLE::PS4
            || stateSettings.console() == lce::CONSOLE::SWITCH) {
            if (fs::exists(inFilePath.parent_path() / "sce_sys" / "param.sfo")) {
                stateSettings.setConsole(lce::CONSOLE::PS4);
            } else if (fs::path temp = inFilePath;
                fs::exists(temp.replace_extension(".sub"))) {
                stateSettings.setConsole(lce::CONSOLE::SWITCH);
            }
            if (fs::exists(stateSettings.filePath().parent_path() / "wd_displayname.txt")) {
                stateSettings.setConsole(lce::CONSOLE::WINDURANGO);
            }
        }
    
        return SUCCESS;
    }
}
