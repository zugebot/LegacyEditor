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
    static inline bool willBeWater(uint16_t block1_13, u8 data);
    static inline u8 convertTo1_12Blocks(uint16_t block1_13, u8& data);
    static u8 convertBlockFrom1_13(LoadedChunks& adjacentChunks, BlockPos& blockPos);
    static NBTTagList* convertEntities(NBTTagList* entitiesNbt);
    static NBTTagList* convertTileEntities(NBTTagList* tileEntitiesNbt, UniversalChunkFormat* chunkData);
    static NBTTagList* convertTileTicks(NBTTagList* tileTicksNbt);
    static inline void convertPortal(u8* dataOut, BlockWithPos& blockWithPos, LoadedChunks& chunks);
    static inline u8 convertDoorNew(u8 data);
    static inline u8 convertDoorOld(u8 data);
    static inline void convertFlowerPotData(u8 data, NBTTagCompound* tileEntity);
    static inline NBTTagCompound* convertTileEntityFromData(UniversalChunkFormat* currentChunk, BlockWithPos& blockWithPos, int xPos, int zPos, const std::string& id);
    static int checkChestRotationViableBlock(u8 block);
    static u8 alignDoubleChest(BlockPos blockPos, LoadedChunks& adjacentChunks, int direction);
    static inline void checkForFixBlock(uint16_t block, u8 data, bool waterLogged, int x, int y, int z, LCEFixes& fixes);

public:
    static u8 alignChest(BlockPos blockPos, LoadedChunks& adjacentChunks);
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
