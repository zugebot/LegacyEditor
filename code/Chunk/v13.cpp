#include "v13.hpp"

#include "code/Chunk/helpers.hpp"
#include "common/nbt.hpp"


namespace editor::chunk {

    void ChunkV13::allocChunk() const {
        chunkData->DataGroupCount = 0;
        chunkData->newBlocks = u16_vec(65536);
        chunkData->submerged = u16_vec(65536);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        chunkData->heightMap = u8_vec(256);
        chunkData->biomes = u8_vec(256);
    }

    // #####################################################
    // #               Read Section
    // #####################################################


    void ChunkV13::readChunk(DataReader& reader) {
        allocChunk();

        maxGridAmount = reader.read<u16>();
        chunkData->chunkX = reader.read<i32>();
        chunkData->chunkZ = reader.read<i32>();
        chunkData->lastUpdate = reader.read<i64>();
        chunkData->inhabitedTime = reader.read<i64>();

        readBlockData(reader);

        {
        c_auto dataArray = fetchSections<4>(chunkData, reader);
        readSection(dataArray[0], &chunkData->skyLight[0]);
        readSection(dataArray[1], &chunkData->skyLight[16348]);
        readSection(dataArray[2], &chunkData->blockLight[0]);
        readSection(dataArray[3], &chunkData->blockLight[16348]);
        }

        reader.readBytes(256, chunkData->heightMap.data());
        chunkData->terrainPopulated = reader.read<i16>();
        reader.readBytes(256, chunkData->biomes.data());

        if (*reader.ptr() == 0x0A) {
            chunkData->oldNBTData.read(reader);
            auto* nbt = chunkData->oldNBTData.getTag("");
            chunkData->entities = nbt->extractTag("Entities").value_or(makeList(eNBT::COMPOUND));
            chunkData->tileEntities = nbt->extractTag("TileEntities").value_or(makeList(eNBT::COMPOUND));
            chunkData->tileTicks = nbt->extractTag("TileTicks").value_or(makeList(eNBT::COMPOUND));
            chunkData->oldNBTData = NBTBase();
        }

        chunkData->lastVersion = 13;
        chunkData->validChunk = true;
    }


    static void setBlocks(u16_vec& writeVec, c_u8* grid, int gridOffset) {
        int readOffset = 0;

        for (int zIter = 0; zIter < 4; zIter++) {
            for (int xIter = 0; xIter < 4; xIter++) {
                for (int yIter = 0; yIter < 4; yIter++) {
                    c_int blockOffset = toIndex<eBlockOrder::yXZy>(xIter,
                                                                   yIter,
                                                                   zIter);
                    c_u8 num1 = grid[readOffset++];
                    c_u8 num2 = grid[readOffset++];
                    writeVec[gridOffset + blockOffset] = static_cast<u16>(num1)
                                                         | static_cast<u16>(num2) << 8U;
                }
            }
        }
    }



