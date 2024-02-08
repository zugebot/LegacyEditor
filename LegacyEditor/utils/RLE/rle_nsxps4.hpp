#pragma once

#include "../dataManager.hpp"
#include "../processor.hpp"
#include <cstring>


/**
 * A form of RLE decompression.
 *
 * @param dataIn buffer_in to parseLayer from
 * @param sizeIn buffer_in size
 * @param dataOut a pointer to allocated buffer_out
 * @param sizeOut the size of the allocated buffer_out
 */
static u32 RLE_NSXPS4_DECOMPRESS(u8* dataIn, const u32 sizeIn, u8* dataOut, const u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    while (managerIn.getPosition() < sizeIn) {

        if (const u8 value = managerIn.readInt8(); value != 0x00) {
            managerOut.writeInt8(value);

        } else {
            int numZeros = managerIn.readInt8();

            if (numZeros == 0) {
                const int numZeros1 = managerIn.readInt8();
                const int numZeros2 = managerIn.readInt8();
                numZeros = numZeros1 << 8 | numZeros2;
                numZeros += 256;
            }

            memset(managerOut.ptr, 0, numZeros);
            managerOut.incrementPointer(numZeros);
        }
    }
    return managerOut.getPosition();
}


/**
 * A form of RLE compression.
 *
 * @param dataIn buffer_in to parseLayer from
 * @param sizeIn buffer_in size
 * @param dataOut a pointer to allocated buffer_out
 * @param sizeOut the size of the allocated buffer_out
 */
static u32 RLE_NSXPS4_COMPRESS(const u8* dataIn, const u32 sizeIn, u8* dataOut, u32 sizeOut) {
    u32 dataIndex = 0;
    sizeOut = 0;

    while (dataIndex < sizeIn) {
        if (const u8 value = dataIn[dataIndex]; value != 0) {
            dataOut[sizeOut++] = value;
            continue;
        }

        u32 runCount = 1;
        while (dataIndex + runCount < sizeIn && dataIn[dataIndex + runCount] == 0) {
            runCount++;
        }

        if (runCount < 256) {
            dataOut[sizeOut++] = 0;
            dataOut[sizeOut++] = runCount;
        } else {
            dataOut[sizeOut++] = 0;
            dataOut[sizeOut++] = 0;
            dataOut[sizeOut++] = (runCount >> 8) - 1;
            dataOut[sizeOut++] = runCount & 255;
        }

        dataIndex += runCount;
    }
    return sizeOut;
}


