#pragma once

#include "LegacyEditor/utils/processor.hpp"

#include "LegacyEditor/utils/dataManager.hpp"
#include "NBTTagList.hpp"
#include "NBTTagString.hpp"
#include "NBTTagTypeArray.hpp"
#include "NBTType.hpp"

/*
template<typename T>
concept NBT_TAG_TYPE = std::is_same_v<T, NBTTagByteArray> ||
                       std::is_same_v<T, NBTTagString> ||
                       std::is_same_v<T, NBTTagList> ||
                       std::is_same_v<T, NBTTagCompound> ||
                       std::is_same_v<T, NBTTagIntArray> ||
                       std::is_same_v<T, NBTTagLongArray>;
*/

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

    // NBT_TAG_TYPE
    template<class classType>
    classType* toType() const {
        return static_cast<classType*>(this->data);
    }

    template<class classType>
    static classType* toType(const NBTBase* nbtPtr) {
        return static_cast<classType*>(nbtPtr->data);
    }

    template<class classType>
    static classType* toType(const NBTBase& nbtPtr) {
        return static_cast<classType*>(nbtPtr.data);
    }
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


NBTBase convertType(NBTBase baseData, NBTType toType);


NBTBase* createNewByType(NBTType id);

