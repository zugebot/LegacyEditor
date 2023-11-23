#pragma once

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include <cstring>


namespace universal {


    class ChunkParserBase {
    public:

        static u32 toIndex(u32 num) {
            return (num + 1) * 128;
        }

        static u8_vec read128(DataManager& inputData) {
            u32 num = (u32) inputData.readInt32();
            u8_vec array1 = inputData.readIntoVector((i32)toIndex(num));
            return array1;
        }

        static u8_vec read256(DataManager& inputData) {
            u8_vec array1 = inputData.readIntoVector(256);
            return array1;
        }


    };

}

