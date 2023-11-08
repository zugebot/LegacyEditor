#include "NBTTagCompound.hpp"

#include "NBTBase.hpp"
#include "NBTTagList.hpp"
#include "LegacyEditor/utils/DataManager.hpp"


void NBTTagCompound::writeEntry(const std::string& name, NBTBase data, DataManager& output) {
    int id = data.getId();
    output.writeByte(id);
    if (id != 0) {
        output.writeUTF(name);
        data.write(output);
    }
}


std::vector<std::string> NBTTagCompound::getKeySet() {
    std::vector<std::string> keySet;
    auto it = tagMap.begin();
    int size = (int) tagMap.size();
    for (int x = 0; x < size; x++) {
        keySet.push_back(it->first);
        it++;
    }
    return keySet;
}


int NBTTagCompound::getSize() const { return (int) tagMap.size(); }


void NBTTagCompound::setTag(const std::string& key, NBTBase value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = value;
}


void NBTTagCompound::setByte(const std::string& key, uint8_t value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 1, NBTType::NBT_INT8);
}


void NBTTagCompound::setShort(const std::string& key, short value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 2, NBTType::NBT_INT16);
}


void NBTTagCompound::setInteger(const std::string& key, int value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 4, NBTType::NBT_INT32);
}


void NBTTagCompound::setLong(const std::string& key, i64 value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 8, NBTType::NBT_INT64);
}

/*
void NBTTagCompound::setUniqueId(const std::string& key, UUIDJava value) {
    setLong(key + "Most", value.getMostSignificantBits());
    setLong(key + "Least", value.getLeastSignificantBits());
}


UUIDJava NBTTagCompound::getUniqueId(const std::string& key) {
    return {getPrimitive<i64>(key + "Most"), getPrimitive<i64>(key + "Least")};
}
*/

bool NBTTagCompound::hasUniqueId(const std::string& key) {
    return hasKey(key + "Most", NBTType::TAG_PRIMITIVE) && hasKey(key + "Least", NBTType::TAG_PRIMITIVE);
}


void NBTTagCompound::setFloat(const std::string& key, float value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 4, NBTType::NBT_FLOAT);
}


void NBTTagCompound::setDouble(const std::string& key, double value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(&value, 8, NBTType::NBT_DOUBLE);
}


void NBTTagCompound::setString(const std::string& key, const std::string& value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(new NBTTagString(value), NBTType::TAG_STRING);
}

void NBTTagCompound::setByteArray(const std::string& key, uint8_t* value, int size) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    auto* data = (u8*) malloc(size);//so the original can be safely deleted
    memcpy(data, value, size);
    tagMap[key] = NBTBase(new NBTTagByteArray(data, size), NBTType::TAG_BYTE_ARRAY);
}


void NBTTagCompound::setIntArray(const std::string& key, int* value, int size) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    int* data = (int*) malloc(size * 4); // so the original can be safely deleted
    memcpy((void*) data, value, size * 4);
    tagMap[key] = NBTBase(new NBTTagIntArray(data, size), NBTType::TAG_INT_ARRAY);
}


void NBTTagCompound::setLongArray(const std::string& key, i64* value, int size) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    auto* data = (i64*) malloc(size * 8);//so the original can be safely deleted
    memcpy((void*) data, value, size * 8);   //the endianness is maintained because it is copied raw
    tagMap[key] = NBTBase(new NBTTagLongArray(data, size), NBTType::TAG_LONG_ARRAY);
}


void NBTTagCompound::setCompoundTag(const std::string& key, NBTTagCompound* compoundTag) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(compoundTag, NBTType::TAG_COMPOUND);
}


void NBTTagCompound::setListTag(const std::string& key, NBTTagList* listTag) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    tagMap[key] = NBTBase(listTag, NBTType::TAG_LIST);
}


void NBTTagCompound::setBool(const std::string& key, uint8_t value) {
    if (tagMap.find(key) != tagMap.end()) tagMap[key].NbtFree();
    value = value ? 1 : 0;
    tagMap[key] = NBTBase(&value, 1, NBTType::NBT_INT8);
}


NBTBase NBTTagCompound::getTag(const std::string& key) {
    auto it = tagMap.find(key);
    if (it != tagMap.end()) return it->second;
    return {};
}


