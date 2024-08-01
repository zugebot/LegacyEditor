#include "fileListing.hpp"

#include <cstdio>

#include "include/ghc/fs_std.hpp"


#include "lce/processor.hpp"

#include "LegacyEditor/utils/NBT.hpp"

#include "LegacyEditor/code/ConsoleParser/headerUnion.hpp"
#include "LegacyEditor/code/ConsoleParser/include.hpp"




namespace editor {


    FileListing::FileListing() {
        consoleInstances.emplace(lce::CONSOLE::XBOX360, std::make_unique<Xbox360DAT>());
        consoleInstances.emplace(lce::CONSOLE::PS3, std::make_unique<PS3>());
        consoleInstances.emplace(lce::CONSOLE::RPCS3, std::make_unique<RPCS3>());
        consoleInstances.emplace(lce::CONSOLE::PS4, std::make_unique<PS4>());
        consoleInstances.emplace(lce::CONSOLE::VITA, std::make_unique<Vita>());
        consoleInstances.emplace(lce::CONSOLE::WIIU, std::make_unique<WiiU>());
        consoleInstances.emplace(lce::CONSOLE::SWITCH, std::make_unique<Switch>());
        consoleInstances.emplace(lce::CONSOLE::XBOX1, std::make_unique<Xbox1>());
    }


    FileListing::~FileListing() {
        deallocate();
    }


    int FileListing::read(const fs::path& inFilePath) {
        myFilePath = inFilePath;

        i32 status1 = findConsole(myFilePath);
        if (status1 != SUCCESS) {
            printf("Failed to find console from %s\n", inFilePath.string().c_str());
            return status1;
        }

        i32 status2 = readSave();
        if (status2 != SUCCESS) {
            printf("Failed to read save from %s\n", inFilePath.string().c_str());
            return status2;
        }

        return SUCCESS;
    }


    int FileListing::readSave() {
        auto it = consoleInstances.find(myConsole);
        if (it != consoleInstances.end()) {
            int status = it->second->read(this, myFilePath);
            printf("detected save as %s\n", lce::consoleToStr(myConsole).c_str());
            return status;
        }
        return INVALID_CONSOLE;
    }


    int FileListing::writeSave(const ConvSettings& theSettings) {
        auto it = consoleInstances.find(theSettings.getConsole());
        if (it != consoleInstances.end()) {
            int status = it->second->write(this, theSettings.getFilePath());
            if (status != 0) {
                printf("failed to write save %s.\n", theSettings.getFilePath().string().c_str());
            }
            return status;
        }
        return INVALID_CONSOLE;
    }


    int FileListing::write(const ConvSettings& theSettings) {
        if (!theSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        // TODO: create default output file path if not set

        if (myConsole != theSettings.getConsole()) {
            if (AUTO_REMOVE_PLAYERS) {
                removeFileTypes({lce::FILETYPE::PLAYER});
            }
            if (AUTO_REMOVE_DATA_MAPPING) {
                removeFileTypes({lce::FILETYPE::DATA_MAPPING});
            }
            removeFileTypes({lce::FILETYPE::GRF});
        }

        convertRegions(theSettings.getConsole());

        int status = writeSave(theSettings);
        if (status != 0) {
            printf("failed to write gamedata to %s", theSettings.getFilePath().string().c_str());
        }
        return status;
    }


    int FileListing::findConsole(const fs::path& inFilePath) {
        static constexpr uint32_t CON_MAGIC = 0x434F4E20;
        static constexpr uint32_t ZLIB_MAGIC = 0x789C;


        FILE *f_in = fopen(inFilePath.string().c_str(), "rb");
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

        Data data;
        data.setScopeDealloc(true);
        if (headerUnion.getInt1() <= 2) {
            if (headerUnion.getShort5() == ZLIB_MAGIC) {
                if (headerUnion.getInt2Swap() >= headerUnion.getDestSize()) {
                    myConsole = lce::CONSOLE::WIIU;
                } else {
                    const std::string parentDir = myFilePath.parent_path().filename().string();
                    myConsole = lce::CONSOLE::SWITCH;
                    if (parentDir == "savedata0") {
                        myConsole = lce::CONSOLE::PS4;
                    }
                }
            } else {
                // TODO: change this to write custom checker for FILE_COUNT * 144 == diff. with
                // TODO: with custom vitaRLE decompress checker
                c_u32 indexFromSF = headerUnion.getInt2Swap() - headerUnion.getInt3Swap();
                if (indexFromSF > 0 && indexFromSF < 65536) {
                    myConsole = lce::CONSOLE::VITA;
                } else { // compressed ps3
                    myConsole = lce::CONSOLE::PS3;
                }
            }
        } else if (headerUnion.getInt2() <= 2) {
            /// if (int2 == 0) it is an xbox savefile unless it's a massive
            /// file, but there won't be 2 files in a savegame file for PS3
            myConsole = lce::CONSOLE::XBOX360;
            // TODO: don't use arbitrary guess for a value
        } else if (headerUnion.getInt2() < 100) { // uncompressed PS3 / RPCS3
            /// otherwise if (int2) > 100 then it is a random file
            /// because likely ps3 won't have more than 100 files
            myConsole = lce::CONSOLE::RPCS3;
        } else if (headerUnion.getInt1() == CON_MAGIC) {
            myConsole = lce::CONSOLE::XBOX360;
        } else {
            return printf_err(INVALID_SAVE, ERROR_3);
        }

        return SUCCESS;
    }


}