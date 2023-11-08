#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "LegacyEditor/utils/processor.hpp"

// #include "Block.hpp"
// #include "UUID.hpp"

#include "NBTType.hpp"

class NBTBase;
class DataManager;


class NBT {
public:
    static inline bool isCompoundTag(NBTType type) { return type == TAG_COMPOUND; }
    static void writeTag(NBTBase* tag, DataManager& output);
    static NBTBase* readTag(DataManager& input);
    static NBTBase* readNBT(NBTType id, const std::string& key, DataManager& input);

    // static u8* writeCompressedTag(NBTBase* rootData, i64* outSize);
    // static u8* writeGZIPCompressedTag(NBTBase* rootData, i64* outSize);
};




/*
class NBTUtil {
public:
    static UUIDJava getUUIDFromTag(NBTTagCompound* tag);
    static NBTTagCompound* createUUIDTag(UUIDJava uuid);
    static BlockPos getPosFromTag(NBTTagCompound* tag);
    static NBTTagCompound* createPosTag(BlockPos pos);
};
 */

