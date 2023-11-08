#pragma once

#include "LegacyEditor/utils/processor.hpp"


template<class classType>
class NBTTagTypeArray {
public:
    classType* array = nullptr;
    int size = 0;

    NBTTagTypeArray(classType* dataIn, int sizeIn)
        : array(dataIn), size(sizeIn) {}

    NBTTagTypeArray() = default;

    MU ND inline classType* getArray() const { return array; }
};


using NBTTagByteArray = NBTTagTypeArray<u8>;
using NBTTagIntArray = NBTTagTypeArray<i32>;
using NBTTagLongArray = NBTTagTypeArray<i64>;
