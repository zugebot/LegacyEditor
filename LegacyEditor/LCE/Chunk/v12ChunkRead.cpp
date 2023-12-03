#include "LegacyEditor/utils/time.hpp"
#include "v12Chunk.hpp"



namespace universal {


    static inline u32 toIndex(u32 num) {
        return (num + 1) * 128;
    }


    void V12Chunk::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
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

        /*
        // u8* start = dataManager->ptr;
        // auto t_start = getNanoSeconds();
        // auto t_end = getNanoSeconds();
        // u8* end = dataManager->ptr;
        // dataManager->writeToFile(start, end - start, dir_path + "block_read.bin");
        // auto diff = t_end - t_start;
        // printf("Block Read Time: %llu (%fms)\n", diff, float(diff) / 1000000.0F);
         */

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


    void V12Chunk::readBlockData() const {
        // u8* start = dataManager->ptr;

        u32 maxSectionAddress = dataManager->readInt16() << 8;
        // dataManager->writeToFile(start, 50 + maxSectionAddress, dir_path + "block_read.bin");


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
                            case V12_0_SINGLE:
                                for (int i = 0; i < 128; i += 2) {
                                    blockGrid[i] = v1;
                                    blockGrid[i + 1] = v2;
                                }
                                break;
                            case V12_1_BIT:
                                success = readGrid<1>(bufferPtr, blockGrid);
                                break;
                            case V12_1_BIT_SUBMERGED:
                                chunkData->hasSubmerged = true;
                                success = readGridSubmerged<1>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_2_BIT:
                                success = readGrid<2>(bufferPtr, blockGrid);
                                break;
                            case V12_2_BIT_SUBMERGED:
                                chunkData->hasSubmerged = true;
                                success = readGridSubmerged<2>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_3_BIT:
                                success = readGrid<3>(bufferPtr, blockGrid);
                                break;
                            case V12_3_BIT_SUBMERGED:
                                chunkData->hasSubmerged = true;
                                success = readGridSubmerged<3>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_4_BIT:
                                success = readGrid<4>(bufferPtr, blockGrid);
                                break;
                            case V12_4_BIT_SUBMERGED:
                                chunkData->hasSubmerged = true;
                                success = readGridSubmerged<4>(bufferPtr, blockGrid, sbmrgGrid);
                                break;
                            case V12_8_FULL:
                                fillWithMaxBlocks(bufferPtr, blockGrid);
                                break;
                            case V12_8_FULL_BLOCKS_SUBMERGED:
                                chunkData->hasSubmerged = true;
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
    bool V12Chunk::readGrid(const u8* buffer, u8 grid[128]) const {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int i = 0;
        while (i < 64) {
            u8 vBlocks[BitsPerBlock];

            // Calculate row and column for current index
            int row = i / 8;
            int column = i % 8;

            // Load the pieces into a buffer
            for (int j = 0; j < BitsPerBlock; j++) {
                vBlocks[j] = buffer[size + row + j * 8];
            }

            u8 mask = 0b10000000 >> column;
            u16 idx = 0;

            // Calculate the index
            for (int k = 0; k < BitsPerBlock; k++) {
                idx |= ((vBlocks[k] & mask) >> (7 - column)) << k;
            }

            if EXPECT_FALSE (idx >= size) {
                return false;
            }

            // Calculate grid index and palette index
            int gridIndex = i * 2;
            int paletteIndex = idx * 2;

            // Assign values to the grid
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
    bool V12Chunk::readGridSubmerged(u8 const* buffer, u8 blockGrid[128], u8 SbmrgGrid[128]) const {
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
    void V12Chunk::readLightData() const {

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

}

