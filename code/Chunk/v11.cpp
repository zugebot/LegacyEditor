#include "v11.hpp"

#include <cstring>

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/helpers.hpp"
#include "common/nbt.hpp"


// TODO: I think I need to rewrite this all to place blocks as only u8's,
// TODO: and to switch it to use oldBlocks instead of newBlocks

namespace editor::chunk {


    void ChunkV11::allocChunk() const {
        chunkData->oldBlocks = u8_vec(65536);
        chunkData->blockData = u8_vec(32768);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        chunkData->heightMap = u8_vec(256);
        chunkData->biomes = u8_vec(256);
    }


    // #####################################################
    // #               Read Section
    // #####################################################


    void ChunkV11::readChunk() const {
        allocChunk();

        chunkData->chunkX = dataManager->read<i32>();
        chunkData->chunkZ = dataManager->read<i32>();
        chunkData->lastUpdate = dataManager->read<i64>();

        chunkData->DataGroupCount = 0;
        if (chunkData->lastVersion > 8) {
            chunkData->inhabitedTime = dataManager->read<i64>();
        }

        readBlockData();

        c_auto dataArray = readGetDataBlockVector<6>(chunkData, dataManager);

        readDataBlock(dataArray[0], &chunkData->blockData[0]);
        readDataBlock(dataArray[1], &chunkData->blockData[16384]);
        readDataBlock(dataArray[2], &chunkData->skyLight[0]);
        readDataBlock(dataArray[3], &chunkData->skyLight[16384]);
        readDataBlock(dataArray[4], &chunkData->blockLight[0]);
        readDataBlock(dataArray[5], &chunkData->blockLight[16384]);

        dataManager->readBytes(256, chunkData->heightMap.data());
        chunkData->terrainPopulated = dataManager->read<i16>();
        dataManager->readBytes(256, chunkData->biomes.data());

        if (*dataManager->ptr() == 0x0A) {
            chunkData->oldNBTData.read(*dataManager);
        }

        chunkData->validChunk = true;
    }



    static int calcOffset(int value) {
        int num = value / 32;
        int num2 = value % 32;
        return num / 4 * 64 + num % 4 * 4 + num2 * 1024;
    }

