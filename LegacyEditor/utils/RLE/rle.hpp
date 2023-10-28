#pragma once

#include "LegacyEditor/utils/processor.hpp"




static void RLE_uncompress(const u8* byte_0, u32 sizeIn, u8* dataOut, u32& sizeOut) {
    u32 i = 0;
    sizeOut = 0;
    while (i < sizeIn) {
        u8 b = byte_0[i++];
        if (b != 255) {
            if (sizeOut >= sizeIn) {
                return;
            }
            dataOut[sizeOut++] = b;
        } else {
            u8 b2 = byte_0[i++];
            u8 value = 255;
            if (b2 >= 3) {
                value = byte_0[i++];
            }
            for (u8 j = 0; j <= b2; j++) {
                if (sizeOut >= sizeIn) {
                    // Prevent buffer overflow
                    return;
                }
                dataOut[sizeOut++] = value;
            }
        }
    }
}









/*
static void RLE_uncompress(const u8* ptrIn, u32 sizeIn, u8* ptrOut, u32& sizeOut) {
    if (sizeIn == 0 || ptrIn == nullptr || ptrOut == nullptr) {
        sizeOut = 0;
        return;
    }

    int i = 0;
    sizeOut = 0;
    while (i < sizeIn) {
        u8 b = ptrIn[i++];
        if (b != 255) {
            ptrOut[sizeOut++] = b;
        } else {
            u8 b2 = ptrIn[i++];
            u8 value = 255;
            if (b2 >= 3) {
                value = ptrIn[i++];
            }
            for (u8 j = 0; j <= b2; j++) {
                ptrOut[sizeOut++] = value;
            }
        }
    }
}
*/

/// TODO: CHATGPT wrote this code, it may not work
static void RLE_compress(const u8* ptrIn, u32 sizeIn, u8* ptrOut, u32& sizeOut) {
    if (sizeIn == 0 || ptrIn == nullptr || ptrOut == nullptr) {
        sizeOut = 0;
        return;
    }

    int i = 0;
    sizeOut = 0;

    while (i < sizeIn) {
        u8 currentByte = ptrIn[i];
        int runLength = 1;

        // Count the number of consecutive identical bytes
        while (i + runLength < sizeIn && ptrIn[i + runLength] == currentByte && runLength < 255) {
            ++runLength;
        }

        // If the byte is not 255, or the run length is less than 3, write it directly
        if (currentByte != 255 || runLength < 3) {
            for (int j = 0; j < runLength; ++j) {
                ptrOut[sizeOut++] = currentByte;
            }
        } else {
            // For byte 255 with run length >= 3, write 255, run length, and then the byte
            ptrOut[sizeOut++] = 255;
            ptrOut[sizeOut++] = runLength - 1;
            ptrOut[sizeOut++] = currentByte;
        }

        i += runLength;
    }
}