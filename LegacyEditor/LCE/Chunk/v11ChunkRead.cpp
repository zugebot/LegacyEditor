#include "v11Chunk.hpp"


namespace universal {


    void V11Chunk::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
        dataManager = managerIn;
        chunkData = chunkDataIn;

        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        chunkData->blocks = u16_vec(65536);

        chunkData->chunkX = (i32) dataManager->readInt32();
        chunkData->chunkZ = (i32) dataManager->readInt32();

        chunkData->lastUpdate = (i64) dataManager->readInt64();

        if (chunkData->lastVersion > 8) {
            chunkData->inhabitedTime = (i64) dataManager->readInt64();
        } else {
            chunkData->inhabitedTime = 0;
        }

        readBlocks();
        readData();
        chunkData->heightMap = dataManager->readIntoVector(256);
        chunkData->terrainPopulated = (i16) dataManager->readInt16();
        chunkData->biomes = dataManager->readIntoVector(256);
        readNBTData();
    }


    static inline int calculateOffset(int value) {
        int num = value / 32;
        int num2 = value % 32;
        return num / 4 * 64 + num % 4 * 4 + num2 * 1024;
    }

    static inline void maxBlocks(uint8_t const* buffer, uint8_t* grid) {
        std::copy_n(buffer, 64, grid);
    }

    // TODO: integrate this into making it write blocks as u16's for compatability
    static inline void putBlocks(
                    std::vector<uint8_t>& writeVector, const uint8_t* readArray,
                    int writeOffset, int readOffset) {
        int num = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    int num2 = readOffset + i * 16 + j + k * 256;
                    writeVector[num2 + writeOffset] = readArray[num++];
                }
            }
        }
    }


    void V11Chunk::readBlocks() {
        for (int loop = 0; loop < 2; ++loop) {
            uint32_t blockLength = dataManager->readInt32();
            blockLength -= 1024;
            std::vector<uint8_t> header = dataManager->readIntoVector(1024);
            if (blockLength > 0) {
                std::vector<uint8_t> data = dataManager->readIntoVector((int) blockLength);
                for (int gridIndex = 0; gridIndex < 1024; gridIndex += 2) {
                    uint8_t byte1 = header[gridIndex];
                    uint8_t byte2 = header[gridIndex + 1];
                    int fillOffset = (loop == 0) ? 0 : 32768;
                    int putBlockOffset = fillOffset;

                    if (byte1 == 7) {
                        if (byte2 != 0) {

                            uint8_t grid[64];
                            int offset = calculateOffset(gridIndex >> 1);
                            for (unsigned char& i: grid) { i = byte2; }
                            putBlocks(chunkData->blocks, grid, fillOffset, offset);
                        }
                    } else {
                        int dataOffset = (byte1 & 252) / 2 + byte2 * 128;
                        int gridPosition = 1054 + dataOffset; // 1024 for header, and 30 for start
                        uint8_t format = byte1 & 3;
                        uint8_t grid[64];

                        switch (format) {
                            case 0:
                                readGrid<1>(dataManager->data + gridPosition, grid);
                                break;
                            case 1:
                                readGrid<2>(dataManager->data + gridPosition, grid);
                                break;
                            case 2:
                                readGrid<4>(dataManager->data + gridPosition, grid);
                                break;
                            case 3:
                                maxBlocks(dataManager->data + gridPosition, grid);
                                break;
                            default:
                                return;
                        }



                        putBlocks(chunkData->blocks, grid, putBlockOffset, calculateOffset(gridIndex >> 1));
                    }
                }
            }
        }
    }


    template<size_t BitsPerBlock>
    bool V11Chunk::readGrid(u8 const* buffer, u8* grid) const {
        int size = (1 << BitsPerBlock);
        std::vector<u8> palette(size);
        std::copy_n(buffer, size, palette.begin());

        int gridIndex = 0;
        int blocksPerByte = 8 / BitsPerBlock;
        for (size_t i = 0; i < BitsPerBlock * 8; i++) {
            u16 v = buffer[size + i];
            for (int j = 0; j < blocksPerByte; j++) {
                u16 idx = 0;
                for (int x = 0; x < BitsPerBlock; x++) {
                    idx |= ((v & 1) > 0 ? 1 : 0) << x;
                    v >>= 1;
                }
                if EXPECT_FALSE (idx >= size) { return false; }
                grid[gridIndex] = palette[idx];
                gridIndex++;
            }
        }
        return true;
    }


    static inline u32 toIndex(u32 num) {
        return (num + 1) * 128;
    }


    void V11Chunk::readData() const {

        chunkData->DataGroupCount = 0;
        u8_vec_vec dataArray(6);
        for (int i = 0; i < 4; i++) {
            u32 num = (u32) dataManager->readInt32();
            u32 index = toIndex(num);
            // TODO: this is really slow, figure out how to remove it
            dataArray[i] = dataManager->readIntoVector(index);
            chunkData->DataGroupCount += (i32)dataArray[i].size();
        }

        auto processLightData = [](const u8_vec& data, u8_vec& blockData, int& offset) {
            for (int k = 0; k < 128; k++) {
                if (data[k] == 128) {
                    memset(&blockData[offset], 0, 128);
                } else if (data[k] == 129) {
                    memset(&blockData[offset], 255, 128);
                } else {
                    std::memcpy(&blockData[offset], &data[toIndex(data[k])], 128);
                }
                offset += 128;
            }
        };

        // Process light data
        int writeOffset = 0;
        processLightData(dataArray[0], chunkData->blockData, writeOffset);
        processLightData(dataArray[1], chunkData->blockData, writeOffset);
        writeOffset = 0; // Reset offset for block light
        processLightData(dataArray[2], chunkData->skyLight, writeOffset);
        processLightData(dataArray[3], chunkData->skyLight, writeOffset);
        writeOffset = 0; // Reset offset for block light
        processLightData(dataArray[4], chunkData->blockLight, writeOffset);
        processLightData(dataArray[5], chunkData->blockLight, writeOffset);
    }


    void V11Chunk::readNBTData() const {
        if (*dataManager->ptr == 0xA) {
            chunkData->NBTData = NBT::readTag(*dataManager);
        }
    }




}


