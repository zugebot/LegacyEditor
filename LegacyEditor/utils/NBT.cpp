#include "NBT.hpp"


void NBTBase::write(DataManager& output) const {
    switch (type) {
        case NBT_INT8: {
            u8 writeVal = 0;
            memcpy(&writeVal, data, 1);
            output.writeByte(writeVal);
            return;
        }
        case NBT_INT16: {
            i16 writeVal = 0;
            memcpy(&writeVal, data, 2);
            output.writeInt16(writeVal);
            return;
        }
        case NBT_INT32: {
            i32 writeVal = 0;
            memcpy(&writeVal, data, 4);
            output.writeInt32(writeVal);
            return;
        }
        case NBT_INT64: {
            i64 writeVal = 0;
            memcpy(&writeVal, data, 8);
            output.writeInt64(writeVal);
            return;
        }
        case NBT_FLOAT: {
            float writeVal = 0;
            memcpy(&writeVal, data, 4);
            output.writeFloat(writeVal);
            return;
        }
        case NBT_DOUBLE: {
            double writeVal = 0;
            memcpy(&writeVal, data, 8);
            output.writeDouble(writeVal);
            return;
        }
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            output.writeInt32(val->size);
            output.write(val->array, val->size);
            return;
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            output.writeUTF(val->getString());
            return;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            output.writeByte(static_cast<int>(val->tagType));
            output.writeInt32(val->tagList.size());
            for (auto& i: val->tagList) { i.write(output); }
            return;
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            auto it = val->tagMap.begin();
            for (int x = 0; x < val->tagMap.size(); x++) {
                NBTTagCompound::writeEntry(it->first, it->second, output);
                it++;
            }

            output.writeByte(0);
            return;
        }

        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            output.writeInt32(val->size);
            for (int x = 0; x < val->size; x++) { output.writeInt32(val->array[x]); }
            return;
        }

        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            output.writeInt32(val->size);

            for (int x = 0; x < val->size; x++) { output.writeInt64(val->array[x]); }
            return;
        }
        default:
            return;
    }
}

void NBTBase::NbtFree() const {
    switch (type) {
        case NBT_INT8:
        case NBT_INT16:
        case NBT_INT32:
        case NBT_INT64:
        case NBT_FLOAT:
        case NBT_DOUBLE:
            free(this->data);
            return;
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            free(val->array);
            delete val;
            return;
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            free(val->data);
            delete val;
            return;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            val->deleteAll();
            delete val;
            return;
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            val->deleteAll();
            delete val;
            return;
        }
        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            free(val->array);
            delete val;
            return;
        }
        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            free(val->array);
            delete val;
            return;
        }
        default:
            return;
    }
}


std::string NBTBase::toString() const {
    switch (type) {
        case NBT_NONE:
            return "END";
        case NBT_INT8: {
            uint8_t val = 0;
            memcpy(&val, this->data, 1);
            return std::to_string(val) + "b";
        }
        case NBT_INT16: {
            int16_t val = 0;
            memcpy(&val, this->data, 2);
            return std::to_string(val) + "s";
        }
        case NBT_INT32: {
            int val = 0;
            memcpy(&val, this->data, 4);
            return std::to_string(val);
        }
        case NBT_INT64: {
            i64 val = 0;
            memcpy(&val, this->data, 8);
            return std::to_string(val) + "L";
        }
        case NBT_FLOAT: {
            float val = 0;
            memcpy(&val, this->data, 4);
            return std::to_string(val) + "f";
        }
        case NBT_DOUBLE: {
            double val = 0;
            memcpy(&val, this->data, 8);
            return std::to_string(val) + "d";
        }
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            std::string stringBuilder = "[B;";

            for (int i = 0; i < val->size; ++i) {
                if (i != 0) { stringBuilder.append(", "); }
                stringBuilder.append(std::to_string(val->array[i]));
            }
            stringBuilder.push_back(']');
            return stringBuilder;
        }

        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            return val->toStringNBT();
        }

        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            std::string stringBuilder = "[";

            for (int i = 0; i < val->tagList.size(); ++i) {
                if (i != 0) { stringBuilder.append(", "); }

                stringBuilder.append(val->tagList.at(i).toString());
            }
            stringBuilder.push_back(']');
            return stringBuilder;
        }

        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            std::string stringBuilder = "{";
            std::vector<std::string> collection = val->getKeySet();

            for (const std::string& s: collection) {
                if (stringBuilder.length() != 1) { stringBuilder.append(", "); }
                stringBuilder.append(s);
                stringBuilder.append(": ");
                stringBuilder.append(val->tagMap.at(s).toString());
            }
            stringBuilder.push_back('}');
            return stringBuilder;
        }

        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            std::string stringBuilder = "[I;";

            for (int i = 0; i < val->size; ++i) {
                if (i != 0) { stringBuilder.append(", "); }

                stringBuilder.append(std::to_string(val->array[i]));
            }
            stringBuilder.push_back(']');
            return stringBuilder;
        }

        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            std::string stringBuilder = "[L;";

            for (int i = 0; i < stringBuilder.length(); ++i) {
                if (i != 0) { stringBuilder.append(", "); }

                stringBuilder.append(std::to_string(val->array[i]));
            }
            stringBuilder.append("]");
            return stringBuilder;
        }
        default:
            return "";
    }
}


