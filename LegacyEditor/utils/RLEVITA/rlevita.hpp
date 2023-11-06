#pragma once

#include "LegacyEditor/utils/dataManager.hpp"
#include <cstring>

void RLEVITA_DECOMPRESS(u8* dataIn, u32 sizeIn,
                        u8* dataOut, u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    u8 value;

    while (managerIn.getPosition() < sizeIn) {
        value = managerIn.readByte();

        if (value != 0) {
            managerOut.writeByte(value);
        } else {
            int numZeros = managerIn.readByte();
            memset(managerOut.ptr, 0, numZeros);
            managerOut.incrementPointer(numZeros);
        }
    }
}


void RLEVITA_COMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut) {
    DataManager managerIn(dataIn, sizeIn);
    DataManager managerOut(dataOut, sizeOut);

    while (managerIn.getPosition() < sizeIn) {
        u8 value = managerIn.readByte();

        if (value != 0) {
            managerOut.writeByte(value);
        } else {
            u8 zeroCount = 0;
            do {
                zeroCount++;
                if (zeroCount == 255 || managerIn.getPosition() >= sizeIn) {
                    break;
                }
            } while (managerIn.peekNextByte() == 0 && managerIn.readByte() == 0);

            managerOut.writeByte(0);
            managerOut.writeByte(zeroCount);
        }
    }
}
