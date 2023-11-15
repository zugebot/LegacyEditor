#include "v12Chunk.hpp"
#include <set>


bool is0_128(u8* ptr) {
    u64* ptr64 = reinterpret_cast<u64*>(ptr);
    for (int i = 0; i < 16; ++i) {
        if (ptr64[i] != 0x0000000000000000) {
            return false;
        }
    }
    return true;
}

bool is255_128(u8* ptr) {
    u64* ptr64 = reinterpret_cast<u64*>(ptr);
    for (int i = 0; i < 16; ++i) {
        if (ptr64[i] != 0xFFFFFFFFFFFFFFFF) {
            return false;
        }
    }
    return true;
}


namespace universal {


    void V12Chunk::writeChunk(DataManager& managerOut, DIM) {
        dataManager = managerOut;

        dataManager.writeInt32(chunkData.chunkX);
        dataManager.writeInt32(chunkData.chunkZ);
        dataManager.writeInt64(chunkData.lastUpdate);
        dataManager.writeInt64(chunkData.inhabitedTime);
        writeBlockData();
        writeLightData();
        dataManager.writeBytes(chunkData.heightMap.data(), 256);
        dataManager.writeInt16(chunkData.terrainPopulated);
        dataManager.writeBytes(chunkData.biomes.data(), 256);
        writeNBTData();
    }


    /**
     * This is not meant to work yet, or even be fast.\n
     * This is my first attempt to get the logic documented on how to
     * write the chunk's block data.
     */
    void V12Chunk::writeBlockData() {

        std::vector<u16> blockVector;
        blockVector.reserve(64);
        std::vector<u16> blockLocations;
        blockLocations.reserve(64);
        std::unordered_map<u16, u8> blockMap;
        blockLocations.reserve(64);

        /// idk how to really use this
        u16 liquidTable[64];

        // header ptr offsets from start
        u32 V_sectionTotalSize = 0;
        const u32 H_SECT_SIZE_TOTAL = dataManager.getPosition(); // step 1: i16 *  1 section size / 256
        const u32 H_SECT_JUMP_TABLE = H_SECT_SIZE_TOTAL + 2;     // step 2: i16 * 16 section jump table
        const u32 H_SECT_SIZE_TABLE = H_SECT_SIZE_TOTAL + 34;    // step 3:  i8 * 16 section size table / 256

        dataManager.incrementPointer(50);
        for (i32 sectionIndex = 0; sectionIndex < 16; sectionIndex++) {

            const u32 H_SECT_START = dataManager.getPosition();

            /// increment 128 for 2bytes * 64 grid headers
            dataManager.incrementPointer(128);

            // serialize grid to buffer
            u32 V_sectionSize = 0;
            for (u32 gridX = 0; gridX < 4; gridX++) {
                for (u32 gridZ = 0; gridZ < 4; gridZ++) {
                    for (u32 gridY = 0; gridY < 4; gridY++) {
                        u32 gridIndex = gridY + 4 * gridZ + 16 * gridX;
                        u32 V_gridSize = 0;

                        blockVector.clear();
                        blockLocations.clear();
                        blockMap.clear();


                        /**
                         * iterate over blocks, count unique, remember locations to a set?
                         * using the unique block count, write the appropriate palette / positions / liquid data
                         * set and vector of blocks
                         * u8[64] table of palette locations
                         */
                        // TODO: these magic numbers are most definitely wrong :(
                        u32 offsetInBlock = sectionIndex * 32 + gridY * 8 + gridZ * 2048 + gridX * 32768;
                        /// used for incrementing blockLocations and submergedData(?)
                        u32 iterCount = 0;

                        /// if has liquid data, later on add 0x01 to format
                        bool hasLiquidData = false;

                        // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                        for (u32 blockZ = 0; blockZ < 4; blockZ++) {
                            for (u32 blockX = 0; blockX < 4; blockX++) {
                                for (u32 blockY = 0; blockY < 4; blockY++) {

                                    // get block
                                    u32 blockIndex = offsetInBlock + blockZ * 8192 + blockX * 512 + blockY * 2;
                                    u16 block = chunkData.blocks[blockIndex];
                                    // count it, evaluate it etc.
                                    if (!blockMap.contains(block)) {
                                        blockMap[block] = blockVector.size();
                                        blockVector.push_back(block);
                                    }
                                    //
                                    blockLocations[iterCount] = blockVector.size();

                                    // IDK how to write submerged data
                                    // WRITE IT HERE


                                    iterCount++;
                                }
                            }
                        }

                        /**
                         * using blockVector, blockMap, and blockLocations, construct the grid data
                         *
                         * convert blockCount and whether liquid data exists to format code
                         *    1 -> 0x00 (0)
                         *    2 -> 0x02 (2)
                         * 3- 4 -> 0x04 (4)
                         * 5- 8 -> 0x06 (6)
                         * 9-16 -> 0x08 (8)
                         * otherwise, 0x0e (14)
                         * if there is liquid data, add 1.
                         */
                        u8 blockCount = blockVector.size();
                        /// grid format
                        u8 format;
                        /// find the correct format of the chunk
                        if (blockCount == 1) {
                            format = 0x00;
                        } else if (blockCount == 2) {
                            format = 0x02;
                        } else if (blockCount <= 4) {
                            format = 0x04;
                        } else if (blockCount <= 8) {
                            format = 0x06;
                        } else if (blockCount <= 16) {
                            format = 0x08;
                        } else {
                            format = 0x0e;
                        }


                        if (hasLiquidData) {
                            format++;
                        }

                        u32 gridSize = GRID_SIZES[format];
                        switch(format) {
                            case 0x00: // data is stored as grid index?
                                // figure out from v12Chunk::readBlockData
                                break;
                            case 0x01: // NOT_USED
                                printf("something went wrong... format=%d", format);
                                break;
                            case 0x02: // 1 bit
                                writeLayer<1>();
                                break;
                            case 0x03: // 1 bit + submerged
                                writeLayers<1>();
                                break;
                            case 0x04: // 2 bit
                                writeLayer<2>();
                                break;
                            case 0x05: // 2 bit + submerged
                                writeLayers<2>();
                                break;
                            case 0x06: // 3 bit
                                writeLayer<3>();
                                break;
                            case 0x07: // 3 bit + submerged
                                writeLayers<3>();
                                break;
                            case 0x08: // 4 bit
                                writeLayer<4>();
                                break;
                            case 0x09: // 4 bit + submerged
                                writeLayers<4>();
                                break;
                            case 0x0e: // write blocks in place [128]
                                writeWithMaxBlocks();
                                break;
                            case 0x0f: // write blocks and liquid in place [256]
                                writeWithMaxBlocks(); // write block data
                                writeWithMaxBlocks(); // write liquid data
                                break;
                            default:
                                printf("something went wrong... format=%d", format);
                                break;
                        }






                        /// write correct header data
                        u16 gridID = 0; // do calculations... (format and offset where stored / 4)
                        dataManager.writeInt16AtOffset(H_SECT_START + 2 * gridIndex, gridID);

                        /// increment a size value of the buffer size of the section
                        V_sectionSize += V_gridSize;

                    } // end of gridY loop
                } // end of gridZ loop
            } // end of gridX loop

            // write correct header data
            /// step 1: write section jump to section jump table
            dataManager.writeInt16AtOffset(H_SECT_JUMP_TABLE + 2 * sectionIndex, H_SECT_START);
            /// step 2: write section size to section size table
            u8 sectionReducedSize = 0; // calculate...
            dataManager.writeInt8AtOffset(H_SECT_SIZE_TABLE + sectionIndex, sectionReducedSize);
            /// step 3: add section size to total section size
            V_sectionTotalSize += V_sectionSize;

        } // end of sector for loop

