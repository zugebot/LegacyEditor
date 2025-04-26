#pragma once

#include <cstring>

#include "include/lce/processor.hpp"

#include "common/dataManager.hpp"


static u32 RLEVITA_DECOMPRESS(u8* dataIn, c_u32 sizeIn, u8* dataOut, c_u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    while (managerIn.tell() < sizeIn) {

        if (c_u8 value = managerIn.read<u8>(); value != 0x00) {
            managerOut.write<u8>(value);
        } else {
            c_int numZeros = managerIn.read<u8>();
            memset(managerOut.ptr(), 0, numZeros);
            managerOut.skip(numZeros);
        }
    }
    return managerOut.tell();
}


/**
 * Technically regular RLE compression.
 *
 * @param dataIn buffer_in to parseLayer from
 * @param sizeIn buffer_in size
 * @param dataOut a pointer to allocated buffer_out
 * @param sizeOut the size of the allocated buffer_out
 */
static u32 RLEVITA_COMPRESS(u8* dataIn, c_u32 sizeIn, u8* dataOut, c_u32 sizeOut) {
    if (sizeOut < 2) {
        return 0;
    }

    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    u8 zeroCount = 0;

    for (u32 i = 0; i < sizeIn; ++i) {

        if (c_u8 value = managerIn.read<u8>(); value != 0) {
            if (zeroCount > 0) {
                managerOut.write<u8>(0);
                managerOut.write<u8>(zeroCount);
                zeroCount = 0;
            }
            managerOut.write<u8>(value);
        } else {
            zeroCount++;
            if (zeroCount == 255 || i == sizeIn - 1) {
                managerOut.write<u8>(0);
                managerOut.write<u8>(zeroCount);
                zeroCount = 0;
            }
        }
    }

    if (zeroCount > 0) {
        managerOut.write<u8>(0);
        managerOut.write<u8>(zeroCount);
    }

    return managerOut.tell();
}