    void ChunkV13::readBlockData(DataReader& reader) const {
        c_u32 maxSectionAddress = reader.read<u16>() << 8;

        u16_vec sectionJumpTable(16);
        for (u32 i = 0; i < 16; i++) {
            sectionJumpTable[i] = reader.read<u16>();
        }

        // size: 16
        c_u8* sizeOfSubChunks = reader.ptr();
        reader.skip<16>();

        if (maxSectionAddress == 0) {
            return;
        }

        for (int sectionY = 0; sectionY < 16; sectionY++) {
            c_u32 address = sectionJumpTable[sectionY];
            reader.seek(SECTION_HEADER_SIZE + DATA_HEADER_SIZE + address); // 28 chunk header + 50 sectionY header
            if (address == maxSectionAddress) {
                break;
            }
            if (sizeOfSubChunks[sectionY] == 0U) {
                continue;
            }
            // TODO: replace with telling cpu to cache that address, and use a ptr?
            c_u8* sectionHeader = reader.ptr(); // size: 128 bytes
            reader.skip<128>();
#ifdef DEBUG
            u16 gridFormats[64] = {0};
            u16 gridOffsets[64] = {0};
            u32 gridFormatIndex = 0;
            u32 gridOffsetIndex = 0;
#endif
            for (int gridX = 0; gridX < 4; gridX++) {
            for (int gridZ = 0; gridZ < 4; gridZ++) {
            for (int gridY = 0; gridY < 4; gridY++) {

                u8 blockGrid[GRID_SIZE] = {0};
                u8 sbmrgGrid[GRID_SIZE] = {0};
                c_int gridIndex = gridY + gridX * 4 + gridZ * 16;
                c_u8 blockLower = sectionHeader[gridIndex * 2];
                c_u8 blockUpper = sectionHeader[gridIndex * 2 + 1];
                c_u16 format = (blockUpper >> 4);
                c_u16 offset = ((0x0F & blockUpper) << 8 | blockLower) * 4;
                // 0x4c for start and 0x80 for header (26+2 chunk header, 50 sectionY header, 128 grid header)
                c_u16 gridPosition = 0xCE + address + offset;
                int gridOffset = toIndex<eBlockOrder::yXZy>(4 * gridX,
                                                            4 * gridY + 16 * sectionY,
                                                            4 * gridZ);
#ifdef DEBUG
                gridFormats[gridFormatIndex++] = format;
                gridOffsets[gridOffsetIndex++] = gridPosition - DATA_HEADER_SIZE;
#endif
                // ensure not reading past the memory buffer
                if EXPECT_FALSE (gridPosition + V13_GRID_SIZES[format] >= reader.size() && format != 0) return;

                const u8* bufferPtr = reader.data() + gridPosition;
                reader.seek(gridPosition + V13_GRID_SIZES[format] + 128);
                bool success = true;
                switch(format) {
                    case V13_0_UNO:
                        for (int i = 0; i < 128; i += 2) {
                            blockGrid[i + 0] = blockLower;
                            blockGrid[i + 1] = blockUpper;
                        }
                        break;
                    case V13_1_BIT:           success = readGrid<1>(bufferPtr, blockGrid); break;
                    case V13_1_BIT_SUBMERGED: success = readGridSubmerged<1>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V13_2_BIT:           success = readGrid<2>(bufferPtr, blockGrid); break;
                    case V13_2_BIT_SUBMERGED: success = readGridSubmerged<2>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V13_3_BIT:           success = readGrid<3>(bufferPtr, blockGrid); break;
                    case V13_3_BIT_SUBMERGED: success = readGridSubmerged<3>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V13_4_BIT:           success = readGrid<4>(bufferPtr, blockGrid); break;
                    case V13_4_BIT_SUBMERGED: success = readGridSubmerged<4>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V13_8_FULL:          fillAllBlocks<GRID_SIZE>(bufferPtr, blockGrid); break;
                    case V13_8_FULL_BLOCKS_SUBMERGED:
                        fillAllBlocks<GRID_SIZE>(bufferPtr +   0, blockGrid);
                        fillAllBlocks<GRID_SIZE>(bufferPtr + 128, sbmrgGrid);
                        break;
                    default: // this should never occur
                        return;
                }

                if EXPECT_FALSE (!success) {
                    return;
                }

                setBlocks(chunkData->newBlocks, blockGrid, gridOffset);
                if ((format & 1) != 0) {
                    chunkData->hasSubmerged = true;
                    setBlocks(chunkData->submerged, sbmrgGrid, gridOffset);
                }
            }
            }
            }
        }
        reader.seek(DATA_HEADER_SIZE + SECTION_HEADER_SIZE + maxSectionAddress);
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
    bool ChunkV13::readGrid(c_u8* buffer, u8 grid[128]) const {
        c_int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int index = 0; index < 64; index++) {
            u8 vBlocks[BitsPerBlock];

            c_int row = index / 8;
            c_int column = index % 8;
            for (u32 j = 0; j < BitsPerBlock; j++)
                vBlocks[j] = buffer[size + row + j * 8];

            u16 idx = 0;
            u8 mask = 0b10000000 >> column;
            for (u32 k = 0; k < BitsPerBlock; k++)
                idx |= ((vBlocks[k] & mask) >> (7 - column)) << k;

            if EXPECT_FALSE (idx >= size)
                return false;

            c_int gridIndex = index * 2;
            c_int paletteIndex = idx * 2;
            grid[gridIndex + 0] = palette[paletteIndex + 0];
            grid[gridIndex + 1] = palette[paletteIndex + 1];
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
    bool ChunkV13::readGridSubmerged(c_u8* buffer,
                                     u8 blockGrid[GRID_SIZE],
                                     u8 SbmrgGrid[GRID_SIZE]) const {
        c_int size = (1 << BitsPerBlock) * 2;
        u16_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        for (int i = 0; i < 8; i++) {
            u8 vBlocks[BitsPerBlock];
            u8 vWaters[BitsPerBlock];

            // this loads the pieces into a buffer
            for (u32 j = 0; j < BitsPerBlock; j++) {
                c_int offset = size + i + j * 8;
                vBlocks[j] = buffer[offset];
                vWaters[j] = buffer[offset + BitsPerBlock * 8];
            }

            for (int j = 0; j < 8; j++) {
                u8 mask = 0b10000000 >> j;
                u16 idxBlock = 0;
                u16 idxSbmrg = 0;
                for (u32 k = 0; k < BitsPerBlock; k++) {
                    idxBlock |= (vBlocks[k] & mask) >> (7 - j) << k;
                    idxSbmrg |= (vWaters[k] & mask) >> (7 - j) << k;
                }

                if EXPECT_FALSE (idxBlock >= size || idxSbmrg >= size) {
                    return false;
                }

                c_int gridIndex = (i * 8 + j) * 2;
                c_int paletteIndexBlock = idxBlock * 2;
                c_int paletteIndexSbmrg = idxSbmrg * 2;
                blockGrid[gridIndex + 0] = palette[paletteIndexBlock + 0];
                blockGrid[gridIndex + 1] = palette[paletteIndexBlock + 1];
                SbmrgGrid[gridIndex + 0] = palette[paletteIndexSbmrg + 0];
                SbmrgGrid[gridIndex + 1] = palette[paletteIndexSbmrg + 1];
            }
        }
        return true;
    }


    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV13::writeChunk(DataWriter& writer) {
        writer.write<u32>(chunkData->chunkX);
        writer.write<u32>(chunkData->chunkZ);
        writer.write<u64>(chunkData->lastUpdate);
        writer.write<u64>(chunkData->inhabitedTime);
        writer.write<u16>(this->maxGridAmount);

        writeBlockData(writer);

        writeSection(writer, &chunkData->skyLight[0]);
        writeSection(writer, &chunkData->skyLight[16348]);
        writeSection(writer, &chunkData->blockLight[0]);
        writeSection(writer, &chunkData->blockLight[16348]);

        writer.writeBytes(chunkData->heightMap.data(), 256);
        writer.write<u16>(chunkData->terrainPopulated);
        writer.writeBytes(chunkData->biomes.data(), 256);

        NBTBase nbt = makeCompound({
                {"", makeCompound(
                             {
                                     {"Entities", chunkData->entities },
                                     {"TileEntities", chunkData->tileEntities },
                                     {"TileTicks", chunkData->tileTicks },
                             }
                             )}
        });
        nbt.write(writer);
    }


    // TODO: garbage and bad!
    void ChunkV13::writeBlockData(DataWriter& writer) const {

        u16_vec blockVector;
        u16_vec blockLocations;
        u16 gridHeader[GRID_COUNT];
        u16 sectJumpTable[SECTION_COUNT] = {};
        u8 sectSizeTable[SECTION_COUNT] = {};
        u8 blockMap[MAP_SIZE] = {0};

        blockVector.reserve(GRID_COUNT);
        blockLocations.reserve(GRID_COUNT);

        // header ptr offsets from start
        // TODO: these will need to be tweaked!
        constexpr u32 H_BEGIN           =           26;
        constexpr u32 H_SECT_JUMP_TABLE = H_BEGIN +  2; // step 2: i16 * 16 section jump table
        constexpr u32 H_SECT_SIZE_TABLE = H_BEGIN + 34; // step 3:  i8 * 16 section size table / 256
        constexpr u32 H_SECT_START      = H_BEGIN + 50;

        /// skip 50 for block header
        writer.seek(H_SECT_START);

        u32 last_section_jump = 0;
        u32 last_section_size;

        for (u32 sectionIndex = 0; sectionIndex < SECTION_COUNT; sectionIndex++) {
            c_u32 CURRENT_INC_SECT_JUMP = last_section_jump * 256;
            c_u32 CURRENT_SECTION_START = H_SECT_START + CURRENT_INC_SECT_JUMP;
            u32 gridIndex = 0;
            u32 sectionSize = 0;

            sectJumpTable[sectionIndex] = CURRENT_INC_SECT_JUMP;

            writer.seek(H_SECT_START + CURRENT_INC_SECT_JUMP + GRID_SIZE);

            u32 blockX, blockY, blockZ;

            for (u32 gridX = 0; gridX < 65536; gridX += 16384) {
            for (u32 gridZ = 0; gridZ < 4096; gridZ += 1024) {
            for (u32 gridY = 0; gridY < 16; gridY += 4) {
                blockVector.clear(); blockLocations.clear();

                // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                c_u32 offsetInBlock = sectionIndex * 16 + gridY + gridZ + gridX;
                for (blockX = 0; blockX < 16384; blockX += 4096) {
                for (blockZ = 0; blockZ < 1024; blockZ += 256) {
                for (blockY = 0; blockY < 4; blockY++) {

                    c_u32 blockIndex = offsetInBlock + blockY + blockZ + blockX;

                    if (u16 block = chunkData->newBlocks[blockIndex]; blockMap[block]) {
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

                u16 gridID;
                u16 gridFormat;
                switch (blockVector.size()) {
                    case  1: gridFormat = V13_0_UNO; gridID = blockVector[0]; blockMap[blockVector[0]] = 0; goto SWITCH_END;

                    case  2: gridFormat = V13_1_BIT; writeGrid<1,  2, 0>(writer, blockVector, blockLocations, blockMap); break;

                    case  3: gridFormat = V13_2_BIT; writeGrid<2,  3, 1>(writer, blockVector, blockLocations, blockMap); break;
                    case  4: gridFormat = V13_2_BIT; writeGrid<2,  4, 0>(writer, blockVector, blockLocations, blockMap); break;

                    case  5: gridFormat = V13_3_BIT; writeGrid<3,  5, 3>(writer, blockVector, blockLocations, blockMap); break;
                    case  6: gridFormat = V13_3_BIT; writeGrid<3,  6, 2>(writer, blockVector, blockLocations, blockMap); break;
                    case  7: gridFormat = V13_3_BIT; writeGrid<3,  7, 1>(writer, blockVector, blockLocations, blockMap); break;
                    case  8: gridFormat = V13_3_BIT; writeGrid<3,  8, 0>(writer, blockVector, blockLocations, blockMap); break;

                    case  9: gridFormat = V13_4_BIT; writeGrid<4,  9, 7>(writer, blockVector, blockLocations, blockMap); break;
                    case 10: gridFormat = V13_4_BIT; writeGrid<4, 10, 6>(writer, blockVector, blockLocations, blockMap); break;
                    case 11: gridFormat = V13_4_BIT; writeGrid<4, 11, 5>(writer, blockVector, blockLocations, blockMap); break;
                    case 12: gridFormat = V13_4_BIT; writeGrid<4, 12, 4>(writer, blockVector, blockLocations, blockMap); break;
                    case 13: gridFormat = V13_4_BIT; writeGrid<4, 13, 3>(writer, blockVector, blockLocations, blockMap); break;
                    case 14: gridFormat = V13_4_BIT; writeGrid<4, 14, 2>(writer, blockVector, blockLocations, blockMap); break;
                    case 15: gridFormat = V13_4_BIT; writeGrid<4, 15, 1>(writer, blockVector, blockLocations, blockMap); break;
                    case 16: gridFormat = V13_4_BIT; writeGrid<4, 16, 0>(writer, blockVector, blockLocations, blockMap); break;

                    default: gridFormat = V13_8_FULL; writeWithMaxBlocks(writer, blockVector, blockLocations, blockMap); break;
                }
                gridID = sectionSize / 4 | gridFormat << 12;
            SWITCH_END:;
                gridHeader[gridIndex++] = gridID;
                sectionSize += V13_GRID_SIZES[gridFormat];

            }
            }
            }

            // write grid header in subsection
            writer.setEndian(Endian::Little);
            for (size_t index = 0; index < GRID_COUNT; index++) {
                writer.writeAtOffset<u16>(CURRENT_SECTION_START + 2 * index, gridHeader[index]);
            }
            writer.setEndian(Endian::Big);

            // write section size to section size table
            if (is_zero_128(writer.data() + CURRENT_SECTION_START)) {
                last_section_size = 0;
                writer.skip(-GRID_SIZE);
            } else {
                last_section_size = (GRID_SIZE + sectionSize + 255) / 256;
                last_section_jump += last_section_size;
            }
            sectSizeTable[sectionIndex] = last_section_size;
        }

        // at root header, write section jump and size tables
        for (size_t sectionIndex = 0; sectionIndex < SECTION_COUNT; sectionIndex++) {
            writer.writeAtOffset<u16>(H_SECT_JUMP_TABLE + 2 * sectionIndex, sectJumpTable[sectionIndex]);
            writer.writeAtOffset<u8>(H_SECT_SIZE_TABLE + sectionIndex, sectSizeTable[sectionIndex]);
        }

        c_u32 final_val = last_section_jump * 256;

        // at root header, write total file size
        writer.writeAtOffset<u16>(H_BEGIN, final_val >> 8);
        writer.seek(H_SECT_START + final_val);
    }


    /**
     * Used to writeGameData only the palette and positions.\n
     * It does not writeGameData liquid data
     * 2: 1 |  2 | [_4] palette, [_8] positions
     * 4: 2 |  4 | [_8] palette, [16] positions
     * 6: 3 |  8 | [16] palette, [24] positions
     * 8: 4 | 16 | [32] palette, [32] positions
     * @tparam BitsPerBlock
     */
    template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
    void ChunkV13::writeGrid(DataWriter& writer, u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const {

        // write the block data
        writer.setEndian(Endian::Little);
        for (size_t blockIndex = 0; blockIndex < BlockCount; blockIndex++) {
            writer.write<u16>(blockVector[blockIndex]);
        }
        writer.setEndian(Endian::Big);

        // fill rest of empty palette with 0xFF's
        // TODO: IDK if this is actually necessary
        for (size_t rest = 0; rest < EmptyCount; rest++) {
            writer.write<u16>(0xFFFF);
        }

        //  write the position data
        //  so, write the first bit of each position, as a single u64,
        //  then the second, third etc. N times, where N is BitsPerBlock
        for (size_t bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (size_t locIndex = 0; locIndex < GRID_COUNT; locIndex++) {
                c_u64 pos = blockLocations[locIndex];
                position |= (pos >> bitIndex & 1) << (GRID_COUNT - locIndex - 1);
            }
            writer.write<u64>(position);
        }

        // clear the table
        for (size_t i = 0; i < BlockCount; ++i) {
            blockMap[blockVector[i]] = 0;
        }

    }


    /// make this copy all u16 blocks from the grid location or whatnot
    /// used to writeGameData full block data, instead of using palette.
    void ChunkV13::writeWithMaxBlocks(DataWriter& writer, const u16_vec& blockVector, const u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const {
        writer.setEndian(Endian::Little);
        for (size_t i = 0; i < GRID_COUNT; i++) {
            c_u16 blockPos = blockLocations[i];
            writer.write<u16>(blockVector[blockPos]);
        }
        writer.setEndian(Endian::Big);

        for (c_u16 block : blockVector) {
            blockMap[block] = 0;
        }
    }

}

