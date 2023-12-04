#include "v11Chunk.hpp"
#include <algorithm>


static inline u32 toIndex(u32 num) {
    return (num + 1) * 128;
}


namespace universal {


    void V11Chunk::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
        dataManager = managerIn;
        chunkData = chunkDataIn;

        chunkData->dimension = dim;
        chunkData->chunkX = (i32) dataManager->readInt32();
        chunkData->chunkZ = (i32) dataManager->readInt32();
        chunkData->lastUpdate = (i64) dataManager->readInt64();
        if (chunkData->lastVersion > 8)
            chunkData->inhabitedTime = (i64) dataManager->readInt64();

        chunkData->newBlocks = u16_vec(65536);
        readBlocks();

        chunkData->DataGroupCount = 0;
        chunkData->blockData = u8_vec(32768);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        readData();

        chunkData->heightMap = dataManager->readIntoVector(256);
        chunkData->terrainPopulated = (i16) dataManager->readInt16();
        chunkData->biomes = dataManager->readIntoVector(256);

        if (*dataManager->ptr == 0x0A) {
            chunkData->NBTData = NBT::readTag(*dataManager);
        }
    }


    static inline int calculateOffset(int value) {
        int num = value / 32;
        int num2 = value % 32;
        return num / 4 * 64 + num % 4 * 4 + num2 * 1024;
    }


    static inline void maxBlocks(u8 const* buffer, u8* grid) {
        /// TODO: can probably use memset
        std::copy_n(buffer, 128, grid);
    }


    static inline void putBlocks(u16_vec& writeVec, const u8* grid, int writeOffset, int readOffset) {
        readOffset = calculateOffset(readOffset);
        int num = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    int currentOffset = readOffset + i * 16 + j + k * 256; // xzy or zxy?
                    u8 v1 = grid[num];
                    num += 2;
                    writeVec[currentOffset + writeOffset] = static_cast<u16>(v1); // | (static_cast<u16>(v2) << 8);
                }
            }
        }
    }


    void V11Chunk::readBlocks() const {
        const u32 CHUNK_HEADER_SIZE = chunkData->lastVersion > 8 ? 26 : 18;

        int putBlockOffset = 0;
        for (size_t loop = 0; loop < 2; ++loop, putBlockOffset += 32768) {
            i32 blockLength = (i32)dataManager->readInt32() - GRID_HEADER_SIZE;

            if (blockLength <= 0) {
                continue;
            }

            u8_vec header = dataManager->readIntoVector(GRID_HEADER_SIZE);
            u8_vec data = dataManager->readIntoVector(blockLength);
            for (int gridIndex = 0; gridIndex < 512; gridIndex++) {
                u8 byte1 = header[gridIndex * 2];
                u8 byte2 = header[gridIndex * 2 + 1];
                u8 grid[128] = {0};

                if (byte1 == 7) {
                    if (byte2 != 0) {

                        for (int i = 0; i < 128; i += 2) {
                            grid[i] = byte2;
                        }

                    }
                } else {
                    int dataOffset = (byte2 << 7) + ((byte1 & 0b11111100) >> 1);
                    u32 gridPosition = 4 + GRID_HEADER_SIZE + CHUNK_HEADER_SIZE + dataOffset;
                    u8 format = byte1 & 0b11;
                    switch (format) {
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
    bool V11Chunk::readGrid(u8 const* buffer, u8 grid[128]) const {
        int size = (1 << BitsPerBlock);
        u8_vec palette(size);
        std::copy_n(buffer, size, palette.begin());

        int gridIndex = 0;
        int blocksPerByte = 8 / BitsPerBlock;
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


    void V11Chunk::readData() const {

        u8_vec_vec dataArray(6);
        for (int i = 0; i < 6; i++) {
            u32 num = dataManager->readInt32();
            u32 index = toIndex(num);
            // TODO: try to remove not-needed copy
            dataArray[i] = dataManager->readIntoVector(index);
            chunkData->DataGroupCount += dataArray[i].size();
        }

        auto processLightData = [](const u8_vec& data, u8_vec& blockData, int& offset) {
            for (int k = 0; k < 128; k++) {
                if (data[k] == 128) {
                    memset(&blockData[offset], 0, 128);
                } else if (data[k] == 129) {
                    memset(&blockData[offset], 255, 128);
                } else {
                    memcpy(&blockData[offset], &data[toIndex(data[k])], 128);
                }
                offset += 128;
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

}


