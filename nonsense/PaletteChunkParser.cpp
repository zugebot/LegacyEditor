#include "PaletteChunkParser.hpp"
#include "LCE-universal.hpp"


PaletteChunkParser::~PaletteChunkParser() {
    if (LCE_ChunkData.NBTData) {
        LCE_ChunkData.NBTData->NbtFree();
        delete LCE_ChunkData.NBTData;
    }
}


UniversalChunkFormat* PaletteChunkParser::ParseChunk(DataManager& inputData, DIM dimension, LCEFixes& fixes,
                                                     int version) {
    this->LCE_ChunkData.version = version;
    this->LCE_ChunkData.chunkX = (int) inputData.readInt32();
    this->LCE_ChunkData.chunkZ = (int) inputData.readInt32();
    this->LCE_ChunkData.lastUpdate = (int64_t) inputData.readInt64();
    if (version > 8) {
        this->LCE_ChunkData.inhabitedTime = (int64_t) inputData.readInt64();
    } else {
        this->LCE_ChunkData.inhabitedTime = 0;
    }
    this->parseBlocks(inputData);
    this->readData(inputData);
    this->LCE_ChunkData.heightMap = PaletteChunkParser::read256(inputData);
    this->LCE_ChunkData.terrainPopulated = (short) inputData.readInt16();
    this->LCE_ChunkData.biomes = PaletteChunkParser::read256(inputData);
    this->readNBTData(inputData);
    return LCE_universal::convertLCE1_12RegionToUniversal(this->LCE_ChunkData, dimension, fixes);
}


UniversalChunkFormat* PaletteChunkParser::ParseChunkForAccess(DataManager& inputData, DIM dimension,
                                                              LCEFixes& fixes) {
    inputData.seek(LCE_ChunkData.version == 8 ? 18 : 26);
    this->parseBlocks(inputData);
    this->readDataOnlyForAccess(inputData);
    return LCE_universal::convertLCE1_12RegionToUniversalForAccess(this->LCE_ChunkData, dimension, fixes);
}


void PaletteChunkParser::readNBTData(DataManager& inputData) {
    if (*inputData.data == 0xA) { this->LCE_ChunkData.NBTData = NBT::readTag(inputData); }
}


void PaletteChunkParser::readData(DataManager& inputData) {
    std::vector<std::vector<uint8_t>> dataArray;
    for (int i = 0; i < 6; i++) {
        std::vector<uint8_t> item = PaletteChunkParser::read128(inputData);
        dataArray.push_back(item);
    }
    this->LCE_ChunkData.DataGroupCount =
            (int) (dataArray[0].size() + dataArray[1].size() + dataArray[2].size() + dataArray[3].size());

    int segments[6] = {0, 0, 1, 1, 2, 2};
    int offsets[6] = {0, 0x4000, 0, 0x4000, 0, 0x4000};
    std::vector<std::vector<uint8_t>> nibbleData = {std::vector<uint8_t>(0x8000), std::vector<uint8_t>(0x8000),
                                                    std::vector<uint8_t>(0x8000)};

    for (int j = 0; j < 6; j++) {
        int startingIndex = offsets[j];
        int currentLightSegment = segments[j];
        std::vector<uint8_t> data = dataArray[j];
        for (int k = 0; k < 0x80; k++) {
            uint8_t headerValue = data[k];
            if (headerValue == 0x80 || headerValue == 0x81) {
                PaletteChunkParser::copyByte128(nibbleData[currentLightSegment], k * 0x80 + startingIndex,
                                                (headerValue == (uint8_t) 0x80) ? (uint8_t) 0 : (uint8_t) 255);
            } else {
                PaletteChunkParser::copyArray128(data, (int) ((headerValue + 1) * 0x80),
                                                 nibbleData[currentLightSegment], k * 0x80 + startingIndex);
            }
        }
    }
    this->LCE_ChunkData.data = nibbleData[0];
    this->LCE_ChunkData.skyLight = nibbleData[1];
    this->LCE_ChunkData.blockLight = nibbleData[2];
}


void PaletteChunkParser::readDataOnlyForAccess(DataManager& inputData) {
    std::vector<uint8_t> dataTo128 = PaletteChunkParser::read128(inputData);
    std::vector<uint8_t> dataFrom128 = PaletteChunkParser::read128(inputData);

    std::vector<uint8_t> blockData(0x8000);
    for (int k = 0; k < 0x80; k++) {
        uint8_t headerValue = dataTo128[k];
        if (headerValue == 0x80 || headerValue == 0x81) {
            PaletteChunkParser::copyByte128(blockData, k * 0x80,
                                            (headerValue == (uint8_t) 0x80) ? (uint8_t) 0 : (uint8_t) 255);
        } else {
            PaletteChunkParser::copyArray128(dataTo128, (int) ((headerValue + 1) * 0x80), blockData, k * 0x80);
        }
    }
    for (int k = 0; k < 0x80; k++) {
        uint8_t headerValue = dataFrom128[k];
        if (headerValue == 0x80 || headerValue == 0x81) {
            PaletteChunkParser::copyByte128(blockData, k * 0x80 + 0x4000,
                                            (headerValue == (uint8_t) 0x80) ? (uint8_t) 0 : (uint8_t) 255);
        } else {
            PaletteChunkParser::copyArray128(dataFrom128, (int) ((headerValue + 1) * 0x80), blockData,
                                             k * 0x80 + 0x4000);
        }
    }
    this->LCE_ChunkData.data = blockData;
}

