#pragma once

#include "include/lce/processor.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace codec {

    static void RLE_decompress(u8* dataIn, c_u32 sizeIn,
                               u8* dataOut, u32& sizeOut) {
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
        sizeOut = indexOut;
    }


    static void RLE_compress(c_u8* dataIn, c_u32 sizeIn, u8* dataOut, u32& sizeOut) {
        u32 dataIndex = 0;
        sizeOut = 0;

        while (dataIndex < sizeIn) {
            c_u8 value = dataIn[dataIndex];
            u32 count = 1;

            // Count the run of the same byte
            while (dataIndex + count < sizeIn && dataIn[dataIndex + count] == value && count < 256) {
                count++;
            }

            if (value == 255) {
                if (count < 4) {
                    dataOut[sizeOut++] = 255;
                    dataOut[sizeOut++] = count - 1;
                } else {
                    dataOut[sizeOut++] = 255;
                    dataOut[sizeOut++] = count - 1;
                    dataOut[sizeOut++] = value;
                }
            } else {
                if (count >= 4) {
                    // If run length is 4 or more, encode it
                    dataOut[sizeOut++] = 255;
                    dataOut[sizeOut++] = count - 1;
                    dataOut[sizeOut++] = value;
                } else {
                    // Else write the bytes directly
                    for (u32 i = 0; i < count; ++i) {
                        dataOut[sizeOut++] = value;
                    }
                }
            }

            dataIndex += count;
        }
    }
}
