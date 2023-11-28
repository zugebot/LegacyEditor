#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "LegacyEditor/LCE/BinFile/FileInfo.hpp"
#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "headerUnion.hpp"


class DataManager;


class ConsoleParser : public Data {
private:
    static constexpr u32 CON_MAGIC = 0x434F4E20;
    static constexpr u32 ZLIB_MAGIC = 0x789C;
    MU static constexpr char error1[69] = "Could not allocate %d bytes of data for source file buffer, exiting\n";
    MU static constexpr char error2[81] = "Could not allocate %d bytes of data for source and decompressed buffer, exiting\n";
    static constexpr char error3[43] = "Not a Minecraft console savefile, exiting\n";

    HeaderUnion headerUnion{};
    FILE* f_in{};

public:
    FileInfo saveGameInfo;
    u64 source_binary_size{};
    u8 header[0xc]{};
    CONSOLE console = CONSOLE::NONE;

    ConsoleParser() : Data() {}
    ~ConsoleParser() { delete[] data; }

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

    MU ND static int saveConsoleFile(const std::string& outfileStr);
    MU ND static int saveWiiU(const std::string& outfileStr, Data& dataOut);
    // TODO: make this static
    MU ND int saveVita(const std::string& outfileStr, Data& dataOut);
    MU ND static int savePS3Uncompressed();
    MU ND static int savePS3Compressed();
    MU ND static int saveXbox360_DAT();
    MU ND static int saveXbox360_BIN();
};
