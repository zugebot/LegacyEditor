#pragma once

#include "include/lce/processor.hpp"

#include "code/Chunk/helpers/bitChecking.hpp"
#include "common/data/DataReader.hpp"


namespace editor {


    inline static u32 toIndex(c_u32 num) {
        return (num + 1) * 128;
    }


    FORCEINLINE static u8 getNibble(const u8_vec& buf, int nibIdx) {
        const int byteIdx = nibIdx >> 1;
        const int shift   = (nibIdx & 1) * 4;
        return (buf[byteIdx] >> shift) & 0xF;
    }


    FORCEINLINE static void setNibble(u8_vec& buf, int nibIdx, u8 value) {
        int byteIdx = nibIdx >> 1;
        int shift   = (nibIdx & 1) * 4;
        buf[byteIdx] &= ~(0xF << shift);
        buf[byteIdx] |=  (value & 0xF) << shift;
    }


    static void readSection(std::span<const u8> dataIn, u8* dataOut) {
        static constexpr int SUB_SECT_SIZE = 128;
        int offset = 0;

        // first section
        for (int k = 0; k < SUB_SECT_SIZE; k++) {
            if (dataIn[k] == SUB_SECT_SIZE) {
                memset(&dataOut[offset], 0, SUB_SECT_SIZE);
            } else if (dataIn[k] == SUB_SECT_SIZE + 1) {
                memset(&dataOut[offset], 255, SUB_SECT_SIZE);
            } else {
                std::memcpy(&dataOut[offset], &dataIn[toIndex(dataIn[k])], SUB_SECT_SIZE);
            }
            offset += SUB_SECT_SIZE;
        }
    }



    template<int SIZE, bool firstTwoWithoutToIndex = false>
    static std::vector<std::span<const u8>>
    fetchSections(ChunkData* chunkData, DataReader& managerIn) {
        std::vector<std::span<const u8>> dataArray(SIZE);
        for (int i = 0; i < SIZE; i++) {
            if (firstTwoWithoutToIndex && i < 2) {
                c_u32 index = managerIn.read<u32>();
                dataArray[i] = {managerIn.ptr(), index};
                managerIn.skip(index);
                chunkData->DataGroupCount += index;
            } else {
                c_u32 index = toIndex(managerIn.read<u32>());
                dataArray[i] = {managerIn.ptr(), index};
                managerIn.skip(index);
                chunkData->DataGroupCount += index;
            }
        }
        return dataArray;
    }


    static void writeSection(DataWriter& writer, const u8* dataIn, bool flatten = true, bool isLight = false)  {
        static constexpr int GRID_COUNT = 64;
        static constexpr int DATA_SECTION_SIZE = 128;

        static u32_vec sectionOffsets;
        sectionOffsets.reserve(GRID_COUNT);

        u32 readOffset = 0;

        c_u32 start = writer.tell();
        writer.write<u32>(0);
        sectionOffsets.clear();

        // Write headers
        u32 sectionOffsetSize = 0;
        c_u8* ptr = dataIn + readOffset;
        for (int i = 0; i < DATA_SECTION_SIZE; i++) {
            bool allowFlatten = flatten || (isLight && (i == DATA_SECTION_SIZE - 1));
            if (is_zero_128(ptr) && allowFlatten) {
                writer.write<u8>(DATA_SECTION_SIZE);
            } else if (is_ff_128(ptr) && allowFlatten) {
                writer.write<u8>(DATA_SECTION_SIZE + 1);
            } else {
                sectionOffsets.push_back(readOffset);
                writer.write<u8>(sectionOffsetSize++);
            }
            ptr += DATA_SECTION_SIZE;
            readOffset += DATA_SECTION_SIZE;
        }

        // Write light data sections
        for (c_u32 offset : sectionOffsets) {
            writer.writeBytes(&dataIn[offset], DATA_SECTION_SIZE);
        }

        // Calculate and write the size
        c_u32 end = writer.tell();
        c_u32 size = (end - start - 4 - 128) / 128; // -4 to exclude size header
        writer.writeAtOffset<u32>(start, size);
    }


    template<int GridSize>
    void fillAllBlocks(c_u8* buffer, u8 grid[GridSize]) {
        std::memcpy(grid, buffer, GridSize);
    }



}