void NBTBase::read(DataManager& input) {
    switch (type) {
        case NBT_INT8: {
            u8 readData = input.readByte();
            data = malloc(1);
            memcpy(data, &readData, 1);
            return;
        }
        case NBT_INT16: {
            auto readData = (i16) input.readInt16();
            data = malloc(2);
            memcpy(data, &readData, 2);
            return;
        }
        case NBT_INT32: {
            auto readData = (i32) input.readInt32();
            data = malloc(4);
            memcpy(data, &readData, 4);
            return;
        }
        case NBT_INT64: {
            auto readData = (i64) input.readInt64();
            data = malloc(8);
            memcpy(data, &readData, 8);
            return;
        }
        case NBT_FLOAT: {
            float readData = input.readFloat();
            data = malloc(4);
            memcpy(data, &readData, 4);
            return;
        }
        case NBT_DOUBLE: {
            double readData = input.readDouble();
            data = malloc(8);
            memcpy(data, &readData, 8);
            return;
        }
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            auto i = (int) input.readInt32();
            val->array = input.readBytes(i);
            val->size = i;
            return;
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            std::string inputString = input.readUTF();
            int size = (int) inputString.size();
            val->data = (char*) malloc(size);
            memcpy(val->data, inputString.c_str(), size);
            val->size = size;
            return;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            val->tagType = static_cast<NBTType>(input.readByte());
            auto size = (int) input.readInt32();
            if (!size) {
                //this prevents the old NBT style where empty list tags would be of type 1 (byte)
                //and then items other than byte tags cannot be added onto it
                val->tagType = NBTType::NBT_NONE;
            }
            if (val->tagType == NBTType::NBT_NONE && size > 0) {
                printf("Missing type on ListTag");

            } else {
                for (int j = 0; j < size; ++j) {
                    NBTBase* nbtBase = createNewByType(val->tagType);
                    nbtBase->read(input);
                    val->tagList.push_back(*nbtBase);
                    //free the base data (the data and the type), the rest is allocated
                    // and this is copied when pushed back so that is why we free it
                    delete nbtBase;
                }
            }
            return;
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            uint8_t b0;

            while (b0 = input.readByte(), b0 != 0) {
                std::string s = input.readUTF();
                NBTBase* nbtBase = NBT::readNBT((NBTType) b0, s, input);
                val->tagMap[s] = *nbtBase;
                delete nbtBase;
            }
            return;
        }
        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            int i = (int) input.readInt32();
            val->array = (int*) malloc(i * 4);//i * size of int

            for (int j = 0; j < i; ++j) { val->array[j] = (int) input.readInt32(); }
            val->size = i;
            return;
        }
        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            int i = (int) input.readInt32();
            val->array = (i64*) malloc(i * 8);//i * size of long

            for (int j = 0; j < i; ++j) { val->array[j] = (int) input.readInt64(); }
            val->size = i;
            return;
        }
        default:
            return;
    }
}

