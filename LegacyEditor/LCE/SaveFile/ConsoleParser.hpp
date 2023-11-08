#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <zconf.h>

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/enums.hpp"

#include "FileInfo.hpp"
#include "headerUnion.hpp"


class ConsoleParser : public Data {
private:
    static constexpr u32 CON_MAGIC = 0x434F4E20;
    static constexpr u32 ZLIB_MAGIC = 0x789C;

    const char* error1 = "Could not allocate %d bytes of data for source file buffer, exiting\n";
    const char* error2 = "Could not allocate %d bytes of data for source and decompressed buffer, exiting\n";
    const char* error3 = "Not a Minecraft console savefile, exiting\n";

    HeaderUnion headerUnion{};

public:
    FileInfo saveGameInfo;
    u8 header[0xc]{};
    CONSOLE console = CONSOLE::NONE;
    FILE* f_in{};
    u64 source_binary_size{};

    ConsoleParser() : Data() {}

    ~ConsoleParser() {
        delete[] data;
    }

    /// READ

    int readConsoleFile(const std::string& inFileStr);
    int readWiiU(u32 file_size);
    int readVita();
    int readPs3Compressed(u32 dest_size);
    int readPs3Uncompressed();
    int readXbox360_DAT();
    int readXbox360_BIN();

    /// SAVE

    int saveConsoleFile(std::string outfileStr);
    int saveWiiU(const std::string& outfileStr, Data& dataOut);
    int saveVita(const std::string& outfileStr, Data& dataOut);
    int savePS3Uncompressed();
    int savePS3Compressed();
    int saveXbox360_DAT();
    int saveXbox360_BIN();
};
