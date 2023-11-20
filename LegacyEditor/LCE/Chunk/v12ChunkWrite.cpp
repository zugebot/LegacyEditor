#include "v12Chunk.hpp"

#include "LegacyEditor/utils/time.hpp"
#include <random>
#include <set>


/**
 * This checks if the next 1024 bits are all zeros.\n
 * this is u8[128]
 * @param ptr
 * @return true if all bits are zero, else 0.
 */
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



/**
 * This checks if the next 1024 bits are all zeros.\n
 * this is u8[128]
 * @param ptr
 * @return true if all bits are zero, else 0.
 */
bool is0_128_slow(const u8* ptr) {
    for (int i = 0; i < 128; ++i) {
        if (ptr[i] != 0x00) {
            return false;
        }
    }
    return true;
}

bool is255_128_slow(const u8* ptr) {
    for (int i = 0; i < 128; ++i) {
        if (ptr[i] != 0xFF) {
            return false;
        }
    }
    return true;
}


namespace universal {


    void V12Chunk::writeChunk(DataManager& managerOut, DIM) {
        dataManager = managerOut;
        dataManager.writeInt16(12);
        dataManager.writeInt32(chunkData.chunkX);
        dataManager.writeInt32(chunkData.chunkZ);
        dataManager.writeInt64(chunkData.lastUpdate);
        dataManager.writeInt64(chunkData.inhabitedTime);

        auto t_start = getNanoSeconds();
        writeBlockData();
        auto t_end = getNanoSeconds();
        auto diff = t_end - t_start;
        printf("Block Write Time: %llu (%llums)\n\n", diff, diff / 1000000);

        u8* start = dataManager.ptr;
        writeLightData();
        u8* end = dataManager.ptr;
        printf("size=%d\n", dataManager.getPosition());
        dataManager.writeToFile(start, end - start, dir_path + "light_write.bin");

        dataManager.writeBytes(chunkData.heightMap.data(), 256);
        dataManager.writeInt16(chunkData.terrainPopulated);
        dataManager.writeBytes(chunkData.biomes.data(), 256);

        /*
        u8* NBT_START = dataManager.ptr;
        printf("\n");
        auto* compound1 = this->chunkData.NBTData->toType<NBTTagCompound>();
        for (const auto& tag : compound1->tagMap) {
            printf("%s: %s\n", tag.first.c_str(), tag.second.toString().c_str());
        }

        writeNBTData();

        dataManager.ptr = NBT_START;
        auto* compound2 = NBT::readTag(dataManager)->toType<NBTTagCompound>();

        printf("\n");
        for (const auto& tag : compound2->tagMap) {
            printf("%s: %s\n", tag.first.c_str(), tag.second.toString().c_str());
        }
        printf("\n");


        chunkData.NBTData = new NBTBase(new NBTTagCompound(), TAG_COMPOUND);
        auto* chunkRootNbtData = static_cast<NBTTagCompound*>(chunkData.NBTData->data);
        auto* entities = new NBTTagList();
        auto* tileEntities = new NBTTagList();
        auto* tileTicks = new NBTTagList();
        chunkRootNbtData->setListTag("Entities", entities);
        chunkRootNbtData->setListTag("TileEntities", tileEntities);
        chunkRootNbtData->setListTag("TileTicks", tileTicks);
        */
        writeNBTData();

    }



