#include "v12Chunk.hpp"

#include <iostream>
#include <random>
// #include "LegacyEditor/utils/time.hpp"



/**
 * This checks if the next 1024 bits are all zeros.\n
 * this is u8[128]
 * @param ptr
 * @return true if all bits are zero, else 0.
 */
static bool is0_128_slow(const u8* ptr) {
    for (int i = 0; i < 128; ++i) {
        if (ptr[i] != 0x00) {
            return false;
        }
    }
    return true;
}

static bool is255_128_slow(const u8* ptr) {
    for (int i = 0; i < 128; ++i) {
        if (ptr[i] != 0xFF) {
            return false;
        }
    }
    return true;
}


namespace universal {
    // static size_t totalTime;
    // static size_t totalAmount;

    void V12Chunk::writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim) {
        dataManager = managerOut;
        chunkData = chunkDataIn;

        dataManager->writeInt32(chunkData->chunkX);
        dataManager->writeInt32(chunkData->chunkZ);
        dataManager->writeInt64(chunkData->lastUpdate);
        dataManager->writeInt64(chunkData->inhabitedTime);

        writeBlockData();

        /*
        u8* start = dataManager->ptr;
        auto t_start = getNanoSeconds();
        auto t_end = getNanoSeconds();
        u8* end = dataManager->ptr;
        // dataManager->writeToFile(start, end - start, dir_path + "block_write.bin");
        auto diff = t_end - t_start;
        totalTime += diff;
        totalAmount++;
        // printf("Block Write Size: %llu bytes. \n", end - start);
        // printf("Block Write Time: (%.3fms) | Avg (%.3fms)\n", float(diff) / 1000000, float(float(totalTime) / float(totalAmount)) / 1000000);
        */

        writeLightData();

        dataManager->writeBytes(chunkData->heightMap.data(), 256);
        dataManager->writeInt16(chunkData->terrainPopulated);
        dataManager->writeBytes(chunkData->biomes.data(), 256);

        if (chunkData->NBTData != nullptr) {
            NBT::writeTag(chunkData->NBTData, *dataManager);
        }

    }



    void V12Chunk::writeBlockData() {

        std::vector<u16> blockVector;
        std::vector<u16> blockLocations;
        u16 gridHeader[64];
        u16 sectJumpTable[16] = {0};
        u8 sectSizeTable[16] = {0};
        u8 blockMap[65536] = {0};

        blockVector.reserve(64);
        blockLocations.reserve(64);

        // header ptr offsets from start
        const u32 H_BEGIN = 26;
        const u32 H_SECT_JUMP_TABLE = H_BEGIN +  2; // step 2: i16 * 16 section jump table
        const u32 H_SECT_SIZE_TABLE = H_BEGIN + 34; // step 3:  i8 * 16 section size table / 256
        const u32 H_SECT_START =      H_BEGIN + 50;
        const u32 V_GRID_SIZE = 128;

        /// increment 50 for block header
        dataManager->seek(H_SECT_START);

        u32 last_section_jump = 0;
        u32 last_section_size;

        for (u32 sectionIndex = 0; sectionIndex < 16; sectionIndex++) {
            const u32 CURRENT_INC_SECT_JUMP = last_section_jump * 256;
            const u32 CURRENT_SECTION_START = H_SECT_START + CURRENT_INC_SECT_JUMP;
            u32 sectionSize = 0;

            sectJumpTable[sectionIndex] = CURRENT_INC_SECT_JUMP;

            dataManager->ptr = dataManager->data + H_SECT_START + CURRENT_INC_SECT_JUMP + V_GRID_SIZE;

            u32 gridIndex = 0;
            for (u32 gridX = 0; gridX < 65536; gridX += 16384) {
                for (u32 gridZ = 0; gridZ < 4096; gridZ += 1024) {
                    for (u32 gridY = 0; gridY < 16; gridY += 4) {
                        blockVector.clear(); blockLocations.clear();

                        // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                        u32 offsetInBlock = sectionIndex * 16 + gridY + gridZ + gridX;
                        for (u32 blockX = 0; blockX < 16384; blockX += 4096) {
                            for (u32 blockZ = 0; blockZ < 1024; blockZ += 256) {
                                for (u32 blockY = 0; blockY < 4; blockY++) {

                                    u32 blockIndex = offsetInBlock + blockY + blockZ + blockX;
                                    u16 block = chunkData->newBlocks[blockIndex];

                                    if (blockMap[block]) {
                                        blockLocations.push_back(blockMap[block] - 1);
                                    } else {
                                        blockMap[block] = blockVector.size() + 1;
                                        u16 location = blockVector.size();
                                        blockVector.push_back(block);
                                        blockLocations.push_back(location);
                                    }
                                }
                            }
                        }

                        u16 gridFormat, gridID, gridSize;
                        switch (blockVector.size()) {
                            case 1:
                                gridSize = 0;
                                gridID = blockVector[0];
                                blockMap[blockVector[0]] = 0;
                                goto SWITCH_END;

                            case 2:
                                gridFormat = V12_1_BIT; gridSize = 12;
                                writeGrid<1, 2, 0>(blockVector, blockLocations, blockMap); break;

                            case 3:
                                gridFormat = V12_2_BIT; gridSize = 24;
                                writeGrid<2, 3, 1>(blockVector, blockLocations, blockMap); break;
                            case 4:
                                gridFormat = V12_2_BIT; gridSize = 24;
                                writeGrid<2, 4, 0>(blockVector, blockLocations, blockMap); break;

                            case 5:
                                gridFormat = V12_3_BIT; gridSize = 40;
                                writeGrid<3, 5, 3>(blockVector, blockLocations, blockMap); break;
                            case 6:
                                gridFormat = V12_3_BIT; gridSize = 40;
                                writeGrid<3, 6, 2>(blockVector, blockLocations, blockMap); break;
                            case 7:
                                gridFormat = V12_3_BIT; gridSize = 40;
                                writeGrid<3, 7, 1>(blockVector, blockLocations, blockMap); break;
                            case 8:
                                gridFormat = V12_3_BIT; gridSize = 40;
                                writeGrid<3, 8, 0>(blockVector, blockLocations, blockMap); break;

                            case 9:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 9, 7>(blockVector, blockLocations, blockMap); break;
                            case 10:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 10, 6>(blockVector, blockLocations, blockMap); break;
                            case 11:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 11, 5>(blockVector, blockLocations, blockMap); break;
                            case 12:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 12, 4>(blockVector, blockLocations, blockMap); break;
                            case 13:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 13, 3>(blockVector, blockLocations, blockMap); break;
                            case 14:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 14, 2>(blockVector, blockLocations, blockMap); break;
                            case 15:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 15, 1>(blockVector, blockLocations, blockMap); break;
                            case 16:
                                gridFormat = V12_4_BIT; gridSize = 64;
                                writeGrid<4, 16, 0>(blockVector, blockLocations, blockMap); break;
                            default:
                                gridFormat = V12_8_FULL; gridSize = 128;
                                writeWithMaxBlocks(blockVector, blockLocations);
                                for (u16 block : blockVector) { blockMap[block] = 0; }
                                break;
                        }

                        gridID = (sectionSize / 4) | gridFormat << 12;
                   SWITCH_END:;

                        gridHeader[gridIndex++] = gridID;
                        sectionSize += gridSize;

                    }

                }
            }

            // write grid header in subsection
            dataManager->setLittleEndian();
            for (size_t index = 0; index < 64; index++) {
                dataManager->writeInt16AtOffset(CURRENT_SECTION_START + 2 * index, gridHeader[index]);
            }
            dataManager->setBigEndian();

            // write section size to section size table
            if (is0_128_slow(dataManager->data + CURRENT_SECTION_START)) {
                last_section_size = 0;
                dataManager->ptr -= V_GRID_SIZE;
            } else {
                last_section_size = (V_GRID_SIZE + sectionSize + 255) / 256;
                last_section_jump += last_section_size;
            }
            sectSizeTable[sectionIndex] = last_section_size;
        }

        // at root header, write section jump and size tables
        for (size_t sectionIndex = 0; sectionIndex < 16; sectionIndex++) {
            dataManager->writeInt16AtOffset(H_SECT_JUMP_TABLE + 2 * sectionIndex, sectJumpTable[sectionIndex]);
            dataManager->writeInt8AtOffset( H_SECT_SIZE_TABLE + 1 * sectionIndex, sectSizeTable[sectionIndex]);
        }

        u32 final_val = sectJumpTable[15] + sectSizeTable[15] * 256;

        // at root header, write total file size
        dataManager->writeInt16AtOffset(H_BEGIN, final_val >> 8);
        dataManager->seek(H_SECT_START + final_val);
    }


    /**
     * 2: 1 |  2 | [_4] palette, [_8] positions
     * 4: 2 |  4 | [_8] palette, [16] positions
     * 6: 3 |  8 | [16] palette, [24] positions
     * 8: 4 | 16 | [32] palette, [32] positions
     * @tparam BitsPerBlock
     */
    template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
    void V12Chunk::writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8* blockMap) {

        // write the block data
        dataManager->setLittleEndian();
        for (size_t blockIndex = 0; blockIndex < BlockCount; blockIndex++) {
            dataManager->writeInt16(blockVector[blockIndex]);
        }
        dataManager->setBigEndian();

        // fill rest of empty palette with 0xFF's
        // TODO: IDK if this is actually necessary
        for (size_t rest = 0; rest < EmptyCount; rest++) {
            dataManager->writeInt16(0xFFFF);
        }


        //  write the position data
        //  so, write the first bit of each position, as a single u64,
        //  then the second, third etc. N times, where N is BitsPerBlock
        for (size_t bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (size_t locIndex = 0; locIndex < 64; locIndex++) {
                u64 pos = blockLocations[locIndex];
                position |= ((pos >> bitIndex) & 1) << (63 - locIndex);
            }
            dataManager->writeInt64(position);
        }

        // clear the table
        for (size_t i = 0; i < BlockCount; ++i) {
            blockMap[blockVector[i]] = 0;
        }


    }


    /// make this copy all u16 blocks from the grid location or whatnot
    void V12Chunk::writeWithMaxBlocks(u16_vec& blockVector, u16_vec& blockLocations) const {
        dataManager->setLittleEndian();
        for (size_t i = 0; i < 64; i++) {
            u16 blockPos = blockLocations[i];
            dataManager->writeInt16(blockVector[blockPos]);
        }
        dataManager->setBigEndian();
    }


    void V12Chunk::writeLightSection(u32& readOffset, u8_vec& light) const {
        static u32_vec sectionOffsets;
        sectionOffsets.reserve(64);

        u32 start = dataManager->getPosition();
        dataManager->writeInt32(0);
        sectionOffsets.clear();

        // Write headers
        u32 sectionOffsetSize = 0;
        u8* ptr = light.data() + readOffset;
        for (int i = 0; i < 128; i++) {
            if (is0_128_slow(ptr)) {
                dataManager->writeInt8(128);
            } else if (is255_128_slow(ptr)) {
                dataManager->writeInt8(129);
            } else {
                sectionOffsets.push_back(readOffset);
                dataManager->writeInt8(sectionOffsetSize++);
            }
            ptr += 128;
            readOffset += 128;
        }

        // Write light data sections
        for (u32 offset : sectionOffsets) {
            dataManager->writeBytes(&light[offset], 128);
        }

        // Calculate and write the size
        u32 end = dataManager->getPosition();
        u32 size = (end - start - 4 - 128) / 128; // -4 to exclude size header
        dataManager->writeInt32AtOffset(start, size);
    }


    void V12Chunk::writeLightData() {
        u32 readOffset = 0;
        writeLightSection(readOffset, chunkData->skyLight);
        writeLightSection(readOffset, chunkData->skyLight);
        readOffset = 0;
        writeLightSection(readOffset, chunkData->blockLight);
        writeLightSection(readOffset, chunkData->blockLight);
    }

}