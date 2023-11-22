#pragma once

#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/utils/processor.hpp"

/**
 * Console File header that holds 12 bytes
 * uses many struct unions to interpret header for different consoles
 */
 class HeaderUnion {
 private:
     static bool isSystemLittle() {
         return isSystemLittleEndian();
     }
public:
    union {
        /// header size: 12 bytes
        struct {
            /// bytes 0-3
            u32 int1;
            /// bytes 4-7
            u32 int2;
            /// bytes 8-11
        } INT_VIEW;
        /// header size: 10 bytes
        struct {
            /// bytes 0-7
            u64 dest_size;
            /// bytes 8-9
            u16 zlib_magic;
        } ZLIB;
        /// header size: 12 bytes
        struct {
            /// bytes 0-3
            u32 src_size;
            /// bytes 4-11
            u64 file_size;
        } DAT;
        /// header size: 8 bytes
        struct {
            /// bytes 0-3
            uint32_t garbo;
            /// bytes 4-7
            uint32_t file_size;
            /// bytes 8-11
            uint32_t file_listing_offset;
        } VITA;
        /// header size: 8 bytes
        struct {
            /// bytes 0-3
            uint32_t file_size;
            /// bytes 4-7
            uint32_t something;
        } SWITCH;
    };
     /// bytes 0-3
    ND u32 getInt1() const { return isSystemLittle() ? ::swapEndian32(INT_VIEW.int1) : INT_VIEW.int1; }
     /// bytes 4-7
    ND u32 getInt2() const { return isSystemLittle() ? ::swapEndian32(INT_VIEW.int2) : INT_VIEW.int2; }
    /// bytes 0-7
    ND u64 getDestSize() const { return isSystemLittle() ? ::swapEndian64(ZLIB.dest_size) : ZLIB.dest_size; }
    /// bytes 8-9
    ND u16 getZlibMagic() const { return isSystemLittle() ? ::swapEndian16(ZLIB.zlib_magic) : ZLIB.zlib_magic; }
    /// bytes 0-3
    ND u32 getSrcSize() const { return isSystemLittle() ? ::swapEndian32(DAT.src_size) : DAT.src_size; }
    /// bytes 4-11
    ND u64 getFileSize() const { return isSystemLittle() ? ::swapEndian64(DAT.file_size) : DAT.file_size; }
    /// bytes 4-7
    ND u32 getVitaFileSize() const { return isSystemLittle() ? VITA.file_size : ::swapEndian32(VITA.file_size) ; }
    /// bytes 8-11
    ND u32 getVitaFileListing() const { return isSystemLittle() ? VITA.file_listing_offset : ::swapEndian32(VITA.file_listing_offset); }
    /// bytes 0-3
    ND u32 getSwitchFileSize() const { return isSystemLittle() ? SWITCH.file_size : ::swapEndian32(SWITCH.file_size) ; }
    /// bytes 4-7
    ND u32 getSwitchSomething() const { return isSystemLittle() ? SWITCH.something : ::swapEndian32(SWITCH.something); }


};
