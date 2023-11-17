#include "LegacyEditor/utils/time.hpp"
#include "v12Chunk.hpp"
#include <algorithm>



namespace universal {


    void V12Chunk::readChunk(DataManager& managerIn, DIM dim) {
        dataManager = managerIn;

        u8* start = dataManager.ptr;
        chunkData.chunkX = (i32) dataManager.readInt32();
        chunkData.chunkZ = (i32) dataManager.readInt32();
        chunkData.lastUpdate = (i64) dataManager.readInt64();
        chunkData.inhabitedTime = (i64) dataManager.readInt64();
        readBlocks();
        readLights();
        chunkData.heightMap = read256(dataManager);
        chunkData.terrainPopulated = (i16) dataManager.readInt16();
        chunkData.biomes = read256(dataManager);
        readNBTData();
        u8* end = dataManager.ptr;

        dataManager.writeToFile(start, end - start, dir_path + "chunk_read.bin");

    }


    void V12Chunk::readChunkForAccess(DataManager& managerIn, DIM dim) {
        dataManager = managerIn;
        dataManager.seek(26);
        readBlocks();
    }


    void V12Chunk::readNBTData() {
        if (*dataManager.ptr == 0xA) {
            chunkData.NBTData = NBT::readTag(dataManager); 
        }
    }


