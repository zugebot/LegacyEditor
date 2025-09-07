#include "chunkFormatGridPalettedSubmerged.hpp"

#include "code/Chunk/helpers/helpers.hpp"
#include "common/fixedVector.hpp"
#include "common/fmt.hpp"
#include "common/nbt.hpp"


namespace editor {

    // #####################################################
    // #               Read Section
    // #####################################################


    void ChunkFormatGridPalettedSubmerged::readChunk(ChunkData* chunkData, DataReader& reader) {
        if (chunkData->lastVersion == 13) {
            chunkData->maxGridCount = reader.read<u16>();
        }
        chunkData->chunkX = reader.read<i32>();
        chunkData->chunkZ = reader.read<i32>();
        chunkData->lastUpdate = reader.read<i64>();
        chunkData->inhabitedTime = reader.read<i64>();


        chunkData->blocks = u16_vec(65536);
        chunkData->submerged = u16_vec(65536);
        readBlockData(chunkData, reader);

        {
            c_auto dataArray = fetchSections<4>(chunkData, reader);
            chunkData->skyLight = u8_vec(32768);
            readSection(dataArray[0], &chunkData->skyLight[0]);
            readSection(dataArray[1], &chunkData->skyLight[16384]);
            chunkData->blockLight = u8_vec(32768);
            readSection(dataArray[2], &chunkData->blockLight[0]);
            readSection(dataArray[3], &chunkData->blockLight[16384]);
        }

        chunkData->heightMap = u8_vec(256);
        reader.readBytes(256, chunkData->heightMap.data());

        chunkData->terrainPopulatedFlags = reader.read<i16>();

        chunkData->biomes = u8_vec(256);
        reader.readBytes(256, chunkData->biomes.data());

        readNBT(chunkData, reader);

        chunkData->validChunk = true;
    }


