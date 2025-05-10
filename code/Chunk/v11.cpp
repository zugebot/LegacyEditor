#include "v11.hpp"

#include <cstring>

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/helpers.hpp"
#include "common/nbt.hpp"


// TODO: I think I need to rewrite this all to place blocks as only u8's,
// TODO: and to switch it to use oldBlocks instead of newBlocks

namespace editor::chunk {


    void Grid::print() const {
        std::cout << "Data Offset: " << getDataOffset() << std::endl;
    }


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

        readBlocks(&chunkData->oldBlocks[0]);
        readBlocks(&chunkData->oldBlocks[32768]);

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




    // TODO: this function is complete ass
    static void putBlocks(u8* writePtr, c_u8* grid, c_int gridIndex) {
        // grid order: XZY
        const int num_ = gridIndex / 32;  // y is stored last, so shifting it over (for X and Z)
        const int num2_ = gridIndex % 32; // y
        const int readOffset = num_ / 4 * 64 + num_ % 4 * 4 + num2_ * 1024;

        int num = 0;
        for (int z = 0; z < 4; z++) {
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    const int num2 = readOffset + x + z * 16 + y * 256;
                    writePtr[num2] = grid[num++];
                }
            }
        }
    }


    void ChunkV11::readBlocks(u8* oldBlockPtr) const {
        c_i32 sectionSize = dataManager->read<i32>();

        c_i32 blockLength = sectionSize - GRID_COUNT * 2;
        if (blockLength < 0) { return; }

        c_u8* gridHeader = dataManager->fetch<GRID_COUNT * 2>();
        c_u8 *const blockDataPtr = dataManager->fetch(blockLength);

        // 64 grids of 4x4x4 blocks, stored in XZY order
        for (int gridIndex = 0; gridIndex < GRID_COUNT; gridIndex++) {

            Grid grid(
                gridHeader[gridIndex * 2],
                gridHeader[gridIndex * 2 + 1]
            );
            u8 blockBuffer[GRID_SIZE] = {};

            if (grid.single.flag == Grid::IS_SINGLE_BLOCK_FLAG) {
                // this is only here to optimize filling with blocks
                if (grid.single.block != 0)
                    for (u8& gridIter: blockBuffer)
                        gridIter = grid.single.block;
            } else {
                // find the location of the grid's data
                c_u8* const gridDataPtr = blockDataPtr + grid.getDataOffset();
                // switch over format
                switch (grid.multiple.grid_format) {
                    case 0: readGrid<1>(gridDataPtr, blockBuffer); break;
                    case 1: readGrid<2>(gridDataPtr, blockBuffer); break;
                    case 2: readGrid<4>(gridDataPtr, blockBuffer); break;
                    case 3: fillAllBlocks<GRID_SIZE>(gridDataPtr, blockBuffer); break;
                    default: return;
                }
            }
            // place the grid blocks into the chunkData
            putBlocks(oldBlockPtr, blockBuffer, gridIndex);
        }
    }



    template<size_t BitsPerBlock>
    bool ChunkV11::readGrid(u8 const* gridDataPtr, u8 blockBuffer[GRID_SIZE]) {
        constexpr int size = 1 << BitsPerBlock;
        constexpr int blocks_per_byte = 8 / BitsPerBlock;

        u8_vec palette(size);
        std::copy_n(gridDataPtr, size, palette.begin());

        int gridIndex = 0;
        // iterates over all bytes
        for (size_t byteOffset = 0; byteOffset < 8 * BitsPerBlock; byteOffset++) {
            u16 currentByte = gridDataPtr[size + byteOffset];
            // iterates over each block in a byte
            for (u32 j = 0; j < blocks_per_byte; j++) {
                u16 paletteIndex = 0;
                // iterates over each bit in the byte, could be made faster?
                for (u32 bitPerBlock = 0; bitPerBlock < BitsPerBlock; bitPerBlock++) {
                    paletteIndex |= (currentByte & 1) << bitPerBlock;
                    currentByte >>= 1;
                }
                blockBuffer[gridIndex++] = palette[paletteIndex];
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

        writeBlocks(&chunkData->oldBlocks[0]);
        writeBlocks(&chunkData->oldBlocks[32768]);

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


    MU void ChunkV11::writeBlocks(u8 const* oldBlockPtr) const {

        u8 blockMap[MAP_SIZE] = {};
        MU u8 gridHeader[1024];
        int gridIndex = 0;


        u16_vec blockVec;
        u16_vec blockLoc;
        for (u32 gridY = 0; gridY < 32; gridY++) {
        for (u32 gridZ = 0; gridZ < 4; gridZ++) {
        for (u32 gridX = 0; gridX < 4; gridX++) {
            c_u32 blockOffset = gridX + gridY + gridZ;
            c_u32 gridOffset = gridX + gridY + gridZ;

            blockVec.clear();
            blockLoc.clear();

            for (u32 blockY = 0; blockY < 4; blockY++) {
            for (u32 blockZ = 0; blockZ < 4; blockZ++) {
            for (u32 blockX = 0; blockX < 4; blockX++) {
                c_u32 blockIndex = blockOffset + blockY + blockZ + blockX;

                c_u8 block = oldBlockPtr[blockIndex];
                if (blockMap[block]) {
                    blockLoc.push_back(blockMap[block] - 1);
                } else {
                    blockMap[block] = blockVec.size() + 1;
                    u16 location = blockVec.size();
                    blockVec.push_back(block);
                    blockLoc.push_back(location);
                }
            }}}

            /*
            * case  1:   // 65535 (-1 unsigned)
            * case  2:   // 0
            * case  3-4: // 1
            * case  5-8: // 2
            * case 9-16: // 3
            * this code assumes blockVec is never 0
             */
            const size_t n = blockVec.size();
            MU const auto gridFormat = static_cast<V11_GRID_STATE>(n == 1 ? 0
                : 32 - __builtin_clz(static_cast<unsigned>(n - 1)) - 1);

            MU u16 gridID;
            switch (gridFormat) {
                case V11_0_BIT:
                    u16 value = blockVec[0]
                    gridHeader[gridIndex++] =
                    goto SWITCH_END;
                case V11_1_BIT:
                    writeGrid<1>(blockVec, blockLoc, blockMap);
                    break;
                case V11_2_BIT:
                    writeGrid<2>(blockVec, blockLoc, blockMap);
                    break;
                case V11_3_BIT:
                    writeGrid<3>(blockVec, blockLoc, blockMap);
                    break;
                case V11_4_BIT:
                    writeGrid<4>(blockVec, blockLoc, blockMap);
                    break;
            }
            // gridID = sectionSize / 4 | (gridFormat - 1) << 12U;
        SWITCH_END:;
            // gridHeader[gridIndex++] = gridID;
            // sectionSize += V11_GRID_SIZES[gridFormat - 1];


        }}}






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
