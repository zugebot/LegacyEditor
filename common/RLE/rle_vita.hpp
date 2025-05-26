#pragma once

#include "include/lce/processor.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"

namespace codec {

    static u32 RLEVITA_DECOMPRESS(c_u8* dataIn, c_u32 sizeIn, u8* dataOut, c_u32 sizeOut) {
        DataReader managerIn(dataIn, sizeIn);
        DataWriter managerOut(sizeOut);

        while (managerIn.tell() < sizeIn) {

            if (c_u8 value = managerIn.read<u8>(); value != 0x00) {
                managerOut.write<u8>(value);
            } else {
                c_int numZeros = managerIn.read<u8>();
                managerOut.writePad(numZeros, 0);
            }
        }
        std::memcpy(dataOut, managerOut.data(), managerOut.size());
        return static_cast<u32>(managerOut.size());
    }


    /**
     * Technically regular RLE compression.
     *
     * @param dataIn buffer_in to parseLayer from
     * @param sizeIn buffer_in size
     * @param dataOut a pointer to allocated buffer_out
     * @param sizeOut the size of the allocated buffer_out
     */
    static u32 RLEVITA_COMPRESS(c_u8* dataIn, c_u32 sizeIn, u8* dataOut, c_u32 sizeOut) {
        if (sizeOut < 2) {
            return 0;
        }

        DataReader managerIn(dataIn, sizeIn);
        DataWriter managerOut(sizeOut);

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

        std::memcpy(dataOut, managerOut.data(), managerOut.size());
        return static_cast<u32>(managerOut.size());
    }

}
