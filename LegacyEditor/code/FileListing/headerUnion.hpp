#pragma once

#include "lce/processor.hpp"

#include "LegacyEditor/utils/utils.hpp"


namespace editor {
    /**
     * Console File header that holds 12 bytes
     * uses many struct unions to interpret header for different consoles
     */
    class HeaderUnion {

        static bool isSystemLittle() {
            return isSystemLittleEndian();
        }

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

        } UNION;
    public:
        /// bytes 0-3
        ND u32 getInt1() const { return isSystemLittle() ? swapEndian32(UNION.INT_VIEW.int1) : UNION.INT_VIEW.int1; }
        /// bytes 4-7
        ND u32 getInt2() const { return isSystemLittle() ? swapEndian32(UNION.INT_VIEW.int2) : UNION.INT_VIEW.int2; }
        /// bytes 8-11
        ND u32 getInt3() const { return isSystemLittle() ? swapEndian32(UNION.INT_VIEW.int3) : UNION.INT_VIEW.int3; }
        /// bytes 8-9
        ND u32 getShort5() const { return isSystemLittle() ? swapEndian16(UNION.ZLIB.zlib_magic) : UNION.ZLIB.zlib_magic; }
        /// bytes 0-7
        ND u64 getDestSize() const { return isSystemLittle() ? swapEndian64(UNION.ZLIB.dest_size) : UNION.ZLIB.dest_size; }
        /// bytes 0-3
        ND u32 getInt1Swapped() const { return isSystemLittle() ? UNION.INT_VIEW.int1 : swapEndian32(UNION.INT_VIEW.int1); }
        /// bytes 4-7
        ND u32 getInt2Swapped() const { return isSystemLittle() ? UNION.INT_VIEW.int2 : swapEndian32(UNION.INT_VIEW.int2); }
        /// bytes 8-11
        ND u32 getInt3Swapped() const { return isSystemLittle() ? UNION.INT_VIEW.int3 : swapEndian32(UNION.INT_VIEW.int3); }
    };

}
