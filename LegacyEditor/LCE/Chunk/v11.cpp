#include "v11.hpp"

#include <cstring>
#include <algorithm>

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/dataManager.hpp"
#include "helpers.hpp"


static u32 toIndex(const u32 num) {
    return (num + 1) * 128;
}

// TODO: I think I need to rewrite this all to place blocks as only u8's,
// TODO: and to switch it to use oldBlocks instead of newBlocks

namespace editor::chunk {

    void ChunkV11::allocChunk() const {
        chunkData->oldBlocks = u8_vec(65536);
        chunkData->blockData = u8_vec(32768);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
    }


    // #####################################################
    // #               Read Section
    // #####################################################


    void ChunkV11::readChunk() {
        allocChunk();

        chunkData->chunkX = static_cast<i32>(dataManager->readInt32());
        chunkData->chunkZ = static_cast<i32>(dataManager->readInt32());
        chunkData->lastUpdate = static_cast<i64>(dataManager->readInt64());

        chunkData->DataGroupCount = 0;
        if (chunkData->lastVersion > 8) {
            chunkData->inhabitedTime = static_cast<i64>(dataManager->readInt64());
        }

        readBlocks();

        auto dataArray = readGetDataBlockVector<6>(chunkData, dataManager);
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


    static int calculateOffset(const int value) {
        const int num = value / 32;
        const int num2 = value % 32;
        return num / 4 * 64 + num % 4 * 4 + num2 * 1024;
    }



    static void putBlocks(u16_vec& writeVec, const u8* grid, const int writeOffset, int readOffset) {
        readOffset = calculateOffset(readOffset);
        int gridIndex = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    const int currentOffset = readOffset + i * 16 + j + k * 256; // xzy or zxy?
                    const u8 num2 = grid[gridIndex++];
                    writeVec[currentOffset + writeOffset] = static_cast<u16>(num2);
                }
            }
        }
    }


    void ChunkV11::readBlocks() const {
        const u32 CHUNK_HEADER_SIZE = chunkData->lastVersion > 8 ? 26 : 18;

        int putBlockOffset = 0;
        for (size_t loop = 0; loop < 2; ++loop, putBlockOffset += 32768) {
            const i32 blockLength = static_cast<i32>(dataManager->readInt32()) - GRID_HEADER_SIZE;

            if (blockLength <= 0) {
                continue;
            }

            u8* header = dataManager->ptr;
            dataManager->incrementPointer(GRID_HEADER_SIZE);

            // TODO: why is this read but not used???
            u8* data = dataManager->ptr;
            dataManager->incrementPointer(blockLength);


            for (int gridIndex = 0; gridIndex < 512; gridIndex++) {
                const u8 byte1 = header[gridIndex * 2];
                const u8 byte2 = header[gridIndex * 2 + 1];
                u8 grid[GRID_SIZE] = {0};

                if (byte1 == 0b111) {
                    if (byte2 != 0) {

                        for (int i = 0; i < GRID_SIZE; i++) {
                            grid[i] = byte2;
                        }

                    }
                } else {
                    const int dataOffset = (byte2 << 7) + ((byte1 & 0b11111100) >> 1);
                    const u32 gridPosition = 4 + GRID_HEADER_SIZE + CHUNK_HEADER_SIZE + dataOffset;
                    switch (byte1 & 0b11) {
                        case 0: readGrid<1>(dataManager->data + gridPosition, grid); break;
                        case 1: readGrid<2>(dataManager->data + gridPosition, grid); break;
                        case 2: readGrid<4>(dataManager->data + gridPosition, grid); break;
                        case 3:
                            fillAllBlocks<GRID_SIZE>(dataManager->data + gridPosition, grid); break;
                        default: return;
                    }
                }

                putBlocks(chunkData->newBlocks, grid, putBlockOffset, gridIndex);
            }

        }
    }


    template<size_t BitsPerBlock>
    bool ChunkV11::readGrid(u8 const* buffer, u8 grid[GRID_SIZE]) {
        const int size = 1 << BitsPerBlock;
        u8_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int gridIndex = 0;
        const int blocksPerByte = 8 / BitsPerBlock;
        for (size_t i = 0; i < BitsPerBlock * 8; i++) {
            u16 v = buffer[size + i];
            for (int j = 0; j < blocksPerByte; j++) {
                u16 idx = 0;
                for (int x = 0; x < BitsPerBlock; x++) {
                    idx |= (v & 1) << x;
                    v >>= 1;
                }
                if EXPECT_FALSE (idx >= size) { return false; }
                grid[gridIndex] = palette[idx];
                gridIndex += 1;
            }
        }
        return true;
    }

    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV11::writeChunk() {

    }




}


