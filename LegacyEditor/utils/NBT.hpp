#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "LegacyEditor/utils/processor.hpp"

// #include "Block.hpp"
#include "DataManager.hpp"
// #include "UUID.hpp"
#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"

class DataManager;

enum NBTType : u8 {
    NBT_NONE = 0,
    NBT_INT8 = 1,
    NBT_INT16 = 2,
    NBT_INT32 = 3,
    NBT_INT64 = 4,
    NBT_FLOAT = 5,
    NBT_DOUBLE = 6,
    TAG_BYTE_ARRAY = 7,
    TAG_STRING = 8,
    TAG_LIST = 9,
    TAG_COMPOUND = 10,
    TAG_INT_ARRAY = 11,
    TAG_LONG_ARRAY = 12,
    TAG_PRIMITIVE = 99
};


template<class classType>
class NBTTagTypeArray {
public:
    classType* array = nullptr;
    int size = 0;

    NBTTagTypeArray(classType* dataIn, int sizeIn) : array(dataIn), size(sizeIn) {}
    NBTTagTypeArray() = default;

    MU ND inline classType* getArray() const { return array; }
};


using NBTTagByteArray = NBTTagTypeArray<u8>;
using NBTTagIntArray = NBTTagTypeArray<i32>;
using NBTTagLongArray = NBTTagTypeArray<i64>;


class NBTTagString;
class NBTTagList;
class NBTTagCompound;

template<typename T>
concept NBT_TAG_TYPE = std::is_same_v<T, NBTTagByteArray> ||
                       std::is_same_v<T, NBTTagString> ||
                       std::is_same_v<T, NBTTagList> ||
                       std::is_same_v<T, NBTTagCompound> ||
                       std::is_same_v<T, NBTTagIntArray> ||
                       std::is_same_v<T, NBTTagLongArray>;


class NBTBase {
public:
    void* data;
    NBTType type;

    NBTBase(void* dataIn, NBTType typeIn) : data(dataIn), type(typeIn) {}

    NBTBase() : NBTBase(nullptr, NBTType::NBT_NONE) {}

    NBTBase(void* dataIn, int dataSizeIn, NBTType typeIn) : NBTBase(dataIn, typeIn) {
        memcpy(data, dataIn, dataSizeIn);
    }

    void write(DataManager& output) const;

    void read(DataManager& input);

    ND std::string toString() const;

    ND NBTBase copy() const;

    void NbtFree() const;

    static bool equals(NBTBase check);

    ND inline NBTType getId() const { return type; }

    template<class classType>
    classType toPrimitiveType() {
        switch (type) {
            case NBTType::NBT_INT8:
                return (classType) *(u8*) data;
            case NBTType::NBT_INT16:
                return (classType) *(i16*) data;
            case NBTType::NBT_INT32:
                return (classType) *(i32*) data;
            case NBTType::NBT_INT64:
                return (classType) *(i64*) data;
            case NBTType::NBT_FLOAT:
                return (classType) *(float*) data;
            case NBTType::NBT_DOUBLE:
                return (classType) *(double*) data;
            default:
                return 0;
        }
    }

    template<NBT_TAG_TYPE classType>
    classType* toType() const {
        return static_cast<classType*>(this->data);
    }

    template<NBT_TAG_TYPE classType>
    static classType* toType(const NBTBase* nbtPtr) {
        return static_cast<classType*>(nbtPtr->data);
    }

    template<NBT_TAG_TYPE classType>
    static classType* toType(const NBTBase& nbtPtr) {
        return static_cast<classType*>(nbtPtr.data);
    }
};



class NBTTagString {
public:
    char* data;
    i64 size;
    NBTTagString() : data(nullptr), size(0) {}

    explicit NBTTagString(const std::string& dataIn) {
        size = (int) dataIn.size();
        data = (char*) malloc(size);
        memcpy(data, dataIn.c_str(), size);
    }

    ND inline bool hasNoTags() const { return size; }

    ND std::string getString() const { return std::string(data, size); }

    ND std::string toStringNBT() const {
        std::string stringBuilder = "\"";
        std::string currentString = getString();
        for (char c0: currentString) {
            if (c0 == '\\' || c0 == '"') { stringBuilder.append("\\"); }

            stringBuilder.push_back(c0);
        }
        return stringBuilder.append("\"");
    }
};


class NBTTagList;

class NBTTagCompound {
private:
    typedef const std::string& STR;

public:
    std::unordered_map<std::string, NBTBase> tagMap;

    static void writeEntry(STR name, NBTBase data, DataManager& output);
    int getSize() const;

    // set tags
    void setTag(STR key, NBTBase value);
    void setByte(STR key, u8 value);
    void setShort(STR key, i16 value);
    void setInteger(STR key, i32 value);
    void setLong(STR key, i64 value);
    void setFloat(STR key, float value);
    void setDouble(STR key, double value);
    void setString(STR key, STR value);
    void setByteArray(STR key, u8* value, int size);
    void setIntArray(STR key, int* value, int size);
    void setLongArray(STR key, i64* value, int size);
    void setCompoundTag(STR key, NBTTagCompound* compoundTag);
    void setListTag(STR key, NBTTagList* listTag);
    void setBool(STR key, u8 value);

