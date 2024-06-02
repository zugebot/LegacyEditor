#pragma once

#include "Utils/Block.hpp"
#include "Utils/Enums.hpp"
#include "Utils/NBT.hpp"
#include "Utils/processor.hpp"

#include "LCE/AquaticChunkParser.hpp"
#include "LCE/SaveFile.hpp"
#include "Universal/universalData.hpp"


class LoadedChunks;
struct LCEFixes;

class LCE_universal {
private:
    static inline bool willBeWater(uint16_t block1_13, uint8_t data);
    static inline uint8_t convertTo1_12Blocks(uint16_t block1_13, uint8_t& data);
    static uint8_t convertBlockFrom1_13(LoadedChunks& adjacentChunks, BlockPos& blockPos);
    static NBTTagList* convertEntities(NBTTagList* entitiesNbt);
    static NBTTagList* convertTileEntities(NBTTagList* tileEntitiesNbt, UniversalChunkFormat* chunkData);
    static NBTTagList* convertTileTicks(NBTTagList* tileTicksNbt);
    static inline void convertPortal(uint8_t* dataOut, BlockWithPos& blockWithPos, LoadedChunks& chunks);
    static inline uint8_t convertDoorNew(uint8_t data);
    static inline uint8_t convertDoorOld(uint8_t data);
    static inline void convertFlowerPotData(uint8_t data, NBTTagCompound* tileEntity);
    static inline NBTTagCompound* convertTileEntityFromData(UniversalChunkFormat* currentChunk, BlockWithPos& blockWithPos, int xPos, int zPos, const std::string& id);
    static int checkChestRotationViableBlock(uint8_t block);
    static uint8_t alignDoubleChest(BlockPos blockPos, LoadedChunks& adjacentChunks, int direction);
    static inline void checkForFixBlock(uint16_t block, uint8_t data, bool waterLogged, int x, int y, int z, LCEFixes& fixes);

public:
    static uint8_t alignChest(BlockPos blockPos, LoadedChunks& adjacentChunks);
    static void applyFixes(UniversalChunkFormat* chunkData, LoadedChunks& adjacentChunks, LCEFixes& fixes);
    static UniversalChunkFormat* convertNBTChunkToUniversal(DataInputManager& input, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCE1_13RegionToUniversal(AquaticChunkData& chunkData, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCE1_12RegionToUniversal(LCEChunkData& chunkData, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCEChunkToUniversal(int xChunk, int zChunk, int realChunkX, int realChunkZ, std::vector<RegionFile>& dimData, int currentRegion, DIMENSION dimension, LCEFixes& fixes);

    static UniversalChunkFormat* convertNBTChunkToUniversalForAccess(DataInputManager& input, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCE1_13RegionToUniversalForAccess(AquaticChunkData& chunkData, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCE1_12RegionToUniversalForAccess(LCEChunkData& chunkData, DIMENSION dimension, LCEFixes& fixes);
    static UniversalChunkFormat* convertLCEChunkToUniversalForAccess(int xChunk, int zChunk, std::vector<RegionFile>& dimData, int currentRegion, DIMENSION dimension, LCEFixes& fixes);
};
