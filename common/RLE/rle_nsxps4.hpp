#pragma once

#include "include/lce/processor.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace codec {

    /**
     * A form of RLE decompression.
     *
     * @param dataIn buffer_in to parseLayer from
     * @param sizeIn buffer_in size
     * @param dataOut a pointer to allocated buffer_out
     * @param sizeOut the size of the allocated buffer_out
     */
    static u32 RLE_NSX_OR_PS4_DECOMPRESS(c_u8* dataIn, c_u32 sizeIn, u8* dataOut, c_u32 sizeOut) {
        DataReader reader(dataIn, sizeIn);
        DataWriter writer(sizeOut);

        while (reader.tell() < sizeIn) {

            if (c_u8 value = reader.read<u8>(); value != 0x00) {
                writer.write<u8>(value);

            } else {
                int numZeros = reader.read<u8>();

                if (numZeros == 0) {
                    c_int numZeros1 = reader.read<u8>();
                    c_int numZeros2 = reader.read<u8>();
                    numZeros = numZeros1 << 8 | numZeros2;
                    numZeros += 256;
                }

                writer.writePad(numZeros, 0);
            }
        }

        std::memcpy(dataOut, writer.data(), writer.size());
        return static_cast<u32>(writer.size());
    }


    /**
     * A form of RLE compression.
     *
     * @param dataIn buffer_in to parseLayer from
     * @param sizeIn buffer_in size
     * @param dataOut a pointer to allocated buffer_out
     * @param sizeOut the size of the allocated buffer_out
     */
    MU static u32 RLE_NSXPS4_COMPRESS(c_u8* dataIn, c_u32 sizeIn, u8* dataOut, u32 sizeOut) {
        u32 dataIndex = 0;
        sizeOut = 0;

        while (dataIndex < sizeIn) {
            if (c_u8 value = dataIn[dataIndex]; value != 0) {
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
}