    void ChunkFormatGridPalettedSubmerged::setBlocks(u16_vec& writeVec, c_u8* grid, MU int gridOffset) {
        int readOffset = 0;

        for (int z = 0; z < 4; z++) {
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    c_int blockOffset = toIndex<CANONICAL_BLOCK_ORDER>(x, y, z);
                    c_u8 num1 = grid[readOffset++];
                    c_u8 num2 = grid[readOffset++];
                    writeVec[gridOffset + blockOffset] = static_cast<u16>(num1)
                                                       | static_cast<u16>(num2) << 8U;
                }
            }
        }
    }


    void ChunkFormatGridPalettedSubmerged::readBlockData(ChunkData* chunkData, DataReader& reader) {
        u32 CHUNK_HEADER_SIZE = chunkData->lastVersion == eChunkVersion::V_13 ? 28 : 26;

        c_u32 maxSectionAddress = reader.read<u16>() << 8;
        c_u32 jumpWhenFinished = CHUNK_HEADER_SIZE + SECTION_HEADER_SIZE + maxSectionAddress;

        u16_vec sectionJumpTable(16);
        for (u32 i = 0; i < 16; i++) {
            sectionJumpTable[i] = reader.read<u16>();
        }

        c_u8* sizeOfSubChunks = reader.fetch<16>();

        if (maxSectionAddress == 0) {
            return;
        }

        for (int sectionY = 0; sectionY < 16; sectionY++) {
            c_u32 address = sectionJumpTable[sectionY];
            reader.seek(CHUNK_HEADER_SIZE + SECTION_HEADER_SIZE + address);
            if (address == maxSectionAddress) {
                break;
            }
            if (sizeOfSubChunks[sectionY] == 0U) {
                continue;
            }

            c_u8* sectionHeader = reader.fetch<GRID_SIZE>();

            for (int gridX = 0; gridX < 4; gridX++) {
            for (int gridZ = 0; gridZ < 4; gridZ++) {
            for (int gridY = 0; gridY < 4; gridY++) {
                int gridOffset = toIndex<CANONICAL_BLOCK_ORDER>(4 * gridX,
                                                                4 * gridY + 16 * sectionY,
                                                                4 * gridZ);

                u8 blockGrid[GRID_SIZE] = {};
                u8 sbmrgGrid[GRID_SIZE] = {};

                c_int gridIndex = gridY + gridX * 4 + gridZ * 16;
                c_u8 blockLower = sectionHeader[gridIndex * 2];
                c_u8 blockUpper = sectionHeader[gridIndex * 2 + 1];
                c_u16 format = blockUpper >> 4U;
                c_u16 offset = ((0x0fU & blockUpper) << 8 | blockLower) * 4;

                c_u16 gridPosition = CHUNK_HEADER_SIZE + SECTION_HEADER_SIZE + GRID_SIZE + address + offset;

                // ensure not reading past the memory buffer
                if EXPECT_FALSE (gridPosition + V12_GRID_SIZES[format] >= reader.size() && format != 0) {
                    cmn::log(cmn::eLog::error, "Tried to read chunk grid past buffer, leaving rest of chunk as air\n");
                    reader.seek(jumpWhenFinished);
                    return;
                }

                const u8* bufferPtr = reader.data() + gridPosition;
                reader.seek(gridPosition + V12_GRID_SIZES[format] + 128);

                bool success = true;
                switch(format) {
                    case V12_0_UNO:
                        for (int i = 0; i < GRID_SIZE; i += 2) {
                            blockGrid[i] = blockLower;
                            blockGrid[i + 1] = blockUpper;
                        }
                        break;
                    // TODO: no fucking clue what this is
                    case V12_0_UNO_SUBMERGED:
                        for (int i = 0; i < GRID_SIZE; i += 2) {
                            blockGrid[i] = blockLower;
                            blockGrid[i + 1] = blockUpper;
                        }
                        break;
                    case V12_1_BIT:           success = readGrid<1>(bufferPtr, blockGrid); break;
                    case V12_1_BIT_SUBMERGED: success = readGridSubmerged<1>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V12_2_BIT:           success = readGrid<2>(bufferPtr, blockGrid); break;
                    case V12_2_BIT_SUBMERGED: success = readGridSubmerged<2>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V12_3_BIT:           success = readGrid<3>(bufferPtr, blockGrid); break;
                    case V12_3_BIT_SUBMERGED: success = readGridSubmerged<3>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V12_4_BIT:           success = readGrid<4>(bufferPtr, blockGrid); break;
                    case V12_4_BIT_SUBMERGED: success = readGridSubmerged<4>(bufferPtr, blockGrid, sbmrgGrid); break;
                    case V12_8_FULL:          fillAllBlocks<GRID_SIZE>(bufferPtr, blockGrid); break;
                    case V12_8_FULL_SUBMERGED:
                        fillAllBlocks<GRID_SIZE>(bufferPtr, blockGrid);
                        fillAllBlocks<GRID_SIZE>(bufferPtr + 128, sbmrgGrid);
                        break;
                    default:
                        cmn::log(cmn::eLog::error, "Chunk grid with invalid format: {}\n", format);

                        reader.seek(jumpWhenFinished);
                        return;
                }

                if EXPECT_FALSE (!success) {
                    cmn::log(cmn::eLog::error, "GENERIC Writing chunk grid failed, leaving rest of chunk blocks as air\n");
                    reader.seek(jumpWhenFinished);
                    return;
                }

                setBlocks(chunkData->blocks, blockGrid, gridOffset);
                if ((format & 1U) != 0) {
                    chunkData->intel.hasSubmerged = true;
                    setBlocks(chunkData->submerged, sbmrgGrid, gridOffset);
                }
            }}}
        }
        reader.seek(jumpWhenFinished);
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
    bool ChunkFormatGridPalettedSubmerged::readGrid(c_u8* buffer, u8 grid[GRID_SIZE]) {
        c_int size = (1U << BitsPerBlock) * 2;
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
    bool ChunkFormatGridPalettedSubmerged::readGridSubmerged(u8 const* buffer,
                                     u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]) {
        c_int size = (1U << BitsPerBlock) * 2;
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
                u8 mask = 0b10000000U >> j;
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
                auto sub1 = palette[paletteIndexSbmrg + 0];
                auto sub2 = palette[paletteIndexSbmrg + 1];
                SbmrgGrid[gridIndex + 0] = sub1;
                SbmrgGrid[gridIndex + 1] = sub2;
            }
        }
        return true;
    }


    // #####################################################
    // #               Write Section
    // #####################################################


    MU void ChunkFormatGridPalettedSubmerged::writeChunkInternal(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer, bool fastMode) {
        if (settings.m_schematic.chunk_lastVersion == 13) {
            writer.write<u16>(chunkData->maxGridCount);
        }
        writer.write<i32>(chunkData->chunkX);
        writer.write<i32>(chunkData->chunkZ);
        writer.write<i64>(chunkData->lastUpdate);
        writer.write<i64>(chunkData->inhabitedTime);

        writeBlockData(chunkData, settings, writer);

        writeSection(writer, &chunkData->skyLight[0]);
        writeSection(writer, &chunkData->skyLight[16384]);
        writeSection(writer, &chunkData->blockLight[0]);
        writeSection(writer, &chunkData->blockLight[16384]);

        writer.writeBytes(chunkData->heightMap.data(), 256);
        writer.write<u16>(chunkData->terrainPopulatedFlags);
        writer.writeBytes(chunkData->biomes.data(), 256);

        writeNBT(chunkData, writer);
    }


    void ChunkFormatGridPalettedSubmerged::writeBlockData(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer) {
        if (chunkData->blocks.size() != 65536) {
            chunkData->blocks = u16_vec(65536);
        }
        if (chunkData->submerged.size() != 65536) {
            chunkData->submerged = u16_vec(65536);
        }

        u16FixVec_t blockVector;
        u16FixVec_t blockLocations;
        u16FixVec_t sbmgdLocations;
        u8 blockMap[MAP_SIZE] = {};

        u16 gridHeader[GRID_COUNT];
        u16 sectJumpTable[SECTION_COUNT] = {};
        u8 sectSizeTable[SECTION_COUNT] = {};


        // header ptr offsets from start
        u32 CHUNK_HEADER_SIZE = settings.m_schematic.chunk_lastVersion == eChunkVersion::V_13 ? 28 : 26;
        u32 H_SECT_JUMP_TABLE = CHUNK_HEADER_SIZE +  2; // step 2: i16 * 16 section jump table
        u32 H_SECT_SIZE_TABLE = CHUNK_HEADER_SIZE + 34; // step 3:  i8 * 16 section size table / 256
        u32 H_SECT_START      = CHUNK_HEADER_SIZE + 50;

        /// skip 50 for block header
        writer.seek(H_SECT_START);

        u32 last_section_jump = 0;
        u32 last_section_size;

        for (i32 sectionY = 0; sectionY < SECTION_COUNT; sectionY++) {
            c_u32 currentSectionOffset = last_section_jump * 256;
            c_u32 CURRENT_SECTION_START = H_SECT_START + currentSectionOffset;
            u32 sectionSize = 0;
            u32 gridIndex = 0;

            sectJumpTable[sectionY] = currentSectionOffset;
            writer.seek(H_SECT_START + currentSectionOffset + GRID_SIZE);

            for (i32 gridZ = 0; gridZ < 4; gridZ++) {
                for (i32 gridX = 0; gridX < 4; gridX++) {
                    for (i32 gridY = 0; gridY < 4; gridY++) {

                        blockVector.set_size(0);
                        blockLocations.set_size(0);
                        sbmgdLocations.set_size(0);

                        // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                        MU bool noSubmerged;

                        int gridOffset = toIndex<CANONICAL_BLOCK_ORDER>(4 * gridX,
                                                                        4 * gridY + 16 * sectionY,
                                                                        4 * gridZ);

                        for (i32 blockZ = 0; blockZ < 4; blockZ++) {
                            for (i32 blockX = 0; blockX < 4; blockX++) {
                                for (i32 blockY = 0; blockY < 4; blockY++) {

                                    c_u32 blockIndex = gridOffset + toIndex<CANONICAL_BLOCK_ORDER>(blockX, blockY, blockZ);

                                    u16 block = chunkData->blocks[blockIndex];
                                    if (blockMap[block]) {
                                        blockLocations.push_back(blockMap[block] - 1);
                                    } else {
                                        blockMap[block] = blockVector.current_size() + 1;
                                        u16 location = blockVector.current_size();
                                        blockVector.push_back(block);
                                        blockLocations.push_back(location);
                                    }

                                    u16 subBlock = chunkData->submerged[blockIndex];
                                    if EXPECT_FALSE(subBlock != 0) {
                                        noSubmerged = false;
                                        if (blockMap[subBlock] != 0) {
                                            sbmgdLocations.push_back(blockMap[subBlock] - 1);
                                        } else {
                                            blockMap[subBlock] = blockVector.current_size() + 1;
                                            u16 location = blockVector.current_size();
                                            blockVector.push_back(subBlock);
                                            sbmgdLocations.push_back(location);
                                        }
                                    } else {
                                        sbmgdLocations.push_back(0);
                                    }



                                }
                            }
                        }

                        // TODO: handle new code for writing submerged
                        u16 gridID;
                        u16 gridFormat;
                        if (true /*noSubmerged*/) {
                            switch (blockVector.current_size()) {
                                case  1: gridFormat = V12_0_UNO; gridID = blockVector[0]; blockMap[blockVector[0]] = 0; goto SWITCH_END;
                                case  2: gridFormat = V12_1_BIT; writeGrid<1,  2, 0>(writer, blockVector, blockLocations, blockMap); break;

                                case  3: gridFormat = V12_2_BIT; writeGrid<2,  3, 1>(writer, blockVector, blockLocations, blockMap); break;
                                case  4: gridFormat = V12_2_BIT; writeGrid<2,  4, 0>(writer, blockVector, blockLocations, blockMap); break;

                                case  5: gridFormat = V12_3_BIT; writeGrid<3,  5, 3>(writer, blockVector, blockLocations, blockMap); break;
                                case  6: gridFormat = V12_3_BIT; writeGrid<3,  6, 2>(writer, blockVector, blockLocations, blockMap); break;
                                case  7: gridFormat = V12_3_BIT; writeGrid<3,  7, 1>(writer, blockVector, blockLocations, blockMap); break;
                                case  8: gridFormat = V12_3_BIT; writeGrid<3,  8, 0>(writer, blockVector, blockLocations, blockMap); break;

                                case  9: gridFormat = V12_4_BIT; writeGrid<4,  9, 7>(writer, blockVector, blockLocations, blockMap); break;
                                case 10: gridFormat = V12_4_BIT; writeGrid<4, 10, 6>(writer, blockVector, blockLocations, blockMap); break;
                                case 11: gridFormat = V12_4_BIT; writeGrid<4, 11, 5>(writer, blockVector, blockLocations, blockMap); break;
                                case 12: gridFormat = V12_4_BIT; writeGrid<4, 12, 4>(writer, blockVector, blockLocations, blockMap); break;
                                case 13: gridFormat = V12_4_BIT; writeGrid<4, 13, 3>(writer, blockVector, blockLocations, blockMap); break;
                                case 14: gridFormat = V12_4_BIT; writeGrid<4, 14, 2>(writer, blockVector, blockLocations, blockMap); break;
                                case 15: gridFormat = V12_4_BIT; writeGrid<4, 15, 1>(writer, blockVector, blockLocations, blockMap); break;
                                case 16: gridFormat = V12_4_BIT; writeGrid<4, 16, 0>(writer, blockVector, blockLocations, blockMap); break;

                                default: gridFormat = V12_8_FULL; writeWithMaxBlocks(writer, blockVector, blockLocations, blockMap); break;
                            }
                        } else {
                            switch (blockVector.current_size()) {
                                case  2: gridFormat = V12_1_BIT_SUBMERGED; writeGridSubmerged<1,  2, 0>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  3: gridFormat = V12_2_BIT_SUBMERGED; writeGridSubmerged<2,  3, 1>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  4: gridFormat = V12_2_BIT_SUBMERGED; writeGridSubmerged<2,  4, 0>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  5: gridFormat = V12_3_BIT_SUBMERGED; writeGridSubmerged<3,  5, 3>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  6: gridFormat = V12_3_BIT_SUBMERGED; writeGridSubmerged<3,  6, 2>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  7: gridFormat = V12_3_BIT_SUBMERGED; writeGridSubmerged<3,  7, 1>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  8: gridFormat = V12_3_BIT_SUBMERGED; writeGridSubmerged<3,  8, 0>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case  9: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4,  9, 7>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 10: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 10, 6>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 11: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 11, 5>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 12: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 12, 4>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 13: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 13, 3>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 14: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 14, 2>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 15: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 15, 1>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                case 16: gridFormat = V12_4_BIT_SUBMERGED; writeGridSubmerged<4, 16, 0>(writer, blockVector, blockLocations, sbmgdLocations, blockMap); break;
                                default: gridFormat = V12_8_FULL_SUBMERGED; writeWithMaxBlocks(writer, blockVector, blockLocations, blockMap);
                                    writeWithMaxBlocks(writer, blockVector, sbmgdLocations, blockMap); break;
                            }
                        }
                        gridID = sectionSize / 4 | gridFormat << 12U;
                    SWITCH_END:;
                        gridHeader[gridIndex++] = gridID;
                        sectionSize += V12_GRID_SIZES[gridFormat];

                    }

                }
            }


            // write grid header in subsection and
            // write section size to section size table
            if (!is_zero_128(reinterpret_cast<u8*>(gridHeader))) {
                writer.setEndian(Endian::Little);
                for (size_t index = 0; index < GRID_COUNT; index++) {
                    writer.writeAtOffset<u16>(CURRENT_SECTION_START + 2 * index, gridHeader[index]);
                }
                writer.setEndian(Endian::Big);
                last_section_size = (GRID_SIZE + sectionSize + 255) / 256;
                last_section_jump += last_section_size;
            } else {
                last_section_size = 0;
            }

            sectSizeTable[sectionY] = last_section_size;
        }

        // at root header, write section jump and size tables
        for (size_t sectionIndex = 0; sectionIndex < SECTION_COUNT; sectionIndex++) {
            writer.writeAtOffset<u16>(H_SECT_JUMP_TABLE + 2 * sectionIndex, sectJumpTable[sectionIndex]);
            writer.writeAtOffset<u8>(H_SECT_SIZE_TABLE + sectionIndex, sectSizeTable[sectionIndex]);
        }

        c_u32 final_val = last_section_jump * 256;

        // at root header, write total file size
        writer.writeAtOffset<u16>(CHUNK_HEADER_SIZE, final_val >> 8U);
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
    void ChunkFormatGridPalettedSubmerged::writeGrid(DataWriter& writer, u16FixVec_t& blockVector,
                             u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]) {

        // write the palette data
        writer.setEndian(Endian::Little);
        for (size_t blockIndex = 0; blockIndex < BlockCount; blockIndex++) {
            writer.write<u16>(blockVector[blockIndex]);
        }
        writer.setEndian(Endian::Big);

        // fill rest of empty palette with 0xFF's
        // TODO: IDK if this is actually necessary
        if constexpr (EmptyCount != 0) {
            for (size_t rest = 0; rest < EmptyCount; rest++) {
                writer.write<u16>(0xFFFF);
            }
        }

        //  write the position data
        //  so, write the first bit of each position, as a single u64,
        //  then the second, third etc. N times, where N is BitsPerBlock
        for (size_t bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (size_t locIndex = 0; locIndex < GRID_COUNT; locIndex++) {
                c_u64 pos = blockLocations[locIndex];
                position |= (pos >> bitIndex & 1U) << (GRID_COUNT - locIndex - 1);
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
    void ChunkFormatGridPalettedSubmerged::writeWithMaxBlocks(DataWriter& writer, const u16FixVec_t& blockVector,
                                      const u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]) {
        // write the palette data
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

    /**
     * Used to writeGameData only the palette and both block and sbmgd positions.\n
     * 2: 1 |  2 | [_4] palette, [_8] positions
     * 4: 2 |  4 | [_8] palette, [16] positions
     * 6: 3 |  8 | [16] palette, [24] positions
     * 8: 4 | 16 | [32] palette, [32] positions
     * @tparam BitsPerBlock
     */
    template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
    MU void ChunkFormatGridPalettedSubmerged::writeGridSubmerged(DataWriter& writer, u16FixVec_t& blockVector, u16FixVec_t& blockLocations,
                                      const u16FixVec_t& sbmrgLocations, u8 blockMap[MAP_SIZE]) {

        // write the palette data
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
                position |= (pos >> bitIndex & 1U) << (GRID_COUNT - locIndex - 1);
            }
            writer.write<u64>(position);
        }

        //  write the sbmgd data
        //  so, write the first bit of each position, as a single u64,
        //  then the second, third etc. N times, where N is BitsPerBlock
        for (size_t bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (size_t locIndex = 0; locIndex < GRID_COUNT; locIndex++) {
                c_u64 pos = sbmrgLocations[locIndex];
                position |= (pos >> bitIndex & 1U) << (GRID_COUNT - locIndex - 1);
            }
            writer.write<u64>(position);
        }

        // clear the table
        for (size_t i = 0; i < BlockCount; ++i) {
            blockMap[blockVector[i]] = 0;
        }


    }

}

