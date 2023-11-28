#pragma once

#include "LegacyEditor/utils/processor.hpp"


#include <cstring>
#include "LegacyEditor/utils/dataManager.hpp"


static void RLEVITA_DECOMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    u8 value;

    while (managerIn.getPosition() < sizeIn) {
        value = managerIn.readInt8();

        if (value != 0) {
            managerOut.writeInt8(value);
        } else {
            int numZeros = managerIn.readInt8();
            memset(managerOut.ptr, 0, numZeros);
            managerOut.incrementPointer(numZeros);
        }
    }
}


/**
 * Technically regular RLE compression.
 * ChatGPT wrote
 * @param dataIn buffer_in to parseLayer from
 * @param sizeIn buffer_in size
 * @param dataOut a pointer to allocated buffer_out
 * @param sizeOut the size of the allocated buffer_out
 */
static u32 RLEVITA_COMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut) {
    if (sizeOut < 2) {
        return 0;
    }

    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    u8 zeroCount = 0;

    for (u32 i = 0; i < sizeIn; ++i) {
        u8 value = managerIn.readInt8();

        if (value != 0) {
            if (zeroCount > 0) {
                managerOut.writeInt8(0);
                managerOut.writeInt8(zeroCount);
                zeroCount = 0;
            }
            managerOut.writeInt8(value);
        } else {
            zeroCount++;
            if (zeroCount == 255 || i == sizeIn - 1) {
                managerOut.writeInt8(0);
                managerOut.writeInt8(zeroCount);
                zeroCount = 0;
            }
        }
    }

    if (zeroCount > 0) {
        managerOut.writeInt8(0);
        managerOut.writeInt8(zeroCount);
    }

    return managerOut.getPosition();
}
