#pragma once


#include "Utils/Enums.hpp"
#include "Utils/NBT.hpp"
#include "Utils/processor.hpp"

#include "LCE/AquaticChunkParser.hpp"


class PaletteChunkParser {
public:
    LCEChunkData LCE_ChunkData;

    ~PaletteChunkParser();

    UniversalChunkFormat* ParseChunk(DataInputManager& inputData, DIMENSION dimension, LCEFixes& fixes, int version);
    UniversalChunkFormat* ParseChunkForAccess(DataInputManager& inputData, DIMENSION dimension, LCEFixes& fixes);

    void readNBTData(DataInputManager& inputData);
    void readData(DataInputManager& inputData);
    void readDataOnlyForAccess(DataInputManager& inputData);
    static void copyByte128(std::vector<uint8_t>& writeVector, int writeOffset, uint8_t value);
    static void copyArray128(std::vector<uint8_t>& readVector, int readOffset, std::vector<uint8_t>& writeVector,
                             int writeOffset);
    void parseBlocks(DataInputManager& inputData);
    static void fillWithByte(std::vector<uint8_t>& writeVector, int writeOffset, int counter, uint8_t fillByte);
    static int calculateOffset(int value);
    static void maxBlocks(uint8_t const* buffer, uint8_t* grid);
    static void putBlocks(std::vector<uint8_t>& writeVector, const uint8_t* readArray, int writeOffset, int readOffset);

    static std::vector<uint8_t> read256(DataInputManager& inputData);
    static std::vector<uint8_t> read128(DataInputManager& inputData);

    template<size_t BitsPerBlock>
    bool parse(uint8_t const* buffer, uint8_t* grid);
};