        /// write correct totalSectionSize (have to divide it by 256)
        V_sectionTotalSize = (V_sectionTotalSize + 255) / 256;
        u16 sectionTotalReducedSize = static_cast<u16>(V_sectionTotalSize);
        dataManager.writeInt16AtOffset(H_SECT_SIZE_TOTAL, sectionTotalReducedSize);
    }


    template<size_t BitsPerBlock>
    void V12Chunk::writeLayer() {

    }

    template<size_t BitsPerBlock>
    void V12Chunk::writeLayers() {

    }

    void V12Chunk::writeWithMaxBlocks() {

    }



    void V12Chunk::writeLightSection(u8_vec& light, int& readOffset) {
        std::vector<int> sectionOffsets; // To store offsets of sections

        // Write headers
        u8* ptr = light.data() + readOffset;
        for (int i = 0; i < 128; i++) {
            if (is0_128(ptr)) {
                dataManager.writeInt8(128);
            } else if (is255_128(ptr)) {
                dataManager.writeInt8(129);
            } else {
                sectionOffsets.push_back(readOffset);
                dataManager.writeInt8(sectionOffsets.size() - 1);
            }
            ptr += 128;
            readOffset += 128;
        }

        // Write light data sections
        for (int offset: sectionOffsets) {
            dataManager.writeBytes(&light[offset], 128);
        }
    }

    void V12Chunk::writeLight(int index, int& readOffset, u8_vec& light) {
        u8* startPtr = dataManager.ptr;
        dataManager.writeInt32(0); // Placeholder for size

        writeLightSection(light, readOffset);

        // Calculate and write the size
        u8* endPtr = dataManager.ptr;
        i64 size = (endPtr - startPtr - 4 - 128) / 128; // -4 to exclude the size field itself
        dataManager.ptr = startPtr;
        dataManager.writeInt32(static_cast<int>(size));
        dataManager.ptr = endPtr;

        printf("%d: size=%d\n", index, dataManager.getPosition());
        dataManager.writeToFile(dataManager.data, dataManager.getPosition(), dir_path + "light_" + std::to_string(index) + ".bin");
    }

    void V12Chunk::writeLightData() {
        int readOffset = 0;
        writeLight(0, readOffset, chunkData.skyLight);
        writeLight(1, readOffset, chunkData.skyLight);
        readOffset = 0;
        writeLight(2, readOffset, chunkData.blockLight);
        writeLight(3, readOffset, chunkData.blockLight);
    }

    void V12Chunk::writeNBTData() {
        if (chunkData.NBTData != nullptr) {
            NBT::writeTag(chunkData.NBTData, dataManager);
        }
    }
} // namespace universal