NBTType NBTTagCompound::getTagId(const std::string& key) {
    NBTBase nbtBase = getTag(key);
    return nbtBase.getId();
}


void NBTTagCompound::deleteAll() {
    for (auto& pair: tagMap) { pair.second.NbtFree(); }
    tagMap.clear();
}


bool NBTTagCompound::hasKey(const std::string& key) {
    if (tagMap.find(key) != tagMap.end()) return true;
    return false;
}


bool NBTTagCompound::hasKey(const std::string& key, int type) {
    if (hasKey(key)) {
        int i = getTagId(key);
        if (i == type) {
            return true;
        } else if (type != NBTType::TAG_PRIMITIVE) {
            return false;
        }
        return i == 1 || i == 2 || i == 3 || i == 4 || i == 5 || i == 6;
    }
    return false;
}


bool NBTTagCompound::hasKey(const std::string& key, NBTType type) {
    if (hasKey(key)) {
        int i = getTagId(key);
        if (i == type) {
            return true;
        } else if (type != NBTType::TAG_PRIMITIVE) {
            return false;
        }
        return i == 1 || i == 2 || i == 3 || i == 4 || i == 5 || i == 6;
    }
    return false;
}


std::string NBTTagCompound::getString(const std::string& key) {
    if (hasKey(key, NBTType::TAG_STRING)) {
        return NBTBase::toType<NBTTagString>(tagMap.at(key))->getString();
    }
    return "";
}


NBTTagByteArray* NBTTagCompound::getByteArray(const std::string& key) {
    if (hasKey(key, NBTType::TAG_BYTE_ARRAY)) {
        NBTBase byteArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagByteArray>(byteArrayBase);
    }
    return nullptr;
}


NBTTagIntArray* NBTTagCompound::getIntArray(const std::string& key) {
    if (hasKey(key, NBTType::TAG_INT_ARRAY)) {
        NBTBase intArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagIntArray>(intArrayBase);
    }
    return nullptr;
}


NBTTagLongArray* NBTTagCompound::getLongArray(const std::string& key) {
    if (hasKey(key, NBTType::TAG_LONG_ARRAY)) {
        NBTBase longArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagLongArray>(longArrayBase);
    }
    return nullptr;
}


NBTTagCompound* NBTTagCompound::getCompoundTag(const std::string& key) {
    if (hasKey(key, NBTType::TAG_COMPOUND)) {
        NBTBase base = tagMap.at(key);
        return NBTBase::toType<NBTTagCompound>(base);
    }
    return nullptr;
}


NBTTagList* NBTTagCompound::getListTag(const std::string& key) {
    if (hasKey(key, NBTType::TAG_LIST)) {
        return NBTBase::toType<NBTTagList>(tagMap.at(key));
    }
    return nullptr;
}


bool NBTTagCompound::getBool(const std::string& key) { return getPrimitive<bool>(key); }


void NBTTagCompound::removeTag(const std::string& key) {
    if (!hasKey(key)) return;
    tagMap.at(key).NbtFree();
    tagMap.erase(key);
}


bool NBTTagCompound::hasNoTags() const { return tagMap.empty(); }

void NBTTagCompound::merge(NBTTagCompound* other) {
    auto it = other->tagMap.begin();
    int size = (int) other->tagMap.size();
    for (int x = 0; x < size; x++) {
        std::string key = it->first;
        NBTBase nbtBase = it->second;
        if (nbtBase.getId() == NBTType::TAG_COMPOUND) {
            if (hasKey(key, NBTType::TAG_COMPOUND)) {
                NBTTagCompound* pNbtTagCompound = getCompoundTag(key);
                pNbtTagCompound->merge(NBTBase::toType<NBTTagCompound>(nbtBase));
            } else {
                setTag(key, nbtBase.copy());
            }
        } else {
            setTag(key, nbtBase.copy());
        }
        it++;
    }
}


template<typename classType>
classType NBTTagCompound::getPrimitive(STR key) {
    if (hasKey(key, NBTType::TAG_PRIMITIVE)) {
        return tagMap.at(key).toPrimitiveType<classType>();
    }
    return (classType) 0;
}



template bool NBTTagCompound::getPrimitive<bool>(STR key);
