#pragma once

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/processor.hpp"


namespace universal {


class ChunkParserBase {
private:
    static constexpr u8 MAGIC_128 = 0x80;
public:

    static u8_vec read128(DataManager& inputData) {
        i32 num = (i32) inputData.readInt32();
        u8_vec array1;
        array1 = inputData.readIntoVector((num + 1) * MAGIC_128);
        return array1;
    }

    static u8_vec read256(DataManager& inputData) {
        u8_vec array1 = inputData.readIntoVector(256);
        return array1;
    }

    /// TODO: can probably use memfill or whatever its called
    static void copyByte128(u8_vec& writeVector, int writeOffset, u8 value) {
        for (int i = 0; i < MAGIC_128; i++) {
            writeVector[writeOffset + i] = value;
        }
    }

    static void copyArray128(u8_vec& readVector, int readOffset, u8_vec& writeVector, int writeOffset) {
        for (int i = 0; i < MAGIC_128; i++) {
            writeVector[writeOffset + i] = readVector[readOffset + i];
        }
    }

};

}

