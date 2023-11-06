#pragma once

#include "LegacyEditor/utils/endian_swap.hpp"
#include "LegacyEditor/utils/processor.hpp"

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
        struct {
            uint32_t garbo;
            uint32_t file_size;
            uint32_t file_listing_offset;
        } VITA;
        struct {
            uint32_t file_size;
            uint32_t something;
        } SWITCH;
    };

    ND u32 getInt1() const { return ::isLittleEndian() ? ::swapEndian32(INT_VIEW.int1) : INT_VIEW.int1; }
    ND u32 getInt2() const { return ::isLittleEndian() ? ::swapEndian32(INT_VIEW.int2) : INT_VIEW.int2; }
    ND u64 getDestSize() const { return ::isLittleEndian() ? ::swapEndian64(ZLIB.dest_size) : ZLIB.dest_size; }
    ND u16 getZlibMagic() const { return ::isLittleEndian() ? ::swapEndian16(ZLIB.zlib_magic) : ZLIB.zlib_magic; }
    ND u32 getSrcSize() const { return ::isLittleEndian() ? ::swapEndian32(DAT.src_size) : DAT.src_size; }
    ND u64 getFileSize() const { return ::isLittleEndian() ? ::swapEndian64(DAT.file_size) : DAT.file_size; }

    ND u32 getVitaFileSize() const { return isLittleEndian() ? VITA.file_size : swapEndian32(VITA.file_size) ; }
    ND u32 getVitaFileListing() const { return isLittleEndian() ? VITA.file_listing_offset : swapEndian32(VITA.file_listing_offset); }

    ND u32 getSwitchFileSize() const { return isLittleEndian() ? SWITCH.file_size : swapEndian32(SWITCH.file_size) ; }
    ND u32 getSwitchSomething() const { return isLittleEndian() ? SWITCH.something : swapEndian32(SWITCH.something); }


};
