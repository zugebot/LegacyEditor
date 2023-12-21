#include "v11.hpp"

#include <cstring>
#include <algorithm>

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


static u32 toIndex(const u32 num) {
    return (num + 1) * 128;
}

// TODO: I think I need to rewrite this all to place blocks as only u8's,
// TODO: and to switch it to use oldBlocks instead of newBlocks

namespace editor::chunk {

    void ChunkV11::allocChunk() const {
        chunkData->newBlocks = u16_vec(65536);
        chunkData->blockData = u8_vec(32768);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
    }

    // #####################################################
    // #               Read Section
    // #####################################################

    void ChunkV11::readChunk(ChunkData* chunkDataIn, DataManager* managerIn) {
        dataManager = managerIn;
        chunkData = chunkDataIn;
        allocChunk();

        chunkData->chunkX = static_cast<i32>(dataManager->readInt32());
        chunkData->chunkZ = static_cast<i32>(dataManager->readInt32());
        chunkData->lastUpdate = static_cast<i64>(dataManager->readInt64());

        chunkData->DataGroupCount = 0;
        if (chunkData->lastVersion > 8) { // Potions
            chunkData->inhabitedTime = static_cast<i64>(dataManager->readInt64());
        }

        readBlocks();
        readData();

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


    static void maxBlocks(u8 const* buffer, u8* grid) {
        /// TODO: can probably use memset
        std::copy_n(buffer, 128, grid);
    }


    static void putBlocks(u16_vec& writeVec, const u8* grid, const int writeOffset, int readOffset) {
        readOffset = calculateOffset(readOffset);
        int num1 = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    const int currentOffset = readOffset + i * 16 + j + k * 256; // xzy or zxy?
                    const u8 num2 = grid[num1];
                    num1 += 2;
                    writeVec[currentOffset + writeOffset] = static_cast<u16>(num2); // | (static_cast<u16>(v2) << 8);
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

            u8_vec header = dataManager->readIntoVector(GRID_HEADER_SIZE);
            u8_vec data = dataManager->readIntoVector(blockLength);
            for (int gridIndex = 0; gridIndex < 512; gridIndex++) {
                const u8 byte1 = header[gridIndex * 2];
                const u8 byte2 = header[gridIndex * 2 + 1];
                u8 grid[GRID_SIZE] = {0};

                if (byte1 == 7) {
                    if (byte2 != 0) {

                        for (int i = 0; i < GRID_SIZE; i += 2) {
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
                        case 3: maxBlocks  (dataManager->data + gridPosition, grid); break;
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
                gridIndex += 2;
            }
        }
        return true;
    }


    void ChunkV11::readData() const {

        u8_vec_vec dataArray(6);
        for (int i = 0; i < 6; i++) {
            const u32 num = dataManager->readInt32();
            const u32 index = toIndex(num);
            // TODO: try to remove not-needed copy
            dataArray[i] = dataManager->readIntoVector(index);
            chunkData->DataGroupCount += dataArray[i].size();
        }

        auto processLightData = [](const u8_vec& data, u8_vec& blockData, int& offset) {
            for (int k = 0; k < DATA_SECTION_SIZE; k++) {
                if (data[k] == DATA_SECTION_SIZE) {
                    memset(&blockData[offset], 0, DATA_SECTION_SIZE);
                } else if (data[k] == DATA_SECTION_SIZE + 1) {
                    memset(&blockData[offset], 255, DATA_SECTION_SIZE);
                } else {
                    memcpy(&blockData[offset], &data[toIndex(data[k])], DATA_SECTION_SIZE);
                }
                offset += DATA_SECTION_SIZE;
            }
        };

        int writeOffset = 0;
        processLightData(dataArray[0], chunkData->blockData, writeOffset);
        processLightData(dataArray[1], chunkData->blockData, writeOffset);
        writeOffset = 0;
        processLightData(dataArray[2], chunkData->skyLight, writeOffset);
        processLightData(dataArray[3], chunkData->skyLight, writeOffset);
        writeOffset = 0;
        processLightData(dataArray[4], chunkData->blockLight, writeOffset);
        processLightData(dataArray[5], chunkData->blockLight, writeOffset);
    }


    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV11::writeChunk(ChunkData* chunkDataIn, DataManager* managerOut) {
        dataManager = managerOut;
        chunkData = chunkDataIn;
    }




}