    // xzy or zxy?
    /**
     *
     * @param writeVec
     * @param grid
     * @param writeOffset ALWAYS 32768 or 65536
     * @param readOffset 1 to 64
     */
    static void putBlocks(u8_vec& writeVec,  c_u8* grid,
                          c_int writeOffset, c_int readOffset) {
        int num = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    int num2 = readOffset + i * 16 + j + k * 256;
                    writeVec[num2 + writeOffset] = grid[num++];
                }
            }
        }
    }


    /**
     * the data is stored in the m_order of
     * [ byte2 | byte1 ]
     * but for readability we will swap it.
     *
     *
     *         [    byte0 |    byte1 ]
     * VAR:    [ XXXXXXXX | XXXXXXXX ]
     *
     * IF:
     * value : [ -------- | 00000111 ]
     *
     * THEN:
     * block : [ XXXXXXXX | -------- ]
     *
     * ELSE:
     * format: [ -------- | ------XX ]
     * offset: [ -------X | XXXXXX-- ]
     *
     * This does not skip dataManager->data.
     *
     * c_u32 dataOffset = (byte1 << 7U) + ((byte2 & 0b11111100U) >> 1);
     */
    class MU Grid {
        MU union {
            u16 grid;
            struct {
                u8 byte0;
                u8 byte1;
            } A;
        } U;
    };



    void ChunkV11::readBlockData() const {

        for (int putBlockOffset = 0; putBlockOffset < 65536; putBlockOffset += 32768) {
            c_i32 blockLength = static_cast<i32>(dataManager->read<u32>()) - GRID_HEADER_SIZE;

            if (blockLength < 0) { continue; }

            // access: 0 <-> 1023
            c_u8* gridHeader = dataManager->ptr();
            dataManager->skip(GRID_HEADER_SIZE);

            // access: 0 <-> blockLength
            c_u8 *const blockDataPtr = dataManager->ptr();
            dataManager->skip(blockLength);

            /**
             * the data is stored in the m_order of
             * [ byte2 | byte1 ]
             * but for readability we will swap it.
             *
             *
             *         [    byte1 |    byte2 ]
             * VAR:    [ XXXXXXXX | XXXXXXXX ]
             *
             * IF:
             * value : [ -------- | 00000111 ]
             *
             * THEN:
             * block : [ XXXXXXXX | -------- ]
             *
             * ELSE:
             * format: [ -------- | ------XX ]
             * offset: [ -------X | XXXXXX-- ]
             *
             * This does not skip dataManager->data.
             */
            for (int gridIndex = 0; gridIndex < GRID_HEADER_SIZE; gridIndex += 2) {
                // read the grid header bytes
                c_u8 byte0 = gridHeader[gridIndex];
                c_u8 byte1 = gridHeader[gridIndex + 1];

                u8 grid[GRID_SIZE] = {};

                if (byte0 == 0b00000111) {
                    // this is only here to optimize filling with blocks
                    if (byte1 != 0) {
                        for (u8& gridIter: grid) {
                            gridIter = byte1;
                        }
                    }

                } else {
                    // find the location of the grid's data
                    c_u32 dataOffset = ((byte0 & 0b11111100U) >> 1) + (byte1 << 7U);
                    c_u8* const gridPositionPtr = blockDataPtr + dataOffset;

                    // switch over format
                    switch (byte0 & 0b11U) {
                        case 0: readGrid<1>(gridPositionPtr, grid); break;
                        case 1: readGrid<2>(gridPositionPtr, grid); break;
                        case 2: readGrid<4>(gridPositionPtr, grid); break;
                        case 3: fillAllBlocks<GRID_SIZE>(gridPositionPtr, grid); break;
                        default: return;
                    }
                }
                // place the grid blocks into the chunkData
                putBlocks(chunkData->oldBlocks, grid, putBlockOffset, calcOffset(gridIndex / 2));
            }
        }
    }


    template<size_t BitsPerBlock>
    bool ChunkV11::readGrid(u8 const* buffer, u8 grid[GRID_SIZE]) {
        c_int size = 1 << BitsPerBlock;
        u8_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int gridIndex = 0;
        c_int blocksPerByte = 8 / BitsPerBlock;
        // iterates over all bytes
        for (size_t byteOffset = 0; byteOffset < 8 * BitsPerBlock; byteOffset++) {
            u16 currentByte = buffer[size + byteOffset];
            // iterates over each block in a byte
            for (u32 j = 0; j < blocksPerByte; j++) {
                u16 paletteIndex = 0;
                // iterates over each bit in the byte, could be made faster?
                for (u32 bitPerBlock = 0; bitPerBlock < BitsPerBlock; bitPerBlock++) {
                    paletteIndex |= (currentByte & 1) << bitPerBlock;
                    currentByte >>= 1;
                }
                grid[gridIndex++] = palette[paletteIndex];
            }
        }
        return true;
    }

    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV11::writeChunk() {
        dataManager->write<i32>(chunkData->chunkX);
        dataManager->write<i32>(chunkData->chunkZ);
        dataManager->write<i64>(chunkData->lastUpdate);

        if (chunkData->lastVersion > 8) {
            dataManager->write<i64>(chunkData->inhabitedTime);
        }

        writeBlockData();

        writeDataBlock(dataManager, &chunkData->blockData[0]);
        writeDataBlock(dataManager, &chunkData->blockData[16384]);
        writeDataBlock(dataManager, &chunkData->skyLight[0]);
        writeDataBlock(dataManager, &chunkData->skyLight[16384]);
        writeDataBlock(dataManager, &chunkData->blockLight[0]);
        writeDataBlock(dataManager, &chunkData->blockLight[16384]);

        dataManager->writeBytes(chunkData->heightMap.data(), 256);
        dataManager->write<i16>(chunkData->terrainPopulated);
        dataManager->writeBytes(chunkData->biomes.data(), 256);

        NBTBase nbt = makeCompound({
                {"", makeCompound(
                             {
                                     {"Entities", chunkData->entities },
                                     {"TileEntities", chunkData->tileEntities },
                                     {"TileTicks", chunkData->tileTicks },
                             }
                             )}
        });
        nbt.write(*dataManager);
    }


    MU void ChunkV11::writeBlockData() const {


        u8 blockMap[MAP_SIZE] = {};
        MU u8 gridHeader[1024];


        for (u32 sectionIndex = 0; sectionIndex < 2; sectionIndex++) {

            u16_vec blockVec;
            u16_vec blockLoc;
            for (u32 gridX = 0; gridX < 65536; gridX += 16384) {
            for (u32 gridZ = 0; gridZ < 4096; gridZ += 1024) {
                c_u32 gridYLower = sectionIndex * 8;
                c_u32 gridYUpper = gridYLower + 8;
            for (u32 gridY = gridYLower; gridY < gridYUpper; gridY += 4) {
                blockVec.clear();
                blockLoc.clear();

                // iterate over the blocks in the 4x4x4 subsection of the chunk, called a grid
                MU bool noSubmerged = true;
                c_u32 offsetInBlock = sectionIndex * 16 + gridY + gridZ + gridX;
                for (u32 blockX = 0; blockX < 16384; blockX += 4096) {
                for (u32 blockZ = 0; blockZ < 1024; blockZ += 256) {
                for (u32 blockY = 0; blockY < 4; blockY++) {
                    c_u32 blockIndex = offsetInBlock + blockY + blockZ + blockX;
                    u16 block = chunkData->newBlocks[blockIndex];
                    if (blockMap[block]) {
                        blockLoc.push_back(blockMap[block] - 1);
                    } else {
                        blockMap[block] = blockVec.size() + 1;
                        u16 location = blockVec.size();
                        blockVec.push_back(block);
                        blockLoc.push_back(location);
                    }
                }
                }
                }

                MU u16 gridID;
                MU u16 gridFormat;
                switch (blockVec.size()) {
                    case 1:
                        /* do something */
                        goto SWITCH_END;
                    case 2:
                        gridFormat = V11_1_BIT; writeGrid<1>(blockVec, blockLoc, blockMap);
                        break;
                    case 3: case 4:
                        gridFormat = V11_2_BIT; writeGrid<2>(blockVec, blockLoc, blockMap);
                        break;
                    case 5: case 6: case 7: case 8:
                        gridFormat = V11_3_BIT; writeGrid<3>(blockVec, blockLoc, blockMap);
                        break;
                    case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
                        gridFormat = V11_4_BIT; writeGrid<4>(blockVec, blockLoc, blockMap);
                        break;
                }
                // gridID = sectionSize / 4 | gridFormat << 12U;
            SWITCH_END:;
                // gridHeader[gridIndex++] = gridID;
                // sectionSize += V11_GRID_SIZES[gridFormat];
            }
            }
            }
        }





    }


    template<size_t BitsPerBlock>
    void ChunkV11::writeGrid(MU u16_vec& blockVector, MU u16_vec& blockLocations, MU u8 blockMap[MAP_SIZE]) const {
        /*
        c_int size = 1 << BitsPerBlock;
        u8_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int gridIndex = 0;
        c_int blocksPerByte = 8 / BitsPerBlock;
        // iterates over all bytes
        for (size_t byteOffset = 0; byteOffset < 8 * BitsPerBlock; byteOffset++) {
            u16 currentByte = buffer[size + byteOffset];
            // iterates over each block in a byte
            for (u32 j = 0; j < blocksPerByte; j++) {
                u16 paletteIndex = 0;
                // iterates over each bit in the byte, could be made faster?
                for (u32 bitPerBlock = 0; bitPerBlock < BitsPerBlock; bitPerBlock++) {
                    paletteIndex |= (currentByte & 1) << bitPerBlock;
                    currentByte >>= 1;
                }
                grid[gridIndex++] = palette[paletteIndex];
            }
        }
        return true;
         */
    }


    void ChunkV11::writeWithMaxBlocks(MU const u16_vec& blockVector, MU const u16_vec& blockLocations, MU u8 blockMap[MAP_SIZE]) const {

    }


} // namespace editor::chunk
