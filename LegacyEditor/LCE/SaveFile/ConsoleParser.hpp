
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/enums.hpp"

#include "LegacyEditor/LCE/BinFile/FileInfo.hpp"
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

    /// FUNCTIONS

    MU ND int convertTo(const std::string& inFileStr, const std::string& outFileStr, CONSOLE consoleOut);
    MU ND int convertAndReplaceRegions(const std::string& inFileStr, const std::string& inFileRegionReplacementStr,
                                                      const std::string& outFileStr, CONSOLE consoleOut);

    /// READ

    MU ND int readConsoleFile(const std::string& inFileStr);
    MU ND int readWiiU(u32 file_size);
    MU ND int readVita();
    MU ND int readPs3(u32 dest_size);
    MU ND int readRpcs3();
    MU ND int readXbox360_DAT();
    MU ND int readXbox360_BIN();

    /// SAVE

    MU ND int saveConsoleFile(const std::string& outfileStr);
    MU ND static int saveWiiU(const std::string& outfileStr, Data& dataOut);
    MU ND int saveVita(const std::string& outfileStr, Data& dataOut);
    MU ND int savePS3Uncompressed();
    MU ND int savePS3Compressed();
    MU ND int saveXbox360_DAT();
    MU ND int saveXbox360_BIN();
};
