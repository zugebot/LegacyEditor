#pragma once

#include "include/lce/processor.hpp"
#include "common/utils.hpp"

namespace editor {
    /**
     * Console File header that holds 12 bytes
     * uses many struct unions to interpret the header for the save file of different consoles.
     */
    class HeaderUnion {
        union {
            struct {
                u32 int1; ///< bytes 0-3
                u32 int2; ///< bytes 4-7
                u32 int3; ///< bytes 8-11
            } INT_VIEW; ///< header size: 12 bytes
            struct {
                u64 dest_size;  ///< bytes 0-7
                u16 zlib_magic; ///< bytes 8-9
            } ZLIB; ///< header size: 10 bytes
        } UNION;
    public:
        ND u32 getInt1()     const { return detail::maybe_bswap(UNION.INT_VIEW.int1, Endian::Big); }    ///< bytes 0-3, Big
        ND u32 getInt2()     const { return detail::maybe_bswap(UNION.INT_VIEW.int2, Endian::Big); }    ///< bytes 4-7, Big
        ND u32 getInt3()     const { return detail::maybe_bswap(UNION.INT_VIEW.int3, Endian::Big); }    ///< bytes 8-11, Big
        ND u32 getShort5()   const { return detail::maybe_bswap(UNION.ZLIB.zlib_magic, Endian::Big); }  ///< bytes 8-9, Big
        ND u64 getDestSize() const { return detail::maybe_bswap(UNION.ZLIB.dest_size, Endian::Big); }   ///< bytes 0-7, Big
        ND u32 getInt2Swap() const { return detail::maybe_bswap(UNION.INT_VIEW.int2, Endian::Little); } ///< bytes 4-7, Little
        ND u32 getInt3Swap() const { return detail::maybe_bswap(UNION.INT_VIEW.int3, Endian::Little); } ///< bytes 8-11, Little
    };
}