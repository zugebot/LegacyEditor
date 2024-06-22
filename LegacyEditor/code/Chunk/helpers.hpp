#pragma once



#include <cstring>

#include "LegacyEditor/utils/dataManager.hpp"

#include "lce/processor.hpp"


namespace editor::chunk {


    static u32 toIndex(c_u32 num) {
        u32 val = (num + 1) * 128;
        return val;
    }


    /**
     * This checks if the next 1024 bits are all zeros.\n
     * this is u8[128]
     * @param ptr
     * @return true if all bits are zero, else 0.
     */
    static bool is0_128_slow(c_u8* ptr) {
        for (int i = 0; i < 128; ++i) {
            if (ptr[i] != 0x00) {
                return false;
            }
        }
        return true;
    }


    static bool is255_128_slow(c_u8* ptr) {
        for (int i = 0; i < 128; ++i) {
            if (ptr[i] != 0xFF) {
                return false;
            }
        }
        return true;
    }



    static void readDataBlock(const u8_vec& dataIn, u8_vec& dataOut, int& offset) {
        static constexpr int DATA_SECTION_SIZE = 128;

        for (int k = 0; k < DATA_SECTION_SIZE; k++) {
            if (dataIn[k] == DATA_SECTION_SIZE) {
                memset(&dataOut[offset], 0, DATA_SECTION_SIZE);
            } else if (dataIn[k] == DATA_SECTION_SIZE + 1) {
                memset(&dataOut[offset], 255, DATA_SECTION_SIZE);
            } else {
                memcpy(&dataOut[offset], &dataIn[toIndex(dataIn[k])], DATA_SECTION_SIZE);
            }
            offset += DATA_SECTION_SIZE;
        }
    }


    static void readDataBlock(c_u8* dataIn1, u8* dataIn2, u8_vec& dataOut) {
        static constexpr int DATA_SECTION_SIZE = 128;
        int offset = 0;

        // first section
        for (int k = 0; k < DATA_SECTION_SIZE; k++) {
            if (dataIn1[k] == DATA_SECTION_SIZE) {
                memset(&dataOut[offset], 0, DATA_SECTION_SIZE);
            } else if (dataIn1[k] == DATA_SECTION_SIZE + 1) {
                memset(&dataOut[offset], 255, DATA_SECTION_SIZE);
            } else {
                memcpy(&dataOut[offset], &dataIn1[toIndex(dataIn1[k])], DATA_SECTION_SIZE);
            }
            offset += DATA_SECTION_SIZE;
        }

        // second section
        for (int k = 0; k < DATA_SECTION_SIZE; k++) {
            if (dataIn2[k] == DATA_SECTION_SIZE) {
                memset(&dataOut[offset], 0, DATA_SECTION_SIZE);
            } else if (dataIn2[k] == DATA_SECTION_SIZE + 1) {
                memset(&dataOut[offset], 255, DATA_SECTION_SIZE);
            } else {
                memcpy(&dataOut[offset], &dataIn2[toIndex(dataIn2[k])], DATA_SECTION_SIZE);
            }
            offset += DATA_SECTION_SIZE;
        }
    }

    template<int SIZE>
    static std::vector<u8*> readGetDataBlockVector(ChunkData* chunkData, DataManager* managerIn) {
        std::vector<u8*> dataArray(SIZE);
        for (int i = 0; i < SIZE; i++) {
            dataArray[i] = managerIn->ptr;
            c_u32 index = toIndex(managerIn->readInt32());
            managerIn->incrementPointer(index);
            chunkData->DataGroupCount += index;
        }
        return dataArray;
    }


    static void writeDataBlock(DataManager* managerIn, const u8_vec& dataIn)  {
        static constexpr int GRID_COUNT = 64;
        static constexpr int DATA_SECTION_SIZE = 128;

        static u32_vec sectionOffsets;
        sectionOffsets.reserve(GRID_COUNT);

        u32 readOffset = 0;

        // it does it twice, after the first interval, readOffset should be 32767
        for (int index = 0; index < 2; index++) {

            c_u32 start = managerIn->getPosition();
            managerIn->writeInt32(0);
            sectionOffsets.clear();

            // Write headers
            u32 sectionOffsetSize = 0;
            c_u8* ptr = dataIn.data() + readOffset;
            for (int i = 0; i < DATA_SECTION_SIZE; i++) {
                if (is0_128_slow(ptr)) {
                    managerIn->writeInt8(DATA_SECTION_SIZE);
                } else if (is255_128_slow(ptr)) {
                    managerIn->writeInt8(DATA_SECTION_SIZE + 1);
                } else {
                    sectionOffsets.push_back(readOffset);
                    managerIn->writeInt8(sectionOffsetSize++);
                }
                ptr += DATA_SECTION_SIZE;
                readOffset += DATA_SECTION_SIZE;
            }

            // Write light data sections
            for (c_u32 offset : sectionOffsets) {
                managerIn->writeBytes(&dataIn[offset], DATA_SECTION_SIZE);
            }

            // Calculate and write the size
            c_u32 end = managerIn->getPosition();
            c_u32 size = (end - start - 4 - 128) / 128; // -4 to exclude size header
            managerIn->writeInt32AtOffset(start, size);
        } // end of for loop


    }


    template<int GRID_SIZE>
    void fillAllBlocks(c_u8* buffer, u8 grid[GRID_SIZE]) {
        memcpy(grid, buffer, GRID_SIZE);
    }



}
