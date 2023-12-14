#pragma once

#include "LegacyEditor/utils/processor.hpp"


#include <cstring>
#include "LegacyEditor/utils/dataManager.hpp"


static u16 swapEndian16_2(u16 value) { return (value << 8) | (value >> 8); }


static u32 RLESWITCH_DECOMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    u8 value;

    while (managerIn.getPosition() < sizeIn) {
        value = managerIn.readInt8();

        if (value != 0x00) {
            managerOut.writeInt8(value);
            continue;
        }

        int numZeros = managerIn.readInt8();

        if (numZeros == 0) {
            u16 doubleZeros = *(u16*)managerIn.ptr;
            managerIn.ptr += 2;
            numZeros = swapEndian16_2(doubleZeros);
            // printf("expanding 0's: %d at address %d\n", numZeros, managerIn.getPosition());
        }

        memset(managerOut.ptr, 0, numZeros);
        managerOut.incrementPointer(numZeros);
    }
    return managerOut.getPosition();
}


/**
 * Technically regular RLE compression.
 * ChatGPT wrote
 * @param dataIn buffer_in to parseLayer from
 * @param sizeIn buffer_in size
 * @param dataOut a pointer to allocated buffer_out
 * @param sizeOut the size of the allocated buffer_out
 */
static u32 RLESWITCH_COMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut) {
    exit(-1);
    /*
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
     */
}