std::vector<uint8_t> PaletteChunkParser::read256(DataManager& inputData) {
    std::vector<uint8_t> array1 = inputData.readIntoVector(256);
    return array1;
}

std::vector<uint8_t> PaletteChunkParser::read128(DataManager& inputData) {
    int num = (int) inputData.readInt32();
    std::vector<uint8_t> array1;
    array1 = inputData.readIntoVector((num + 1) * 0x80);
    return array1;
}

void PaletteChunkParser::copyByte128(std::vector<uint8_t>& writeVector, int writeOffset, uint8_t value) {
    for (int i = 0; i < 0x80; i++) { writeVector[writeOffset + i] = value; }
}

void PaletteChunkParser::copyArray128(std::vector<uint8_t>& readVector, int readOffset,
                                      std::vector<uint8_t>& writeVector, int writeOffset) {
    for (int i = 0; i < 0x80; i++) { writeVector[writeOffset + i] = readVector[readOffset + i]; }
}

void PaletteChunkParser::parseBlocks(DataManager& inputData) {
    this->LCE_ChunkData.blocks = std::vector<uint8_t>(0x10000);
    for (int loop = 0; loop < 2; ++loop) {
        uint32_t blockLength = inputData.readInt32();
        blockLength -= 1024;
        std::vector<uint8_t> header = inputData.readIntoVector(1024);
        if (blockLength > 0) {
            std::vector<uint8_t> data = inputData.readIntoVector((int) blockLength);
            for (int i = 0; i < 1024; i += 2) {
                uint8_t byte1 = header[i];
                uint8_t byte2 = header[i + 1];
                int fillOffset = (loop == 0) ? 0 : 32768;
                int putBlockOffset = fillOffset;

                if (byte1 == 7) {
                    if (byte2 != 0) {
                        PaletteChunkParser::fillWithByte(this->LCE_ChunkData.blocks, fillOffset, i >> 1, byte2);
                    }
                } else {
                    int dataOffset = (byte1 & 252) / 2 + byte2 * 128;
                    int gridPosition = 1054 + dataOffset; // 1024 for header, and 30 for start
                    uint8_t format = byte1 & 3;
                    uint8_t grid[64];

                    switch (format) {
                        case 0:
                            parse<1>(inputData.data + gridPosition, grid);
                            break;
                        case 1:
                            parse<2>(inputData.data + gridPosition, grid);
                            break;
                        case 2:
                            parse<4>(inputData.data + gridPosition, grid);
                            break;
                        case 3:
                            maxBlocks(inputData.data + gridPosition, grid);
                            break;
                        default:
                            return;
                    }

                    PaletteChunkParser::putBlocks(this->LCE_ChunkData.blocks, grid, putBlockOffset, calculateOffset(i >> 1));
                }
            }
        }
    }
}

void PaletteChunkParser::fillWithByte(std::vector<uint8_t>& writeVector, int writeOffset, int counter,
                                      uint8_t fillByte) {
    uint8_t grid[64];
    int offset = PaletteChunkParser::calculateOffset(counter);
    for (unsigned char& i: grid) { i = fillByte; }
    PaletteChunkParser::putBlocks(writeVector, grid, writeOffset, offset);
}


int PaletteChunkParser::calculateOffset(int value) {
    int num = value / 32;
    int num2 = value % 32;
    return num / 4 * 64 + num % 4 * 4 + num2 * 1024;
}


void PaletteChunkParser::maxBlocks(uint8_t const* buffer, uint8_t* grid) {
    std::copy_n(buffer, 64, grid);
}


void PaletteChunkParser::putBlocks(
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

template<size_t BitsPerBlock>
bool PaletteChunkParser::parse(uint8_t const* buffer, uint8_t* grid) {
    int size = (1 << BitsPerBlock);
    std::vector<uint8_t> palette(size);
    std::copy_n(buffer, size, palette.begin());
    int totalIndices = BitsPerBlock * 8;
    int blocksPerByte = 8 / BitsPerBlock;
    int gridIndex = 0;
    for (size_t i = 0; i < totalIndices; i++) {
        uint16_t v = buffer[size + i];
        for (int j = 0; j < blocksPerByte; j++) {
            uint16_t idx = 0;
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
