#pragma once

#include "LegacyEditor/utils/processor.hpp"

#include "NBTBase.hpp"
#include "NBTType.hpp"
#include "NBTTagTypeArray.hpp"

class NBTBase;
class NBTTagCompound;


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

    /*
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
     */

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