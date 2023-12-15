#pragma once

#include <cstring>
#include <unordered_map>
#include <vector>
#include <ranges>

#include "LegacyEditor/utils/processor.hpp"

// #include "Block.hpp"
#include "dataManager.hpp"
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

    NBTTagTypeArray(classType* dataIn, const size_t sizeIn) : array(dataIn), size(sizeIn) {}
    NBTTagTypeArray() = default;

    MU ND classType* getArray() const { return array; }
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

    NBTBase(void* dataIn, const NBTType typeIn) : data(dataIn), type(typeIn) {}

    NBTBase() : NBTBase(nullptr, NBT_NONE) {}

    NBTBase(void* dataIn, const int dataSizeIn, const NBTType typeIn) : NBTBase(dataIn, typeIn) {
        memcpy(data, dataIn, dataSizeIn);
    }

    void write(DataManager& output) const;

    void read(DataManager& input);

    ND std::string toString() const;

    ND NBTBase copy() const;

    void NbtFree() const;

    static bool equals(NBTBase check);

    ND NBTType getId() const { return type; }

    template<class classType>
    classType toPrimitiveType() {
        switch (type) {
            case NBT_INT8:
                return (classType) *(u8*) data;
            case NBT_INT16:
                return (classType) *(i16*) data;
            case NBT_INT32:
                return (classType) *(i32*) data;
            case NBT_INT64:
                return (classType) *(i64*) data;
            case NBT_FLOAT:
                return (classType) *(float*) data;
            case NBT_DOUBLE:
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
        size = static_cast<int>(dataIn.size());
        data = static_cast<char*>(malloc(size));
        memcpy(data, dataIn.c_str(), size);
    }

    ND inline bool hasNoTags() const { return size != 0; }

    ND std::string getString() const { return std::string(data, size); }

    ND std::string toStringNBT() const {
        std::string stringBuilder = "\"";
        for (const std::string currentString = getString(); const char ch : currentString) {
            if (ch == '\\' || ch == '"') { stringBuilder.append("\\"); }

            stringBuilder.push_back(ch);
        }
        return stringBuilder.append("\"");
    }
};


class NBTTagList;

class NBTTagCompound {
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
    void setByteArray(STR key, const u8* value, int size);
    void setIntArray(STR key, const int* value, int size);
    void setLongArray(STR key, const i64* value, int size);
    void setCompoundTag(STR key, NBTTagCompound* compoundTag);
    void setListTag(STR key, NBTTagList* listTag);
    void setBool(STR key, u8 value);

    bool hasUniqueId(STR key);
    NBTBase getTag(STR key);
    NBTType getTagId(STR key);

    bool hasKey(STR key) const;
    bool hasKey(STR key, int type);
    bool hasKey(STR key, NBTType type);
    std::vector<std::string> getKeySet();
    template<typename classType>
    classType getPrimitive(STR key) {
        if (hasKey(key, TAG_PRIMITIVE)) {
            return tagMap.at(key).toPrimitiveType<classType>();
        }
        return static_cast<classType>(0);
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

    NBTTagList() : tagType(NBT_NONE) {}

    void appendTag(NBTBase nbt);
    void set(int index, NBTBase nbt);
    void insert(int index, NBTBase nbt);
    void removeTag(int index);
    void deleteAll();
    ND bool hasNoTags() const;

    template<typename classType>
    classType getPrimitiveAt(const int index) {
        if (tagType < 7 && tagType > 0) {
            if (index >= 0 && index < tagList.size()) {
                NBTBase nbtBase = tagList.at(index);
                return nbtBase.toPrimitiveType<classType>();
            }
        }

        return static_cast<classType>(0);
    }

    NBTTagByteArray* getByteArrayAt(int index) const;
    std::string getStringTagAt(int index) const;
    NBTTagList* getListTagAt(int index) const;
    NBTTagCompound* getCompoundTagAt(int index) const;
    NBTTagIntArray* getIntArrayAt(int index) const;
    NBTTagLongArray* getLongArrayAt(int index) const;
    NBTBase get(int index) const;
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
    static bool isCompoundTag(const NBTType type) { return type == TAG_COMPOUND; }
    static void writeTag(const NBTBase* tag, DataManager& output);
    static NBTBase* readTag(DataManager& input);
    static NBTBase* readNBT(NBTType tagID, const std::string& key, DataManager& input);
};


static NBTBase createNBT_INT8(const int8_t dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_INT8;
    return nbtBase;
}


static NBTBase createNBT_INT16(const i16 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_INT16;
    return nbtBase;
}


static NBTBase createNBT_INT32(const i32 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_INT32;
    return nbtBase;
}


static NBTBase createNBT_INT64(const i64 dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_INT64;
    return nbtBase;
}


static NBTBase createNBT_FLOAT(const float dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_FLOAT;
    return nbtBase;
}


static NBTBase createNBT_DOUBLE(const double dataIn) {
    NBTBase nbtBase;
    nbtBase.data = malloc(sizeof(dataIn));
    memcpy(nbtBase.data, &dataIn, sizeof(dataIn));
    nbtBase.type = NBT_DOUBLE;
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


inline NBTBase* createNewByType(NBTType type) {
    switch (type) {
        case NBT_NONE:
        case NBT_INT8:
        case NBT_INT16:
        case NBT_INT32:
        case NBT_INT64:
        case NBT_FLOAT:
        case NBT_DOUBLE:
        default:
            return new NBTBase(nullptr, type);
        case TAG_BYTE_ARRAY:
            return new NBTBase(new NBTTagByteArray(), type);
        case TAG_STRING:
            return new NBTBase(new NBTTagString(), type);
        case TAG_LIST:
            return new NBTBase(new NBTTagList(), type);
        case TAG_COMPOUND:
            return new NBTBase(new NBTTagCompound(), type);
        case TAG_INT_ARRAY:
            return new NBTBase(new NBTTagIntArray(), type);
        case TAG_LONG_ARRAY:
            return new NBTBase(new NBTTagLongArray(), type);
    }
}


static void compareNBT(const NBTBase* first, const NBTBase* second) {
    auto* firstNBT = NBTBase::toType<NBTTagCompound>(first)->getCompoundTag("Data");
    auto* secondNBT = NBTBase::toType<NBTTagCompound>(second)->getCompoundTag("Data");;

    for (const auto& key : firstNBT->tagMap | std::views::keys) {
        if (!secondNBT->hasKey(key)) {
            printf("second does not contain tag '%s'\n", key.c_str());
        }
    }

    for (const auto& key : secondNBT->tagMap | std::views::keys) {
        if (!firstNBT->hasKey(key)) {
            printf("first does not contain tag '%s'\n", key.c_str());
        }
    }
}