#include "NBTTagList.hpp"


void NBTTagList::appendTag(NBTBase nbt) {
    if (tagType == NBTType::NBT_NONE) {
        tagType = nbt.getId();
    } else if (tagType != nbt.getId()) {
        nbt.NbtFree();
        return;
    }
    tagList.push_back(nbt);
}


void NBTTagList::set(int index, NBTBase nbt) {
    if (index >= 0 && index < tagList.size()) {
        tagList[index].NbtFree();
        tagList[index] = nbt;
    } else {
        nbt.NbtFree();
    }
}


void NBTTagList::insert(int index, NBTBase nbt) {
    if (index >= 0 && index < tagList.size()) {
        tagList.insert(tagList.begin() + index, nbt);
    } else {
        nbt.NbtFree();
    }
}


void NBTTagList::removeTag(int index) {
    if (index >= 0 && index < tagList.size()) {
        tagList[index].NbtFree();
        tagList.erase(tagList.begin() + index);
    }
}


void NBTTagList::deleteAll() {
    for (auto& pair: tagList) { pair.NbtFree(); }
    tagList.clear();
}


bool NBTTagList::hasNoTags() const { return tagList.empty(); }


NBTBase NBTTagList::get(int index) {
    return (index >= 0 && index < tagList.size() ? tagList[index] : NBTBase(nullptr, NBTType::NBT_NONE));
}


NBTTagByteArray* NBTTagList::getByteArrayAt(int index) {
    if (tagType == NBTType::TAG_BYTE_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagByteArray>(nbtBase);
        }
    }
    return nullptr;
}


std::string NBTTagList::getStringTagAt(int index) {
    if (tagType == NBTType::TAG_STRING) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagString>(nbtBase)->getString();
        }
    }
    return "";
}


NBTTagList* NBTTagList::getListTagAt(int index) {
    if (tagType == NBTType::TAG_LIST) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagList>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagCompound* NBTTagList::getCompoundTagAt(int index) {
    if (tagType == NBTType::TAG_COMPOUND) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagCompound>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagIntArray* NBTTagList::getIntArrayAt(int index) {
    if (tagType == NBTType::TAG_INT_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagIntArray>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagLongArray* NBTTagList::getLongArrayAt(int index) {
    if (tagType == NBTType::TAG_LONG_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagLongArray>(nbtBase);
        }
    }
    return nullptr;
}


int NBTTagList::tagCount() const {
    return (int) tagList.size();
}


NBTType NBTTagList::getTagType() const {
    return tagType;
}



