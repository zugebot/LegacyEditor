#pragma once

#include <unordered_map>

#include "LegacyEditor/utils/processor.hpp"

#include "NBTType.hpp"
#include "NBTTagTypeArray.hpp"

class DataManager;
class NBTBase;
class NBTTagList;


class NBTTagCompound {
private:
    typedef const std::string& STR;

public:
    std::unordered_map<std::string, NBTBase> tagMap;

    static void writeEntry(STR name, NBTBase data, DataManager& output);
    int getSize() const;
    void setTag(STR key, NBTBase value);
    void setByte(STR key, u8 value);
    void setShort(STR key, i16 value);
    void setInteger(STR key, i32 value);
    void setLong(STR key, i64 value);

    // void setUniqueId(STR key, UUIDJava value);
    // UUIDJava getUniqueId(STR key);

    bool hasUniqueId(STR key);
    void setFloat(STR key, float value);
    void setDouble(STR key, double value);
    void setString(STR key, STR value);
    void setByteArray(STR key, u8* value, int size);
    void setIntArray(STR key, int* value, int size);
    void setLongArray(STR key, i64* value, int size);
    void setCompoundTag(STR key, NBTTagCompound* compoundTag);
    void setListTag(STR key, NBTTagList* listTag);
    void setBool(STR key, u8 value);
    NBTBase getTag(STR key);
    NBTType getTagId(STR key);

    bool hasKey(STR key);
    bool hasKey(STR key, int type);
    bool hasKey(STR key, NBTType type);
    std::vector<std::string> getKeySet();

    template<typename classType>
    classType getPrimitive(STR key);
    std::string getString(STR key);
    NBTTagByteArray* getByteArray(STR key);
    NBTTagIntArray* getIntArray(STR key);
    NBTTagLongArray* getLongArray(STR key);
    NBTTagCompound* getCompoundTag(STR key);
    NBTTagList* getListTag(STR key);
    bool getBool(STR key);
    void removeTag(STR key);
    ND bool hasNoTags() const;
    void merge(NBTTagCompound* other);

    /// it will free everything inside the tag map
    /// so no need to worry about memory management
    void deleteAll();
};

