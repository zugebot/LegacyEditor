#include "v11.hpp"

#include <cstring>

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/code/Chunk/chunkData.hpp"
#include "LegacyEditor/code/Chunk/helpers.hpp"


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

        chunkData->chunkX = static_cast<i32>(dataManager->readInt32());
        chunkData->chunkZ = static_cast<i32>(dataManager->readInt32());
        chunkData->lastUpdate = static_cast<i64>(dataManager->readInt64());

        chunkData->DataGroupCount = 0;
        if (chunkData->lastVersion > 8) {
            chunkData->inhabitedTime = static_cast<i64>(dataManager->readInt64());
        }

        readBlocks();

        c_auto dataArray = readGetDataBlockVector<6>(chunkData, dataManager);
        readDataBlock(dataArray[0], dataArray[1], chunkData->blockData);
        readDataBlock(dataArray[2], dataArray[3], chunkData->skyLight);
        readDataBlock(dataArray[4], dataArray[5], chunkData->blockLight);

        dataManager->readOntoData(256, chunkData->heightMap.data());
        chunkData->terrainPopulated = static_cast<i16>(dataManager->readInt16());
        dataManager->readOntoData(256, chunkData->biomes.data());

        if (*dataManager->ptr == 0x0A) {
            chunkData->NBTData = NBT::readTag(*dataManager);
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
    static void putBlocks(u8_vec& writeVec, c_u8* grid, c_int writeOffset, c_int readOffset) {
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


    void ChunkV11::readBlocks() const {

        for (int putBlockOffset = 0; putBlockOffset < 65536; putBlockOffset += 32768) {
            c_i32 blockLength = static_cast<i32>(dataManager->readInt32()) - GRID_HEADER_SIZE;

            if (blockLength < 0) { continue; }

            // access: 0 <-> 1023
            c_u8* gridHeader = dataManager->ptr;
            dataManager->incrementPointer(GRID_HEADER_SIZE);

            // access: 0 <-> blockLength
            c_u8 *const blockDataPtr = dataManager->ptr;
            dataManager->incrementPointer(blockLength);

            /**
             * the data is stored in the order of
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
             * This does not increment dataManager->data.
             */
            for (int gridIndex = 0; gridIndex < GRID_HEADER_SIZE; gridIndex += 2) {
                // read the grid header bytes
                c_u8 byte2 = gridHeader[gridIndex];
                c_u8 byte1 = gridHeader[gridIndex + 1];
                u8 grid[GRID_SIZE] = {};

                if (byte2 == 0b111) {
                    // this is only here to optimize filling with air
                    if (byte1 != 0) {
                        for (u8& gridIter: grid) {
                            gridIter = byte1;
                        }
                    }

                } else {
                    // find the location of the grid's data
                    c_u32 dataOffset = (byte1 << 7U) + ((byte2 & 0b11111100U) >> 1);
                    c_u8* const gridPositionPtr = blockDataPtr + dataOffset;

                    // switch over format
                    switch (byte2 & 0b11U) {
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
    }


} // namespace editor::chunk
