#pragma once


#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/error_status.hpp"


#include "LegacyEditor/LCE/AquaticChunkParser.hpp"



class PaletteChunkParser {
public:
    LCEChunkData LCE_ChunkData;

    ~PaletteChunkParser();

    UniversalChunkFormat* ParseChunk(DataInputManager& inputData, DIMENSION dimension, LCEFixes& fixes, int version);
    UniversalChunkFormat* ParseChunkForAccess(DataInputManager& inputData, DIMENSION dimension, LCEFixes& fixes);

    void readNBTData(DataInputManager& inputData);
    void readData(DataInputManager& inputData);
    void readDataOnlyForAccess(DataInputManager& inputData);
    static void copyByte128(std::vector<u8>& writeVector, int writeOffset, u8 value);
    static void copyArray128(std::vector<u8>& readVector, int readOffset, std::vector<u8>& writeVector,
                             int writeOffset);
    void parseBlocks(DataInputManager& inputData);
    static void fillWithByte(std::vector<u8>& writeVector, int writeOffset, int counter, u8 fillByte);
    static int calculateOffset(int value);
    static void maxBlocks(u8 const* buffer, u8* grid);
    static void putBlocks(std::vector<u8>& writeVector, c_u8 * readArray, int writeOffset, int readOffset);

    static std::vector<u8> read256(DataInputManager& inputData);
    static std::vector<u8> read128(DataInputManager& inputData);

    template<size_t BitsPerBlock>
    bool parse(u8 const* buffer, u8* grid);
};
