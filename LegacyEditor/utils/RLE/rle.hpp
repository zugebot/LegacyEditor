#pragma once

#include "LegacyEditor/utils/processor.hpp"


static void RLE_uncompress(const u8* ptrIn, u32 sizeIn, u8* ptrOut, u32& sizeOut) {
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


void RLE_compress(const u8* ptrIn, u32 sizeIn, u8* ptrOut, u32& sizeOut) {

}