    /**
     * 0b10000000 = 128
     * 0b10000001 = 129
     * 0b11111111 = 255
     *
     * toIndex(num) = return num * 128 + 128;
     * java and lce supposedly store light data in the same way
     */
    void V12Chunk::readLights() {

        chunkData.skyLight = u8_vec(32768);
        chunkData.blockLight = u8_vec(32768);
        int writeOffset = 0;

        chunkData.DataGroupCount = 0;
        u8_vec_vec dataArray(4);
        for (int i = 0; i < 4; i++) {
            u32 num = (u32) dataManager.readInt32();
            int index = toIndex(num);
            dataArray[i] = dataManager.readIntoVector(index);
            chunkData.DataGroupCount += (u32)dataArray[i].size();
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

        // Process light data
        processLightData(dataArray[0], chunkData.skyLight, writeOffset);
        processLightData(dataArray[1], chunkData.skyLight, writeOffset);
        writeOffset = 0; // Reset offset for block light
        processLightData(dataArray[2], chunkData.blockLight, writeOffset);
        processLightData(dataArray[3], chunkData.blockLight, writeOffset);
    }


    void V12Chunk::readBlocks() {
        u8* START = dataManager.ptr;
        chunkData.blocks = u16_vec(131072);
        chunkData.submerged = u16_vec(131072);

        u32 maxSectionAddress = dataManager.readInt16() << 8;



        u16_vec sectionJumpTable(16); // read 16 shorts so 32 bytes
        for (int i = 0; i < 16; i++) {
            u16 address = dataManager.readInt16();
            sectionJumpTable[i] = address;
        }

        u8_vec sizeOfSubChunks = dataManager.readIntoVector(16);

        if (maxSectionAddress == 0) { return; }

        for (int section = 0; section < 16; section++) {
            int address = sectionJumpTable[section];
            dataManager.seek(76 + address); // 26 chunk header + 50 section header
            if (address == maxSectionAddress) { break; }
            if (!sizeOfSubChunks[section]) { continue; }
            u8_vec sectionHeader = dataManager.readIntoVector(128);

            u16 gridFormats[64] = {0};
            u16 gridOffsets[64] = {0};
            u32 gridFormatIndex = 0;
            u32 gridOffsetIndex = 0;
            for (int gridX = 0; gridX < 4; gridX++) {
                for (int gridZ = 0; gridZ < 4; gridZ++) {
                    for (int gridY = 0; gridY < 4; gridY++) {
                        int gridIndex = gridX * 16 + gridZ * 4 + gridY;
                        u16 grid[128];
                        u16 submergedData[128];

                        u8 v1 = sectionHeader[gridIndex * 2];
                        u8 v2 = sectionHeader[gridIndex * 2 + 1];

                        u16 format = (v2 >> 4);
                        u16 offset = ((0x0f & v2) << 8 | v1) * 4;


                        u16 gridPosition = 0xcc + address + offset; // 0x4c for start and 0x80 for header (26 chunk header, 50 section header, 128 grid header)

                        int offsetInBlockWrite = section * 32 + gridY * 8 + gridZ * 2048 + gridX * 32768;

                        gridFormats[gridFormatIndex++] = format;
                        gridOffsets[gridOffsetIndex++] = gridPosition - 26;

                        if (section == 0)
                            printf("%d: %d, %d\n", gridFormatIndex, format, gridPosition - 26);

                        // ensure not reading past the memory buffer
                        if EXPECT_FALSE (gridPosition + GRID_SIZES[format] >= dataManager.size) { return; }

                        u8* bufferPtr = dataManager.data + gridPosition;
                        switch(format) {
                            case 0x00: // literally just fill the grid
                                singleBlock(v1, v2, grid);
                                break;
                            case 0x02: // 1 bit
                                if EXPECT_FALSE (!parseLayer<1>(bufferPtr, grid)) { return; }
                                break;
                            case 0x03: // 1 bit + submerged
                                if EXPECT_FALSE (!parseWithLayers<1>(bufferPtr, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            case 0x04: // 2 bit
                                if EXPECT_FALSE (!parseLayer<2>(bufferPtr, grid)) { return; }
                                break;
                            case 0x05: // 2 bit + submerged
                                if EXPECT_FALSE (!parseWithLayers<2>(bufferPtr, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            case 0x06: // 3 bit
                                if EXPECT_FALSE (!parseLayer<3>(bufferPtr, grid)) { return; }
                                break;
                            case 0x07: // 3 bit + submerged
                                if EXPECT_FALSE (!parseWithLayers<3>(bufferPtr, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            case 0x08: // 4 bit
                                if EXPECT_FALSE (!parseLayer<4>(bufferPtr, grid)) { return; }
                                break;
                            case 0x09: // 4bit + submerged
                                if EXPECT_FALSE (!parseWithLayers<4>(bufferPtr, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            case 0x0e: // read 128 bytes for normal blocks
                                fillWithMaxBlocks(bufferPtr, grid);
                                break;
                            case 0x0f:
                                fillWithMaxBlocks(bufferPtr, grid);
                                fillWithMaxBlocks(bufferPtr + 128, submergedData);
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            default: // this should never occur
                                return;
                        }

                        putBlocks(chunkData.blocks, grid, offsetInBlockWrite);
                    } // end of gy
                } // end of gz
            } // end of gx
        } // end of section
    }

    
    void V12Chunk::putBlocks(u16_vec& writeVec, const u16* readArray, int writeOffset) {
        int readOffset = 0;
        for (int z = 0; z < 4; z++) {
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    int currentOffset = z * 8192 + x * 512 + y * 2;
                    writeVec[currentOffset + writeOffset] = readArray[readOffset++];
                    writeVec[currentOffset + writeOffset + 1] = readArray[readOffset++];
                }
            }
        }
    }



    /**
     *
     * @param buffer
     * @param grid u16[128]
     */
    void V12Chunk::singleBlock(u8 v1, u8 v2, u16* grid) {
        for (int i = 0; i < 128; i++) {
            if (i & 1) {
                grid[i] = v2;
            } else {
                grid[i] = v1;
            }
        }
    }

    /**
     *
     * @param buffer
     * @param grid u16[128]
     */
    void V12Chunk::fillWithMaxBlocks(const u8* buffer, u16* grid) {
        std::copy_n(buffer, 128, grid);
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
    bool V12Chunk::parseLayer(const u8* buffer, u16* grid) {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 vBlocks[BitsPerBlock];

            // BitsPerBlock = how many times to do shift?
            for (int j = 0; j < BitsPerBlock; j++) {
                vBlocks[j] = buffer[size + i + j * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 128 >> j;
                u16 idx = 0;

                for (int k = 0; k < BitsPerBlock; k++) {
                    idx |= ((vBlocks[k] & mask) >> (7 - j)) << k;
                }
                if EXPECT_FALSE (idx >= size) { return false; }
                int gridIndex = (i * 8 + j) * 2;
                int paletteIndex = idx * 2;
                grid[gridIndex] = palette[paletteIndex];
                grid[gridIndex + 1] = palette[paletteIndex + 1];
            }
        }
        return true;
    }
    
    /**
     * parses the palette + positions + liquid data.
     *
     * @tparam BitsPerBlock
     * @param buffer
     * @param grid
     * @param submergedGrid
     * @return
     */
    template<size_t BitsPerBlock>
    bool V12Chunk::parseWithLayers(u8 const* buffer, u16* grid, u16* submergedGrid) {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 vBlocks[BitsPerBlock];
            u8 vwaters[BitsPerBlock];

            // this loads the pieces into a buffer
            for (int j = 0; j < BitsPerBlock; j++) {
                int offset = size + i + j * 8;
                vBlocks[j] = buffer[offset];
                vwaters[j] = buffer[offset + BitsPerBlock * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 0x80 >> j;
                u16 idxBlock = 0;
                u16 idxWater = 0;
                for (int k = 0; k < BitsPerBlock; k++) {
                    idxBlock |= ((vBlocks[k] & mask) >> (7 - j)) << k;
                    idxWater |= ((vwaters[k] & mask) >> (7 - j)) << k;
                }

                if EXPECT_FALSE (idxBlock >= size || idxWater >= size) { return false; }

                int gridIndex = (i * 8 + j) * 2;
                int paletteIndex = idxBlock * 2;
                int paletteIndexSubmerged = idxWater * 2;
                grid[gridIndex] = palette[paletteIndex];
                grid[gridIndex + 1] = palette[paletteIndex + 1];
                submergedGrid[gridIndex] = palette[paletteIndexSubmerged];
                submergedGrid[gridIndex + 1] = palette[paletteIndexSubmerged + 1];
            }
        }
        return true;
    }

}

