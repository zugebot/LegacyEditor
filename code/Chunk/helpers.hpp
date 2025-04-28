#pragma once

#include <cstring>

#include "bitChecking.hpp"
#include "common/dataManager.hpp"
#include "include/lce/processor.hpp"


namespace editor::chunk {


    inline static u32 toIndex(c_u32 num) {
        return (num + 1) * 128;
    }


    __forceinline static u8 getNibble(const u8_vec& buf, int nibIdx) {
        const int byteIdx = nibIdx >> 1;
        const int shift   = (nibIdx & 1) * 4;
        return (buf[byteIdx] >> shift) & 0xF;
    }


    __forceinline static void setNibble(u8_vec& buf, int nibIdx, u8 value) {
        int byteIdx = nibIdx >> 1;
        int shift   = (nibIdx & 1) * 4;
        buf[byteIdx] &= ~(0xF << shift);
        buf[byteIdx] |=  (value & 0xF) << shift;
    }


    static void readDataBlock(c_u8* dataIn, u8* dataOut) {
        static constexpr int DATA_SECTION_SIZE = 128;
        int offset = 0;

        // first section
        for (int k = 0; k < DATA_SECTION_SIZE; k++) {
            if (dataIn[k] == DATA_SECTION_SIZE) {
                memset(&dataOut[offset], 0, DATA_SECTION_SIZE);
            } else if (dataIn[k] == DATA_SECTION_SIZE + 1) {
                memset(&dataOut[offset], 255, DATA_SECTION_SIZE);
            } else {
                std::memcpy(&dataOut[offset], &dataIn[toIndex(dataIn[k])], DATA_SECTION_SIZE);
            }
            offset += DATA_SECTION_SIZE;
        }
    }

    template<int SIZE>
    static std::vector<u8*> readGetDataBlockVector(ChunkData* chunkData, DataManager* managerIn) {
        std::vector<u8*> dataArray(SIZE);
        for (int i = 0; i < SIZE; i++) {
            c_u32 index = toIndex(managerIn->read<u32>());
            dataArray[i] = managerIn->ptr();
            managerIn->skip(index);
            chunkData->DataGroupCount += index;
        }
        return dataArray;
    }


    static void writeDataBlock(DataManager* managerIn, const u8* dataIn)  {
        static constexpr int GRID_COUNT = 64;
        static constexpr int DATA_SECTION_SIZE = 128;

        static u32_vec sectionOffsets;
        sectionOffsets.reserve(GRID_COUNT);

        u32 readOffset = 0;

        // it does it twice, after the first interval, readOffset should be 32767
        // for (int index = 0; index < countIn; index++) {

        c_u32 start = managerIn->tell();
        managerIn->write<u32>(0);
        sectionOffsets.clear();

        // Write headers
        u32 sectionOffsetSize = 0;
        c_u8* ptr = dataIn + readOffset;
        for (int i = 0; i < DATA_SECTION_SIZE; i++) {
            if (is_zero_128(ptr)) {
                managerIn->write<u8>(DATA_SECTION_SIZE);
            } else if (is_ff_128(ptr)) {
                managerIn->write<u8>(DATA_SECTION_SIZE + 1);
            } else {
                sectionOffsets.push_back(readOffset);
                managerIn->write<u8>(sectionOffsetSize++);
            }
            ptr += DATA_SECTION_SIZE;
            readOffset += DATA_SECTION_SIZE;
        }

        // Write light data sections
        for (c_u32 offset : sectionOffsets) {
            managerIn->writeBytes(&dataIn[offset], DATA_SECTION_SIZE);
        }

        // Calculate and write the size
        c_u32 end = managerIn->tell();
        c_u32 size = (end - start - 4 - 128) / 128; // -4 to exclude size header
        managerIn->writeAtOffset<u32>(start, size);
    }


    template<int GRID_SIZE>
    void fillAllBlocks(c_u8* buffer, u8 grid[GRID_SIZE]) {
        std::memcpy(grid, buffer, GRID_SIZE);
    }



}
