#include "v12Chunk.hpp"
#include <algorithm>


namespace universal {


    void V12Chunk::readChunk(DataManager& managerIn, DIM dim) {
        dataManager = managerIn;


        chunkData.chunkX = (i32) dataManager.readInt32();
        chunkData.chunkZ = (i32) dataManager.readInt32();
        chunkData.lastUpdate = (i64) dataManager.readInt64();
        chunkData.inhabitedTime = (i64) dataManager.readInt64();
        readBlocks();
        readLights();
        chunkData.heightMap = read256(dataManager);
        chunkData.terrainPopulated = (short) dataManager.readInt16();
        chunkData.biomes = read256(dataManager);
        readNBTData();
        // return convertLCE1_13RegionToUniversal(chunkData, dim);
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


    void V12Chunk::readLights() {
        u8_vec_vec dataArray;

        for (int i = 0; i < 4; i++) {
            u8_vec item = read128(dataManager);
            dataArray.push_back(item);
        }

        chunkData.DataGroupCount = (i32)(dataArray[0].size() + dataArray[1].size() + dataArray[2].size() + dataArray[3].size());
        
        int segments[4] = {0, 0, 1, 1};
        int offsets[4] = {0, 16384, 0, 16384};
        u8_vec_vec lightsData = {u8_vec(32768), u8_vec(32768)};

        for (int j = 0; j < 4; j++) {
            int startingIndex = offsets[j];
            int currentLightSegment = segments[j];
            u8_vec data = dataArray[j];

            for (int k = 0; k < 128; k++) {
                u8 headerValue = data[k];
                if (headerValue == 128 || headerValue == 0x81) {
                    copyByte128(lightsData[currentLightSegment],
                                k * 128 + startingIndex,
                                (headerValue == (u8) 0x80) ? (u8) 0 : (u8) 255);
                } else {
                    copyArray128(data,
                                 (int) ((headerValue + 1) * 128),
                                 lightsData[currentLightSegment], k * 128 + startingIndex);
                }
            }
        }

        // java stores skylight (and block-light?) the same way LCE does, it does not need to be converted
        chunkData.skyLight = lightsData[0];
        chunkData.blockLight = lightsData[1];
    }


    void V12Chunk::readBlocks() {
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
            dataManager.seek(76 + address);
            if (address == maxSectionAddress) { break; }
            if (!sizeOfSubChunks[section]) { continue; }
            u8_vec sectionHeader = dataManager.readIntoVector(128);

            for (int gx = 0; gx < 4; gx++) {
                for (int gz = 0; gz < 4; gz++) {
                    for (int gy = 0; gy < 4; gy++) {
                        int gridIndex = gx * 16 + gz * 4 + gy;

                        u8 v1 = sectionHeader[gridIndex * 2];
                        u8 v2 = sectionHeader[gridIndex * 2 + 1];

                        u16 offset = ((0x0f & v2) << 8 | v1) * 4;
                        u16 format = (v2 >> 4);




                        u16 grid[128];
                        u16 submergedData[128];
                        u16 gridPosition = 0xcc + address + offset; // 0x4c for start and 0x80 for header
                        int offsetInBlockWrite = section * 32 + gy * 8 + gz * 2048 + gx * 32768;

                        switch(format) {
                            case 0x00: {
                                singleBlock(v1, v2, grid);
                                break;
                            }
                            case 0x0f: {
                                if EXPECT_FALSE (gridPosition + 256 >= dataManager.size) { return; }
                                maxBlocks(dataManager.start() + gridPosition, grid);
                                maxBlocks(dataManager.start() + gridPosition + 128, submergedData);
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            }
                            case 0x0e: { // read 128 bytes for normal blocks
                                if EXPECT_FALSE (gridPosition + 128 >= dataManager.size) { return; }
                                maxBlocks(dataManager.start() + gridPosition, grid);
                                break;
                            }
                            case 0x02: { // 1 bit
                                if EXPECT_FALSE (gridPosition + 12 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!read<1>(dataManager.start() + gridPosition, grid)) { return; }
                                break;
                            }
                            case 0x03: { // 1 bit + submerged
                                if EXPECT_FALSE (gridPosition + 20 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!parseWithLayers<1>(dataManager.start() + gridPosition, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            }
                            case 0x04: { // 2 bit
                                if EXPECT_FALSE (gridPosition + 24 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!read<2>(dataManager.start() + gridPosition, grid)) { return; }
                                break;
                            }
                            case 0x05: { // 2 bit + submerged
                                if EXPECT_FALSE (gridPosition + 40 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!parseWithLayers<2>(dataManager.start() + gridPosition, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            }
                            case 0x06: { // 3 bit
                                if EXPECT_FALSE (gridPosition + 40 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!read<3>(dataManager.start() + gridPosition, grid)) { return; }
                                break;
                            }
                            case 0x07: { // 3 bit + submerged
                                if EXPECT_FALSE (gridPosition + 64 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!parseWithLayers<3>(dataManager.start() + gridPosition, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            }
                            case 0x08: { // 4 bit
                                if EXPECT_FALSE (gridPosition + 64 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!read<4>(dataManager.start() + gridPosition, grid)) { return; }
                                break;
                            }
                            case 0x09: { // 4bit + submerged
                                if EXPECT_FALSE (gridPosition + 96 >= dataManager.size) { return; }
                                if EXPECT_FALSE (!parseWithLayers<4>(dataManager.start() + gridPosition, grid, submergedData)) { return; }
                                putBlocks(chunkData.submerged, submergedData, offsetInBlockWrite);
                                break;
                            }
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

    
    void V12Chunk::singleBlock(u16 v1, u16 v2, u16* grid) {
        for (int i = 0; i < 128; i++) {
            if (i & 1) {
                grid[i] = v2;
            } else {
                grid[i] = v1;
            }
        }
    }

    
    void V12Chunk::maxBlocks(const u8* buffer, u16* grid) {
        std::copy_n(buffer, 128, grid);
    }
    
    
    template<size_t BitsPerBlock>
    bool V12Chunk::read(const u8* buffer, u16* grid) {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 v[BitsPerBlock];

            for (int j = 0; j < BitsPerBlock; j++) {
                v[j] = buffer[size + i + j * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 128 >> j;
                u16 idx = 0;

                for (int k = 0; k < BitsPerBlock; k++) {
                    idx |= ((v[k] & mask) >> (7 - j)) << k;
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
    
    
    
    template<size_t BitsPerBlock>
    bool V12Chunk::parseWithLayers(u8 const* buffer, u16* grid, u16* submergedGrid) {
        int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 v[BitsPerBlock];
            u8 vSubmerged[BitsPerBlock];

            for (int j = 0; j < BitsPerBlock; j++) {
                int offset = size + i + j * 8;
                v[j] = buffer[offset];
                vSubmerged[j] = buffer[offset + BitsPerBlock * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 128 >> j;
                u16 idx = 0;
                u16 idxSubmerged = 0;
                for (int k = 0; k < BitsPerBlock; k++) {
                    idx |= ((v[k] & mask) >> (7 - j)) << k;
                    idxSubmerged |= ((vSubmerged[k] & mask) >> (7 - j)) << k;
                }

                if EXPECT_FALSE (idx >= size || idxSubmerged >= size) { return false; }
                int gridIndex = (i * 8 + j) * 2;
                int paletteIndex = idx * 2;
                int paletteIndexSubmerged = idxSubmerged * 2;
                grid[gridIndex] = palette[paletteIndex];
                grid[gridIndex + 1] = palette[paletteIndex + 1];
                submergedGrid[gridIndex] = palette[paletteIndexSubmerged];
                submergedGrid[gridIndex + 1] = palette[paletteIndexSubmerged + 1];
            }
        }
        return true;
    }
    
    
    
    
}