    void V12Chunk::writeBlockData() {

        std::vector<u16> blockVector;
        std::vector<u16> blockLocations;
        u8 blockMap[65536] = {0};

        blockVector.reserve(64);
        blockLocations.reserve(64);
        blockLocations.reserve(64);

        // header ptr offsets from start
        const u32 H_SECT_SIZE_TOTAL = dataManager.getPosition(); // step 1: i16 *  1 section size / 256
        const u32 H_SECT_JUMP_TABLE = H_SECT_SIZE_TOTAL + 2;     // step 2: i16 * 16 section jump table
        const u32 H_SECT_SIZE_TABLE = H_SECT_SIZE_TOTAL + 34;    // step 3:  i8 * 16 section size table / 256
        const u32 H_SECT_START = H_SECT_SIZE_TOTAL + 50;

        /// increment 50 for block header
        dataManager.incrementPointer(50);


        u32 last_section_jump = 0;
        u32 last_section_size = 0;

        for (u32 sectionIndex = 0; sectionIndex < 16; sectionIndex++) {
            const u32 sectorStart = H_SECT_START + 256 * last_section_jump;
            u32 sectionSize = 0;

            {
            u32 reducedSectionJump = last_section_jump * 256;
            u32 index = H_SECT_JUMP_TABLE + 2 * sectionIndex;
            dataManager.writeInt16AtOffset(index, reducedSectionJump);
            }

            // serialize grid to buffer
            // u16 gridFormats[64];
            // u16 gridOffsets[64];A
            // u32 gridFormatIndex = 0;
            // u32 gridOffsetIndex = 0;
            for (u32 gridX = 0; gridX < 4; gridX++) {
                for (u32 gridZ = 0; gridZ < 4; gridZ++) {
                    for (u32 gridY = 0; gridY < 4; gridY++) {
                        u32 gridIndex = gridY + 4 * gridZ + 16 * gridX;
                        blockVector.clear();
                        blockLocations.clear();

                        dataManager.ptr = dataManager.data + H_SECT_SIZE_TOTAL + 50 + 128 + last_section_jump * 256 + sectionSize;

                        u32 offsetInBlock = sectionIndex * 16 + gridY * 4 + gridZ * 1024 + gridX * 16384;

                        // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                        for (u32 blockZ = 0; blockZ < 4; blockZ++) {
                            for (u32 blockX = 0; blockX < 4; blockX++) {
                                for (u32 blockY = 0; blockY < 4; blockY++) {
                                    u32 blockIndex = offsetInBlock + blockY + blockZ * 256 + blockX * 4096;
                                    u16 block = chunkData.blocks[blockIndex];
                                    if (blockMap[block]) {
                                        blockLocations.push_back(blockMap[block] - 1);
                                    } else {
                                        blockMap[block] = blockVector.size() + 1;
                                        u16 location = blockVector.size();
                                        blockVector.push_back(block);
                                        blockLocations.push_back(location);
                                    }
                                    /*
                                    if (blockMap.contains(block)) {
                                        blockLocations.push_back(blockMap[block]);
                                    } else {
                                        blockMap[block] = blockVector.size();
                                        u16 location = blockVector.size();
                                        blockVector.push_back(block);
                                        blockLocations.push_back(location);
                                    }
                                     */
                                }
                            }
                        }

                        // hardcode blocks here for testing
                        // auto rng = std::default_random_engine {};
                        // std::shuffle(std::begin(blockVector), std::end(blockVector), rng);
                        // memset(blockLocations.data(), 0, 128);

                        u16 gridFormat;
                        u8 blockCount = blockVector.size();
                        u16 gridID;

                        if (blockCount == 1) {
                            gridFormat = 0x0000;
                            gridID = blockVector[0];
                        } else {
                            if (blockCount == 2) { // 1 bit
                                gridFormat = 0x0002;
                                writeLayer<1>(blockVector, blockLocations);
                            } else if (blockCount <= 4) { // 2 bit
                                gridFormat = 0x0004;
                                writeLayer<2>(blockVector, blockLocations);
                            } else if (blockCount <= 8) { // 3 bit
                                gridFormat = 0x0006;
                                writeLayer<3>(blockVector, blockLocations);
                            } else if (blockCount <= 16) { // 4 bit
                                gridFormat = 0x0008;
                                writeLayer<4>(blockVector, blockLocations);
                            } else {
                                gridFormat = 0x000e;
                                writeWithMaxBlocks(blockVector, blockLocations);
                            }
                            gridID = sectionSize >> 2 | gridFormat << 12;
                        }

                        for (u16 block : blockVector) {
                            blockMap[block] = 0;
                        }

                        // debugging
                        // gridFormats[gridFormatIndex++] = gridFormat;
                        // gridOffsets[gridOffsetIndex++] = dataManager.getPosition();

                        // write correct grid header
                        dataManager.setLittleEndian();
                        dataManager.writeInt16AtOffset(sectorStart + 2 * gridIndex, gridID);
                        dataManager.setBigEndian();

                        sectionSize += GRID_SIZES[gridFormat];


                    }
                }
            }

            // write section size to section size table
            u8* position = dataManager.data + sectorStart;
            u32 index = H_SECT_SIZE_TABLE + sectionIndex;
            if (is0_128_slow(position)) {
                dataManager.ptr -= 128;
                last_section_size = 0;
            } else {
                last_section_size = (128 + sectionSize + 255) / 256;
                last_section_jump += last_section_size;
            }
            dataManager.writeInt8AtOffset(index, last_section_size);

        }

        dataManager.writeInt16AtOffset(H_SECT_SIZE_TOTAL, last_section_jump + last_section_size);



    }


    /**
     * 2: 1 |  2 | [_4] palette, [_8] positions
     * 4: 2 |  4 | [_8] palette, [16] positions
     * 6: 3 |  8 | [16] palette, [24] positions
     * 8: 4 | 16 | [32] palette, [32] positions
     * @tparam BitsPerBlock
     */
    template<size_t BitsPerBlock>
    void V12Chunk::writeLayer(u16_vec& blocks, u16_vec& positions) {
        u32 palette_size = (1 << BitsPerBlock);
        dataManager.setLittleEndian();

        // write the block data
        for (u16 block : blocks) {
            dataManager.writeInt16(block);
        }
        // fill rest of empty palette with 0xFF's
        for (u64 rest = 0; rest < palette_size - blocks.size(); rest++) {
            dataManager.writeInt16(0xffff);
        }
        dataManager.setBigEndian();

        /*
         * write the position data
         *
         * so, write the first bit of each position, as a single u64,
         * then the second, third etc. N times,
         * where N is BitsPerBlock
         */
         for (u64 bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (u32 locIndex = 0; locIndex < 64; locIndex++) {
                u64 pos = positions[locIndex];
                position |= ((pos >> bitIndex) & 1) << (63 - locIndex);
            }
            dataManager.writeInt64(position);
         }
    }


    /// make this copy all u16 blocks from the grid location or whatnot
    void V12Chunk::writeWithMaxBlocks(u16_vec& blocks, u16_vec& positions) {
         dataManager.setLittleEndian();
         for (u32 i = 0; i < 64; i++) {
            u16 blockPos = positions[i];
            dataManager.writeInt16(blocks[blockPos]);
         }
         dataManager.setBigEndian();
    }

    void V12Chunk::writeLightSection(u8_vec& light, int& readOffset) {
        sectionOffsets.clear();

        // Write headers
        u8* ptr = light.data() + readOffset;
        for (int i = 0; i < 128; i++) {
            if (is0_128_slow(ptr)) {
                dataManager.writeInt8(128);
            } else if (is255_128_slow(ptr)) {
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

        // printf("%d: size=%d\n", index, dataManager.getPosition());
        // dataManager.writeToFile(dataManager.data, dataManager.getPosition(), dir_path + "light_" + std::to_string(index) + ".bin");
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