    bool hasUniqueId(STR key);
    NBTBase getTag(STR key);
    NBTType getTagId(STR key);

    bool hasKey(STR key);
    bool hasKey(STR key, int type);
    bool hasKey(STR key, NBTType type);
    std::vector<std::string> getKeySet();
    template<typename classType>
    classType getPrimitive(STR key) {
        if (hasKey(key, NBTType::TAG_PRIMITIVE)) {
            return tagMap.at(key).toPrimitiveType<classType>();
        }
        return (classType) 0;
    }
    std::string getString(STR key);
    NBTTagByteArray* getByteArray(STR key);
    NBTTagIntArray* getIntArray(STR key);
    NBTTagLongArray* getLongArray(STR key);
    NBTTagCompound* getCompoundTag(STR key);
    NBTTagList* getListTag(STR key);
    bool getBool(STR key);
    void removeTag(STR key);
    bool hasNoTags() const;
    void merge(NBTTagCompound* other);

    /// it will free everything inside the tag map
    /// so no need to worry about memory management
    void deleteAll();
};


class NBTTagList {
public:
    std::vector<NBTBase> tagList;
    NBTType tagType;

    NBTTagList() : tagType(NBTType::NBT_NONE) {}

    void appendTag(NBTBase nbt);
    void set(int index, NBTBase nbt);
    void insert(int index, NBTBase nbt);
    void removeTag(int index);
    void deleteAll();
    ND bool hasNoTags() const;

    template<typename classType>
    classType getPrimitiveAt(int index) {
        if (tagType < 7 && tagType > 0) {
            if (index >= 0 && index < tagList.size()) {
                NBTBase nbtBase = tagList.at(index);
                return nbtBase.toPrimitiveType<classType>();
            }
        }

        return (classType) 0;
    }

    NBTTagByteArray* getByteArrayAt(int index);
    std::string getStringTagAt(int index);
    NBTTagList* getListTagAt(int index);
    NBTTagCompound* getCompoundTagAt(int index);
    NBTTagIntArray* getIntArrayAt(int index);
    NBTTagLongArray* getLongArrayAt(int index);
    NBTBase get(int index);
    ND int tagCount() const;
    ND NBTType getTagType() const;
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


class NBT {
public:
    static inline bool isCompoundTag(NBTType type) { return type == NBTType::TAG_COMPOUND; }
    static void writeTag(NBTBase* tag, DataManager& output);
    static NBTBase* readTag(DataManager& input);
    static NBTBase* readNBT(NBTType id, const std::string& key, DataManager& input);
};


static NBTBase createNBT_INT8(int8_t dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_INT8;
    return nbtBase;
}


static NBTBase createNBT_INT16(i16 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_INT16;
    return nbtBase;
}


static NBTBase createNBT_INT32(i32 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_INT32;
    return nbtBase;
}


static NBTBase createNBT_INT64(i64 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_INT64;
    return nbtBase;
}


static NBTBase createNBT_FLOAT(float dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_FLOAT;
    return nbtBase;
}


static NBTBase createNBT_DOUBLE(double dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBTType::NBT_DOUBLE;
    return nbtBase;
}


static NBTBase convertType(NBTBase baseData, NBTType toType) {
    switch (toType) {
        case NBT_INT8: {
            auto valueB = baseData.toPrimitiveType<u8>();
            return {&valueB, 1, NBT_INT8};
        }
        case NBT_INT16: {
            auto valueS = baseData.toPrimitiveType<i16>();
            return {&valueS, 2, NBT_INT16};
        }
        case NBT_INT32: {
            int value = baseData.toPrimitiveType<i32>();
            return {&value, 4, NBT_INT32};
        }
        case NBT_INT64: {
            auto valueL = baseData.toPrimitiveType<i64>();
            return {&valueL, 8, NBT_INT64};
        }
        case NBT_FLOAT: {
            auto valueF = baseData.toPrimitiveType<float>();
            return {&valueF, 4, NBT_FLOAT};
        }
        case NBT_DOUBLE: {
            auto valueD = baseData.toPrimitiveType<double>();
            return {&valueD, 8, NBT_DOUBLE};
        }
        default:
            return baseData.copy();
    }
}


inline NBTBase* createNewByType(NBTType id) {
    switch (id) {
        case NBTType::NBT_NONE:
        case NBTType::NBT_INT8:
        case NBTType::NBT_INT16:
        case NBTType::NBT_INT32:
        case NBTType::NBT_INT64:
        case NBTType::NBT_FLOAT:
        case NBTType::NBT_DOUBLE:
        default:
            return new NBTBase(nullptr, id);
        case NBTType::TAG_BYTE_ARRAY:
            return new NBTBase(new NBTTagByteArray(), id);
        case NBTType::TAG_STRING:
            return new NBTBase(new NBTTagString(), id);
        case NBTType::TAG_LIST:
            return new NBTBase(new NBTTagList(), id);
        case NBTType::TAG_COMPOUND:
            return new NBTBase(new NBTTagCompound(), id);
        case NBTType::TAG_INT_ARRAY:
            return new NBTBase(new NBTTagIntArray(), id);
        case NBTType::TAG_LONG_ARRAY:
            return new NBTBase(new NBTTagLongArray(), id);
    }
}
