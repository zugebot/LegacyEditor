#pragma once

#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/utils/processor.hpp"


namespace editor {
    /**
     * Console File header that holds 12 bytes
     * uses many struct unions to interpret header for different consoles
     */
    class HeaderUnion {
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
                u32 int3;
            } INT_VIEW;
            /// header size: 10 bytes
            struct {
                /// bytes 0-7
                u64 dest_size;
                /// bytes 8-9
                u16 zlib_magic;
            } ZLIB;

        };
        /// bytes 0-3
        ND u32 getInt1() const { return isSystemLittle() ? swapEndian32(INT_VIEW.int1) : INT_VIEW.int1; }
        /// bytes 4-7
        ND u32 getInt2() const { return isSystemLittle() ? swapEndian32(INT_VIEW.int2) : INT_VIEW.int2; }
        /// bytes 8-11
        ND u32 getInt3() const { return isSystemLittle() ? swapEndian32(INT_VIEW.int3) : INT_VIEW.int3; }
        /// bytes 8-9
        ND u32 getShort5() const { return isSystemLittle() ? swapEndian16(ZLIB.zlib_magic) : ZLIB.zlib_magic; }
        /// bytes 0-7
        ND u64 getDestSize() const { return isSystemLittle() ? swapEndian64(ZLIB.dest_size) : ZLIB.dest_size; }
    };

}
