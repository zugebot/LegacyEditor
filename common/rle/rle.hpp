#pragma once

#include "include/lce/processor.hpp"

#include "common/data/DataReader.hpp"
#include "common/data/DataWriter.hpp"


namespace codec {

    static void RLE_decompress(u8* dataOut, u32* sizeOut, u8* dataIn, c_u32 sizeIn) {
        u32 indexIn = 0;
        u32 indexOut = 0;

        while (indexIn < sizeIn) {
            c_u8 first = dataIn[indexIn++];
            if (first != 255) {
                dataOut[indexOut++] = first;

            } else {
                c_u32 length = dataIn[indexIn++];
                u8 value = 255;
                if (length >= 3) {
                    value = dataIn[indexIn++];
                }
                for (u32 j = 0; j <= length; j++) {
                    dataOut[indexOut++] = value;
                }
            }
        }
        *sizeOut = indexOut;
    }


    static u32 RLE_safe_compress_size(c_u32 sizeIn) {
        return sizeIn + ((sizeIn + 1) / 2);
    }

    static void RLE_compress(u8* dataOut, u32* sizeOut, c_u8* dataIn, c_u32 sizeIn) {
        u32 indexOut = 0;
        u32 indexIn = 0;

        while (indexIn < sizeIn) {
            c_u8 value = dataIn[indexIn];
            u32 count = 1;

            // Count the run of the same byte
            while (indexIn + count < sizeIn && dataIn[indexIn + count] == value && count < 256) {
                count++;
            }

            if (value == 255) {
                if (count < 4) {
                    dataOut[indexOut++] = 255;
                    dataOut[indexOut++] = count - 1;
                } else {
                    dataOut[indexOut++] = 255;
                    dataOut[indexOut++] = count - 1;
                    dataOut[indexOut++] = value;
                }
            } else {
                if (count >= 4) {
                    // If run length is 4 or more, encode it
                    dataOut[indexOut++] = 255;
                    dataOut[indexOut++] = count - 1;
                    dataOut[indexOut++] = value;
                } else {
                    // Else write the bytes directly
                    for (u32 i = 0; i < count; ++i) {
                        dataOut[indexOut++] = value;
                    }
                }
            }

            indexIn += count;
        }
        *sizeOut = indexOut;
    }
}