NBTBase NBTBase::copy() const {
    switch (type) {
        case NBT_INT8: {
            auto* copyData = (uint8_t*) malloc(1);
            memcpy(copyData, data, 1);
            return {copyData, type};
        }
        case NBT_INT16: {
            auto* copyData = (int16_t*) malloc(2);
            memcpy(copyData, data, 2);
            return {copyData, type};
        }
        case NBT_INT32: {
            int* copyData = (int*) malloc(4);
            memcpy(copyData, data, 4);
            return {copyData, type};
        }
        case NBT_INT64: {
            auto* copyData = (i64*) malloc(8);
            memcpy(copyData, data, 8);
            return {copyData, type};
        }
        case NBT_FLOAT: {
            auto* copyData = (float*) malloc(4);
            memcpy(copyData, data, 4);
            return {copyData, type};
        }
        case NBT_DOUBLE: {
            auto* copyData = (double*) malloc(8);
            memcpy(copyData, data, 8);
            return {copyData, type};
        }
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            int size = val->size;
            auto* aByte = (uint8_t*) malloc(size);
            memcpy(aByte, val->array, size);
            return {new NBTTagByteArray(aByte, size), NBTType::TAG_BYTE_ARRAY};
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            NBTBase copied = NBTBase(new NBTTagString(val->getString()), NBTType::TAG_STRING);
            return copied;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            auto* pNbtTagList = new NBTTagList();
            pNbtTagList->tagType = val->tagType;

            for (NBTBase nbtBase: val->tagList) {
                NBTBase nbtBase1 = nbtBase.copy();
                pNbtTagList->tagList.push_back(nbtBase1);
            }

            return {pNbtTagList, NBTType::TAG_LIST};
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            auto* pNbtTagCompound = new NBTTagCompound();
            std::vector<std::string> keySet = val->getKeySet();
            for (const std::string& s: keySet) { pNbtTagCompound->setTag(s, (val->tagMap.at(s)).copy()); }

            return {pNbtTagCompound, NBTType::TAG_COMPOUND};
        }
        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            int size = val->size * 4;
            int* anInt = (int*) malloc(size);//size is in the number of ints
            memcpy(anInt, val->array, size);
            return {new NBTTagIntArray(anInt, val->size), NBTType::TAG_INT_ARRAY};
        }
        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            int size = (int) val->size * 8;//size is in the number of longs
            auto* along = (i64*) malloc(size);
            memcpy(along, val->array, size);
            return {new NBTTagLongArray(along, val->size), NBTType::TAG_LONG_ARRAY};
        }
        case NBT_NONE:
        default:
            return {nullptr, this->type};
    }
}

/// I don't think this is necessary, but if it is then I'll do it.
/// It just is in the java code but never used
MU bool NBTBase::equals(NBTBase check) { return false; }


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
    auto* data = (uint8_t*) malloc(size);//so the original can be safely deleted
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

/*
UUIDJava NBTUtil::getUUIDFromTag(NBTTagCompound* tag) {
    return {tag->getPrimitive<i64>("M"), tag->getPrimitive<i64>("L")};
}


NBTTagCompound* NBTUtil::createUUIDTag(UUIDJava uuid) {
    auto* pNbtTagCompound = new NBTTagCompound();
    pNbtTagCompound->setLong("M", uuid.getMostSignificantBits());
    pNbtTagCompound->setLong("L", uuid.getLeastSignificantBits());
    return pNbtTagCompound;
}


BlockPos NBTUtil::getPosFromTag(NBTTagCompound* tag) {
    auto pos = BlockPos(tag->getPrimitive<int>("X"), tag->getPrimitive<int>("Y"), tag->getPrimitive<int>("Z"));
    return pos;
}


NBTTagCompound* NBTUtil::createPosTag(BlockPos pos) {
    auto* pNbtTagCompound = new NBTTagCompound();
    pNbtTagCompound->setInteger("X", pos.x);
    pNbtTagCompound->setInteger("Y", pos.y);
    pNbtTagCompound->setInteger("Z", pos.z);
    return pNbtTagCompound;
}
*/

void NBT::writeTag(NBTBase* tag, DataManager& output) {
    output.writeByte(tag->getId());

    if (tag->getId() != NBTType::NBT_NONE) {
        output.writeUTF("");
        tag->write(output);
    }
}


NBTBase* NBT::readTag(DataManager& input) {
    NBTBase* returnValue = nullptr;
    int id = input.readByte();
    if (id != 0) {
        std::string key = input.readUTF();
        returnValue = readNBT(static_cast<NBTType>(id), key, input);
    }
    return returnValue;
}


NBTBase* NBT::readNBT(NBTType id, const std::string& key, DataManager& input) {
    NBTBase* pNbtBase = createNewByType(id);
    pNbtBase->read(input);
    return pNbtBase;
}

/*
uint8_t* NBT::writeCompressedTag(NBTBase* rootData, i64* outSize) {
    DataOutputManager output;
    writeTag(rootData, output);
    // compress the data
    uLong size = output.data.size();
    auto* compressedData = (Bytef*) malloc(size);
    uLongf compressedSize = size;
    compress(compressedData, &compressedSize, output.data.data(), size);
    *outSize = compressedSize;
    return compressedData;
}


uint8_t* NBT::writeGZIPCompressedTag(NBTBase* rootData, i64* outSize) {
    DataOutputManager output;
    writeTag(rootData, output);
    int sizeOfUncompressed = (int) output.data.size();
    uLongf sizeOfCompressed = sizeOfUncompressed + 18;
    auto* compressedData = (uint8_t*) malloc(sizeOfCompressed);
    def(output.data.data(), compressedData, sizeOfUncompressed, &sizeOfCompressed, 31);
    *outSize = sizeOfCompressed;
    return compressedData;
}
*/