#pragma once

#include "include/lce/processor.hpp"

#include "../data/DataReader.hpp"
#include "../data/DataWriter.hpp"


namespace codec {

    static void RLE_decompress(u8* dataIn, c_u32 sizeIn, u8* dataOut, u32& sizeOut) {
        // DataManager managerIn(dataIn, sizeIn);
        // DataManager managerOut(dataOut, sizeOut);

        // dataIn, indexIn, sizeIn
        // dataOut, indexOut, sizeOut
        u32 indexIn = 0;
        u32 indexOut = 0;

        while (indexIn < sizeIn) {
            if (c_u8 byte1 = dataIn[indexIn++]; byte1 != 255) {
                dataOut[indexOut++] = byte1;
            } else {
                c_u8 byte2 = dataIn[indexIn++];
                u8 value = 255;
                if (byte2 >= 3) {
                    value = dataIn[indexIn++];
                }
                for (int j = 0; j <= static_cast<int>(byte2); j++) {
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
