#include "LegacyEditor/utils/time.hpp"
#include "v12.hpp"


static inline u32 toIndex(u32 num) {
    return (num + 1) * 128;
}


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




namespace chunk {

    // #####################################################
    // #               Read Section
    // #####################################################

    void ChunkV12::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
        dataManager = managerIn;
        chunkData = chunkDataIn;

        chunkData->dimension = dim;
        chunkData->chunkX = (i32) dataManager->readInt32();
        chunkData->chunkZ = (i32) dataManager->readInt32();
        chunkData->lastUpdate = (i64) dataManager->readInt64();
        chunkData->inhabitedTime = (i64) dataManager->readInt64();

        chunkData->newBlocks = u16_vec(65536);
        chunkData->submerged = u16_vec(65536);
        readBlockData();

        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        readLightData();

        chunkData->heightMap = dataManager->readIntoVector(256);
        chunkData->terrainPopulated = (i16) dataManager->readInt16();
        chunkData->biomes = dataManager->readIntoVector(256);

        if (*dataManager->ptr == 0xA) {
            chunkData->NBTData = NBT::readTag(*dataManager);
        }
    }


    static void placeBlocks(u16_vec& writeVec, const u8* grid, int writeOffset) {
        int readOffset = 0;
        for (int x = 0; x < 4; x++) {
            for (int z = 0; z < 4; z++) {
                for (int y = 0; y < 4; y++) {
                    int currentOffset = y + z * 256 + x * 4096;
                    u8 v1 = grid[readOffset++];
                    u8 v2 = grid[readOffset++];
                    writeVec[currentOffset + writeOffset] = static_cast<u16>(v1) | (static_cast<u16>(v2) << 8);
                }
            }
        }
    }


    static void fillWithMaxBlocks(const u8* buffer, u8 grid[128]) {
        std::copy_n(buffer, 128, grid);
    }


    void ChunkV12::readBlockData() const {
        u32 maxSectionAddress = dataManager->readInt16() << 8;

        u16_vec sectionJumpTable(16);
        for (int i = 0; i < 16; i++) {
            u16 address = dataManager->readInt16();
            sectionJumpTable[i] = address;
        }

        u8_vec sizeOfSubChunks = dataManager->readIntoVector(16);

        if (maxSectionAddress == 0) {
            return;
        }

        for (int section = 0; section < 16; section++) {
            int address = sectionJumpTable[section];
            dataManager->seek(76 + address); // 26 chunk header + 50 section header
            if (address == maxSectionAddress) {
                break;
            }
            if (!sizeOfSubChunks[section]) {
                continue;
            }
            // TODO: replace with telling cpu to cache that address, and use a ptr?
            u8_vec sectionHeader = dataManager->readIntoVector(128);

            u16 gridFormats[64] = {0};
            u16 gridOffsets[64] = {0};
            u32 gridFormatIndex = 0;
            u32 gridOffsetIndex = 0;
            for (int gridX = 0; gridX < 4; gridX++) {
                for (int gridZ = 0; gridZ < 4; gridZ++) {
                    for (int gridY = 0; gridY < 4; gridY++) {
                        int gridIndex = gridX * 16 + gridZ * 4 + gridY;
                        u8 blockGrid[128] = {0};
                        u8 sbmrgGrid[128] = {0};

                        u8 v1 = sectionHeader[gridIndex * 2];
                        u8 v2 = sectionHeader[gridIndex * 2 + 1];

                        u16 format = (v2 >> 4);
                        u16 offset = ((0x0f & v2) << 8 | v1) * 4;

                        // 0x4c for start and 0x80 for header (26 chunk header, 50 section header, 128 grid header)
                        u16 gridPosition = 0xcc + address + offset;

                        int offsetInBlockWrite = (section * 16 + gridY * 4) + gridZ * 1024 + gridX * 16384;

                        gridFormats[gridFormatIndex++] = format;
                        gridOffsets[gridOffsetIndex++] = gridPosition - 26;

                        // ensure not reading past the memory buffer
                        if EXPECT_FALSE (gridPosition + V12_GRID_SIZES[format] >= dataManager->size && format != 0) {
                            return;
                        }

                        u8* bufferPtr = dataManager->data + gridPosition;
                        dataManager->ptr = bufferPtr + V12_GRID_SIZES[format] + 128;
                        bool success = true;
                        switch(format) {
                            case V12_0_UNO:
                                for (int i = 0; i < 128; i += 2) {
                                    blockGrid[i] = v1;
                                    blockGrid[i + 1] = v2;
                                }
                                break;
                            case V12_1_BIT:
                                success = readGrid<1>(bufferPtr, blockGrid);
                                break;
                            case V12_1_BIT_SUBMERGED:
                                success = readGridSubmerged<1>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_2_BIT:
                                success = readGrid<2>(bufferPtr, blockGrid);
                                break;
                            case V12_2_BIT_SUBMERGED:
                                success = readGridSubmerged<2>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_3_BIT:
                                success = readGrid<3>(bufferPtr, blockGrid);
                                break;
                            case V12_3_BIT_SUBMERGED:
                                success = readGridSubmerged<3>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_4_BIT:
                                success = readGrid<4>(bufferPtr, blockGrid);
                                break;
                            case V12_4_BIT_SUBMERGED:
                                success = readGridSubmerged<4>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_8_FULL:
                                fillWithMaxBlocks(bufferPtr, blockGrid);
                                break;
                            case V12_8_FULL_BLOCKS_SUBMERGED:
                                fillWithMaxBlocks(bufferPtr, blockGrid);
                                fillWithMaxBlocks(bufferPtr + 128, sbmrgGrid);
                                break;
                            default: // this should never occur
                                return;
                        }

                        if EXPECT_FALSE (!success) {
                            return;
                        }
                        placeBlocks(chunkData->newBlocks, blockGrid, offsetInBlockWrite);
                        if (format & 1) {
                            chunkData->hasSubmerged = true;
                            placeBlocks(chunkData->submerged, sbmrgGrid, offsetInBlockWrite);
                        }
                    }
                }
            }
        }
        dataManager->seek(76 + maxSectionAddress);
    }


    /**
     * only parse the palette + positions.
     * DOES NOT PARSE LIQUID DATA
     * @tparam BitsPerBlock
     * @param buffer
     * @param grid
     * @return
     */
    template<size_t BitsPerBlock>
    bool ChunkV12::readGrid(const u8* buffer, u8 grid[128]) const {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int i = 0;
        while (i < 64) {
            u8 vBlocks[BitsPerBlock];

            int row = i / 8;
            int column = i % 8;

            for (int j = 0; j < BitsPerBlock; j++) {
                vBlocks[j] = buffer[size + row + j * 8];
            }

            u8 mask = 0b10000000 >> column;
            u16 idx = 0;

            for (int k = 0; k < BitsPerBlock; k++) {
                idx |= ((vBlocks[k] & mask) >> (7 - column)) << k;
            }

            if EXPECT_FALSE (idx >= size) {
                return false;
            }

            int gridIndex = i * 2;
            int paletteIndex = idx * 2;

            grid[gridIndex + 0] = palette[paletteIndex + 0];
            grid[gridIndex + 1] = palette[paletteIndex + 1];

            i++;
        }
        return true;
    }



    /**
     * parses the palette + positions + liquid data.
     *
     * @tparam BitsPerBlock
     * @param buffer
     * @param blockGrid
     * @param SbmrgGrid
     * @return
     */
    template<size_t BitsPerBlock>
    bool ChunkV12::readGridSubmerged(u8 const* buffer, u8 blockGrid[128], u8 SbmrgGrid[128]) const {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 vBlocks[BitsPerBlock];
            u8 vWaters[BitsPerBlock];

            // this loads the pieces into a buffer
            for (int j = 0; j < BitsPerBlock; j++) {
                int offset = size + i + j * 8;
                vBlocks[j] = buffer[offset];
                vWaters[j] = buffer[offset + BitsPerBlock * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 0b10000000 >> j;
                u16 idxBlock = 0;
                u16 idxSbmrg = 0;
                for (int k = 0; k < BitsPerBlock; k++) {
                    idxBlock |= ((vBlocks[k] & mask) >> (7 - j)) << k;
                    idxSbmrg |= ((vWaters[k] & mask) >> (7 - j)) << k;
                }

                if EXPECT_FALSE (idxBlock >= size || idxSbmrg >= size) {
                    return false;
                }

                int gridIndex = (i * 8 + j) * 2;
                int paletteIndexBlock = idxBlock * 2;
                int paletteIndexSbmrg = idxSbmrg * 2;
                blockGrid[gridIndex + 0] = palette[paletteIndexBlock + 0];
                blockGrid[gridIndex + 1] = palette[paletteIndexBlock + 1];
                SbmrgGrid[gridIndex + 0] = palette[paletteIndexSbmrg + 0];
                SbmrgGrid[gridIndex + 1] = palette[paletteIndexSbmrg + 1];
            }
        }
        return true;
    }


    /**
     * 0b10000000 = 128
     * 0b10000001 = 129
     * 0b11111111 = 255
     *
     * toIndex(num) = return num * 128 + 128;
     * java and lce supposedly store light data in the same way
     */
    void ChunkV12::readLightData() const {

        chunkData->DataGroupCount = 0;
        u8_vec_vec dataArray(4);
        for (int i = 0; i < 4; i++) {
            u32 num = dataManager->readInt32();
            u32 index = toIndex(num);
            // TODO: this is really slow, figure out how to remove it
            dataArray[i] = dataManager->readIntoVector(index);
            chunkData->DataGroupCount += dataArray[i].size();
        }

        auto processLightData = [](const u8_vec& data, u8_vec& lightData, int& offset) {
            for (int k = 0; k < 128; k++) {
                if (data[k] == 128) {
                    memset(&lightData[offset], 0, 128);
                } else if (data[k] == 129) {
                    memset(&lightData[offset], 255, 128);
                } else {
                    std::memcpy(&lightData[offset], &data[toIndex(data[k])], 128);
                }
                offset += 128;
            }
        };

        int writeOffset = 0;
        processLightData(dataArray[0], chunkData->skyLight, writeOffset);
        processLightData(dataArray[1], chunkData->skyLight, writeOffset);
        writeOffset = 0;
        processLightData(dataArray[2], chunkData->blockLight, writeOffset);
        processLightData(dataArray[3], chunkData->blockLight, writeOffset);
    }


    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV12::writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim) {
        dataManager = managerOut;
        chunkData = chunkDataIn;

        dataManager->writeInt32(chunkData->chunkX);
        dataManager->writeInt32(chunkData->chunkZ);
        dataManager->writeInt64(chunkData->lastUpdate);
        dataManager->writeInt64(chunkData->inhabitedTime);

        writeBlockData();
        writeLightData();

        dataManager->writeBytes(chunkData->heightMap.data(), 256);
        dataManager->writeInt16(chunkData->terrainPopulated);
        dataManager->writeBytes(chunkData->biomes.data(), 256);

        if (chunkData->NBTData != nullptr) {
            NBT::writeTag(chunkData->NBTData, *dataManager);
        }

    }


    void ChunkV12::writeBlockData() const {

        std::vector<u16> blockVector;
        std::vector<u16> blockLocations;
        u16 gridHeader[64];
        u16 sectJumpTable[16] = {0};
        u8 sectSizeTable[16] = {0};
        u8 blockMap[65536] = {0};

        blockVector.reserve(64);
        blockLocations.reserve(64);

        // header ptr offsets from start
        const u32 H_BEGIN           =           26;
        const u32 H_SECT_JUMP_TABLE = H_BEGIN +  2; // step 2: i16 * 16 section jump table
        const u32 H_SECT_SIZE_TABLE = H_BEGIN + 34; // step 3:  i8 * 16 section size table / 256
        const u32 H_SECT_START      = H_BEGIN + 50;
        const u32 V_GRID_SIZE       =          128;

        /// increment 50 for block header
        dataManager->seek(H_SECT_START);

        u32 last_section_jump = 0;
        u32 last_section_size;

        for (u32 sectionIndex = 0; sectionIndex < 16; sectionIndex++) {
            const u32 CURRENT_INC_SECT_JUMP = last_section_jump * 256;
            const u32 CURRENT_SECTION_START = H_SECT_START + CURRENT_INC_SECT_JUMP;
            u32 sectionSize = 0;
            u32 gridIndex = 0;

            sectJumpTable[sectionIndex] = CURRENT_INC_SECT_JUMP;

            dataManager->ptr = dataManager->data + H_SECT_START + CURRENT_INC_SECT_JUMP + V_GRID_SIZE;

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

                        u16 gridFormat, gridID;
                        switch (blockVector.size()) {
                            case  1: gridFormat = V12_0_UNO; gridID = blockVector[0]; blockMap[blockVector[0]] = 0; goto SWITCH_END;
                            case  2: gridFormat = V12_1_BIT; writeGrid<1,  2, 0>(blockVector, blockLocations, blockMap); break;
                            case  3: gridFormat = V12_2_BIT; writeGrid<2,  3, 1>(blockVector, blockLocations, blockMap); break;
                            case  4: gridFormat = V12_2_BIT; writeGrid<2,  4, 0>(blockVector, blockLocations, blockMap); break;
                            case  5: gridFormat = V12_3_BIT; writeGrid<3,  5, 3>(blockVector, blockLocations, blockMap); break;
                            case  6: gridFormat = V12_3_BIT; writeGrid<3,  6, 2>(blockVector, blockLocations, blockMap); break;
                            case  7: gridFormat = V12_3_BIT; writeGrid<3,  7, 1>(blockVector, blockLocations, blockMap); break;
                            case  8: gridFormat = V12_3_BIT; writeGrid<3,  8, 0>(blockVector, blockLocations, blockMap); break;
                            case  9: gridFormat = V12_4_BIT; writeGrid<4,  9, 7>(blockVector, blockLocations, blockMap); break;
                            case 10: gridFormat = V12_4_BIT; writeGrid<4, 10, 6>(blockVector, blockLocations, blockMap); break;
                            case 11: gridFormat = V12_4_BIT; writeGrid<4, 11, 5>(blockVector, blockLocations, blockMap); break;
                            case 12: gridFormat = V12_4_BIT; writeGrid<4, 12, 4>(blockVector, blockLocations, blockMap); break;
                            case 13: gridFormat = V12_4_BIT; writeGrid<4, 13, 3>(blockVector, blockLocations, blockMap); break;
                            case 14: gridFormat = V12_4_BIT; writeGrid<4, 14, 2>(blockVector, blockLocations, blockMap); break;
                            case 15: gridFormat = V12_4_BIT; writeGrid<4, 15, 1>(blockVector, blockLocations, blockMap); break;
                            case 16: gridFormat = V12_4_BIT; writeGrid<4, 16, 0>(blockVector, blockLocations, blockMap); break;
                            default: gridFormat = V12_8_FULL; writeWithMaxBlocks(blockVector, blockLocations, blockMap); break;
                        }
                        gridID = (sectionSize / 4) | gridFormat << 12;
                    SWITCH_END:;
                        gridHeader[gridIndex++] = gridID;
                        sectionSize += V12_GRID_SIZES[gridFormat];

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
            dataManager->writeInt8AtOffset( H_SECT_SIZE_TABLE +     sectionIndex, sectSizeTable[sectionIndex]);
        }

        u32 final_val = sectJumpTable[15] + sectSizeTable[15] * 256;

        // at root header, write total file size
        dataManager->writeInt16AtOffset(H_BEGIN, final_val >> 8);
        dataManager->seek(H_SECT_START + final_val);
    }


    /**
     * Used to write only the palette and positions.\n
     * It does not write liquid data
     * 2: 1 |  2 | [_4] palette, [_8] positions
     * 4: 2 |  4 | [_8] palette, [16] positions
     * 6: 3 |  8 | [16] palette, [24] positions
     * 8: 4 | 16 | [32] palette, [32] positions
     * @tparam BitsPerBlock
     */
    template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
    void ChunkV12::writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8* blockMap) const {

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
    /// used to write full block data, instead of using palette.
    void ChunkV12::writeWithMaxBlocks(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[65536]) const {
        dataManager->setLittleEndian();
        for (size_t i = 0; i < 64; i++) {
            u16 blockPos = blockLocations[i];
            dataManager->writeInt16(blockVector[blockPos]);
        }
        dataManager->setBigEndian();

        for (u16 block : blockVector) {
            blockMap[block] = 0;
        }
    }


    void ChunkV12::writeLightSection(u32& readOffset, u8_vec& light) const {
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


    void ChunkV12::writeLightData() const {
        u32 readOffset = 0;
        writeLightSection(readOffset, chunkData->skyLight);
        writeLightSection(readOffset, chunkData->skyLight);
        readOffset = 0;
        writeLightSection(readOffset, chunkData->blockLight);
        writeLightSection(readOffset, chunkData->blockLight);
    }

}

