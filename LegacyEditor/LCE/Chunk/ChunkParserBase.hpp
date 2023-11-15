#pragma once

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include <cstring>


namespace universal {


class ChunkParserBase {
public:

    static i32 toIndex(i32 num) {
        return (num + 1) * 128;
    }

    static u8_vec read128(DataManager& inputData) {
        i32 num = (i32) inputData.readInt32();
        u8_vec array1 = inputData.readIntoVector(toIndex(num));
        return array1;
    }

    static u8_vec read256(DataManager& inputData) {
        u8_vec array1 = inputData.readIntoVector(256);
        return array1;
    }

    /// TODO: can probably use memfill or whatever its called
    static void copyByte128(u8_vec& writeVector, int writeOffset, u8 value) {
        for (int i = 0; i < 128; i++) {
            writeVector[writeOffset + i] = value;
        }
    }

    static void copy0Byte128(u8_vec& writeVector, int writeOffset) {
        memset(&writeVector[writeOffset], 0, 128);
    }

    static void copy255Byte128(u8_vec& writeVector, int writeOffset) {
        memset(&writeVector[writeOffset], 255, 128);

    }

    static void copyArray128(const u8_vec& srcVector, int srcOffset, u8_vec& destVector, int destOffset) {
        std::memcpy(&destVector[destOffset], &srcVector[srcOffset], 128);
    }


};

}

