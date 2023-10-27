#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <zconf.h>

#include "LegacyEditor/LCE/BINSupport.hpp"
#include "LegacyEditor/LCE/FileInfo.hpp"
#include "LegacyEditor/LCE/fileListing.hpp"
#include "LegacyEditor/utils/endian_swap.hpp"


class HeaderUnion {
public:
    union {
        struct {
            u32 int1;
            u32 int2;
        } INT_VIEW;
        struct {
            u64 dest_size;
            u16 zlib_magic;
        } ZLIB;
        struct {
            u32 src_size;
            u64 file_size;
        } DAT;
    };

    ND u32 getInt1() const { return ::isLittleEndian() ? ::swapEndian32(INT_VIEW.int1) : INT_VIEW.int1; }
    ND u32 getInt2() const { return ::isLittleEndian() ? ::swapEndian32(INT_VIEW.int2) : INT_VIEW.int2; }
    ND u64 getDestSize() const { return ::isLittleEndian() ? ::swapEndian64(ZLIB.dest_size) : ZLIB.dest_size; }
    ND u16 getZlibMagic() const { return ::isLittleEndian() ? ::swapEndian16(ZLIB.zlib_magic) : ZLIB.zlib_magic; }
    ND u32 getSrcSize() const { return ::isLittleEndian() ? ::swapEndian32(DAT.src_size) : DAT.src_size; }
    ND u64 getFileSize() const { return ::isLittleEndian() ? ::swapEndian64(DAT.file_size) : DAT.file_size; }
};


class ConsoleFileParser : public Data {
private:
    static constexpr int CON_MAGIC = 0x434F4E20;
    static constexpr int ZLIB_MAGIC = 0x789C;

    const char* error1 = "Could not allocate %d bytes of data for source file buffer, exiting\n";
    const char* error2 = "Could not allocate %d bytes of data for source and decompressed buffer, exiting\n";
    const char* error3 = "Not a Minecraft console savefile, exiting\n";

    HeaderUnion headerUnion{};

public:
    FileInfo saveGameInfo;
    uint8_t header[0xc]{};
    CONSOLE console = CONSOLE::NONE;
    FILE* f_in{};
    uint64_t source_binary_size{};

    ConsoleFileParser() : Data() {}

    int loadWiiU(u32 file_size);
    int loadPs3Compressed(u32 dest_size);
    int loadPs3Uncompressed();
    int loadXbox360_DAT();
    int loadXbox360_BIN();

    int loadConsoleFile(const char* infileStr);

    int saveWiiU(const std::string& outfileStr, DataOutManager& outManager);
    int savePS3Uncompressed();
    int savePS3Compressed();
    int saveXbox360_DAT();
    int saveXbox360_BIN();
    int saveConsoleFile(const char* outfileStr);
};
