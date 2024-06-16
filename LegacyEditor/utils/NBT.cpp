#include "NBT.hpp"


static constexpr int TO_STRING_MAX_LIST_SIZE = 128;


void NBTBase::write(DataManager& output) const {
    switch (type) {
        case NBT_INT8: {
            u8 writeVal = 0;
            memcpy(&writeVal, data, 1);
            output.writeInt8(writeVal);
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
            c_auto* val = toType<NBTTagByteArray>();
            output.writeInt32(val->size);
            output.writeBytes(val->array, val->size);
            return;
        }
        case TAG_STRING: {
            c_auto* val = toType<NBTTagString>();
            output.writeUTF(val->getString());
            return;
        }
        case TAG_LIST: {
            c_auto* val = toType<NBTTagList>();
            output.writeInt8(val->tagType);
            output.writeInt32(val->tagList.size());
            for (c_auto& item: val->tagList) { item.write(output); }
            return;
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            auto iter = val->tagMap.begin();
            for (int sizeIter = 0; sizeIter < val->tagMap.size(); sizeIter++) {
                NBTTagCompound::writeEntry(iter->first, iter->second, output);
                ++iter;
            }

            output.writeInt8(0);
            return;
        }

        case TAG_INT_ARRAY: {
            c_auto* val = toType<NBTTagIntArray>();
            output.writeInt32(val->size);
            for (int sizeIter = 0; sizeIter < val->size; sizeIter++) {
                output.writeInt32(val->array[sizeIter]);
            }
            return;
        }

        case TAG_LONG_ARRAY: {
            c_auto* val = toType<NBTTagLongArray>();
            output.writeInt32(val->size);

            for (int sizeIter = 0; sizeIter < val->size; sizeIter++) {
                output.writeInt64(val->array[sizeIter]);
            }
        }
        default:;
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
            c_auto* val = toType<NBTTagByteArray>();
            free(val->array);
            delete val;
            return;
        }
        case TAG_STRING: {
            c_auto* val = toType<NBTTagString>();
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
            c_auto* val = toType<NBTTagIntArray>();
            free(val->array);
            delete val;
            return;
        }
        case TAG_LONG_ARRAY: {
            c_auto* val = toType<NBTTagLongArray>();
            free(val->array);
            delete val;
        }
        default:;
    }
}


std::string NBTBase::toString() const {
    switch (type) {
        case NBT_NONE:
            return "END";
        case NBT_INT8: {
            u8 val = 0;
            memcpy(&val, this->data, 1);
            return std::to_string(val) + "b";
        }
        case NBT_INT16: {
            i16 val = 0;
            memcpy(&val, this->data, 2);
            return std::to_string(val) + "s";
        }
        case NBT_INT32: {
            i32 val = 0;
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
            c_auto* val = toType<NBTTagByteArray>();
            std::string stringBuilder = "[B;";

            for (int i = 0; i < std::min(TO_STRING_MAX_LIST_SIZE, val->size); ++i) {
                if (i != 0) { stringBuilder.append(", "); }
                stringBuilder.append(std::to_string(val->array[i]));
            }
            if (val->size > TO_STRING_MAX_LIST_SIZE) {
                stringBuilder.append("...");
            }
            stringBuilder.push_back(']');
            return stringBuilder;
        }

        case TAG_STRING: {
            c_auto* val = toType<NBTTagString>();
            return val->toStringNBT();
        }

        case TAG_LIST: {
            c_auto* val = toType<NBTTagList>();
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

            for (const std::string& str: val->getKeySet()) {
                if (stringBuilder.length() != 1) { stringBuilder.append(", "); }
                stringBuilder.append(str);
                stringBuilder.append(": ");
                stringBuilder.append(val->tagMap.at(str).toString());
            }
            stringBuilder.push_back('}');
            return stringBuilder;
        }

        case TAG_INT_ARRAY: {
            c_auto* val = toType<NBTTagIntArray>();
            std::string stringBuilder = "[I;";

            for (int i = 0; i < std::min(TO_STRING_MAX_LIST_SIZE, val->size); ++i) {
                if (i != 0) { stringBuilder.append(", "); }
                stringBuilder.append(std::to_string(val->array[i]));
            }
            if (val->size > TO_STRING_MAX_LIST_SIZE) {
                stringBuilder.append("...");
            }
            stringBuilder.push_back(']');
            return stringBuilder;
        }

        case TAG_LONG_ARRAY: {
            c_auto* val = toType<NBTTagLongArray>();
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
            c_u8 readData = input.readInt8();
            data = malloc(1);
            memcpy(data, &readData, 1);
            return;
        }
        case NBT_INT16: {
            c_auto readData = static_cast<i16>(input.readInt16());
            data = malloc(2);
            memcpy(data, &readData, 2);
            return;
        }
        case NBT_INT32: {
            c_auto readData = static_cast<i32>(input.readInt32());
            data = malloc(4);
            memcpy(data, &readData, 4);
            return;
        }
        case NBT_INT64: {
            c_auto readData = static_cast<i64>(input.readInt64());
            data = malloc(8);
            memcpy(data, &readData, 8);
            return;
        }
        case NBT_FLOAT: {
            const float readData = input.readFloat();
            data = malloc(4);
            memcpy(data, &readData, 4);
            return;
        }
        case NBT_DOUBLE: {
            const double readData = input.readDouble();
            data = malloc(8);
            memcpy(data, &readData, 8);
            return;
        }
        case TAG_BYTE_ARRAY: {
            auto* val = toType<NBTTagByteArray>();
            c_auto num = static_cast<int>(input.readInt32());
            val->array = input.readBytes(num);
            val->size = num;
            return;
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            const std::string inputString = input.readUTF();
            c_int size = static_cast<int>(inputString.size());
            val->data = static_cast<char*>(malloc(size));
            memcpy(val->data, inputString.c_str(), size);
            val->size = size;
            return;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            val->tagType = static_cast<NBTType>(input.readInt8());
            c_auto size = static_cast<int>(input.readInt32());
            if (size == 0) {
                //this prevents the old NBT style where empty list tags would be of type 1 (byte)
                //and then items other than byte tags cannot be added onto it
                val->tagType = NBT_NONE;
            }
            if (val->tagType == NBT_NONE && size > 0) {
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
            u8 byte;

            while (byte = input.readInt8(), byte != 0) {
                std::string str = input.readUTF();
                const NBTBase* nbtBase = NBT::readNBT(static_cast<NBTType>(byte), str, input);
                val->tagMap[str] = *nbtBase;
                delete nbtBase;
            }
            return;
        }
        case TAG_INT_ARRAY: {
            auto* val = toType<NBTTagIntArray>();
            c_int size = static_cast<int>(input.readInt32());
            val->array = static_cast<int*>(malloc(size * 4)); // i * size of int

            for (int j = 0; j < size; ++j) {
                val->array[j] = static_cast<int>(input.readInt32());
            }
            val->size = size;
            return;
        }
        case TAG_LONG_ARRAY: {
            auto* val = toType<NBTTagLongArray>();
            c_int size = static_cast<int>(input.readInt32());
            val->array = static_cast<i64*>(malloc(size * 8)); // i * size of long

            for (int j = 0; j < size; ++j) {
                val->array[j] = static_cast<int>(input.readInt64());
            }
            val->size = size;
        }
        default:;
    }
}


NBTBase NBTBase::copy() const {
    switch (type) {
        case NBT_INT8: {
            auto* copyData = static_cast<u8*>(malloc(1));
            memcpy(copyData, data, 1);
            return {copyData, type};
        }
        case NBT_INT16: {
            auto* copyData = static_cast<i16*>(malloc(2));
            memcpy(copyData, data, 2);
            return {copyData, type};
        }
        case NBT_INT32: {
            auto* copyData = static_cast<i32*>(malloc(4));
            memcpy(copyData, data, 4);
            return {copyData, type};
        }
        case NBT_INT64: {
            auto* copyData = static_cast<i64*>(malloc(8));
            memcpy(copyData, data, 8);
            return {copyData, type};
        }
        case NBT_FLOAT: {
            auto* copyData = static_cast<float*>(malloc(4));
            memcpy(copyData, data, 4);
            return {copyData, type};
        }
        case NBT_DOUBLE: {
            auto* copyData = static_cast<double*>(malloc(8));
            memcpy(copyData, data, 8);
            return {copyData, type};
        }
        case TAG_BYTE_ARRAY: {
            c_auto* val = toType<NBTTagByteArray>();
            c_int size = val->size;
            auto* aByte = static_cast<u8*>(malloc(size));
            memcpy(aByte, val->array, size);
            return {new NBTTagByteArray(aByte, size), TAG_BYTE_ARRAY};
        }
        case TAG_STRING: {
            c_auto* val = toType<NBTTagString>();
            c_auto copied = NBTBase(new NBTTagString(val->getString()), TAG_STRING);
            return copied;
        }
        case TAG_LIST: {
            c_auto* val = toType<NBTTagList>();
            auto* pNbtTagList = new NBTTagList();
            pNbtTagList->tagType = val->tagType;

            for (NBTBase nbtBase: val->tagList) {
                NBTBase nbtBase1 = nbtBase.copy();
                pNbtTagList->tagList.push_back(nbtBase1);
            }

            return {pNbtTagList, TAG_LIST};
        }
        case TAG_COMPOUND: {
            auto* val = toType<NBTTagCompound>();
            auto* pNbtTagCompound = new NBTTagCompound();
            for (const std::vector<std::string> keySet = val->getKeySet(); const std::string& str: keySet) {
                pNbtTagCompound->setTag(str, val->tagMap.at(str).copy());
            }

            return {pNbtTagCompound, TAG_COMPOUND};
        }
        case TAG_INT_ARRAY: {
            c_auto* val = toType<NBTTagIntArray>();
            c_int size = val->size * 4;
            auto* const anInt = static_cast<int*>(malloc(size)); //size is in the number of ints
            memcpy(anInt, val->array, size);
            return {new NBTTagIntArray(anInt, val->size), TAG_INT_ARRAY};
        }
        case TAG_LONG_ARRAY: {
            c_auto* val = toType<NBTTagLongArray>();
            c_int size = val->size * 8; // size is in the number of longs
            auto* along = static_cast<i64*>(malloc(size));
            memcpy(along, val->array, size);
            return {new NBTTagLongArray(along, val->size), TAG_LONG_ARRAY};
        }
        case NBT_NONE:
        default:
            return {nullptr, this->type};
    }
}

/// I don't think this is necessary, but if it is then I'll do it.
/// It just is in the java code but never used
MU bool NBTBase::equals(MU NBTBase check) { return false; }


void NBTTagCompound::writeEntry(const std::string& name, const NBTBase data, DataManager& output) {
    c_int tagID = data.getId();
    output.writeInt8(tagID);
    if (tagID != 0) {
        output.writeUTF(name);
        data.write(output);
    }
}


std::vector<std::string> NBTTagCompound::getKeySet() {
    std::vector<std::string> keySet;
    auto iter = tagMap.begin();
    c_int size = static_cast<int>(tagMap.size());
    for (int sizeIter = 0; sizeIter < size; sizeIter++) {
        keySet.push_back(iter->first);
        ++iter;
    }
    return keySet;
}


int NBTTagCompound::getSize() const { return static_cast<int>(tagMap.size()); }


void NBTTagCompound::setTag(const std::string& key, const NBTBase value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = value;
}


void NBTTagCompound::setByte(const std::string& key, u8 value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 1, NBT_INT8);
}


void NBTTagCompound::setShort(const std::string& key, short value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 2, NBT_INT16);
}


void NBTTagCompound::setInteger(const std::string& key, int value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 4, NBT_INT32);
}


void NBTTagCompound::setLong(const std::string& key, i64 value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 8, NBT_INT64);
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
    return hasKey(key + "Most", TAG_PRIMITIVE) && hasKey(key + "Least", TAG_PRIMITIVE);
}


void NBTTagCompound::setFloat(const std::string& key, float value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 4, NBT_FLOAT);
}


void NBTTagCompound::setDouble(const std::string& key, double value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(&value, 8, NBT_DOUBLE);
}


void NBTTagCompound::setString(const std::string& key, const std::string& value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(new NBTTagString(value), TAG_STRING);
}

void NBTTagCompound::setByteArray(const std::string& key, c_u8* value, c_int size) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    auto* data = static_cast<u8*>(malloc(size)); // so the original can be safely deleted
    memcpy(data, value, size);
    tagMap[key] = NBTBase(new NBTTagByteArray(data, size), TAG_BYTE_ARRAY);
}


void NBTTagCompound::setIntArray(const std::string& key, c_int* value, c_int size) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    auto* const data = static_cast<int*>(malloc(size * 4)); // so the original can be safely deleted
    memcpy(data, value, size * 4);
    tagMap[key] = NBTBase(new NBTTagIntArray(data, size), TAG_INT_ARRAY);
}


void NBTTagCompound::setLongArray(const std::string& key, const i64* value, c_int size) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    auto* data = static_cast<i64*>(malloc(size * 8)); //so the original can be safely deleted
    memcpy(data, value, size * 8);                    //the endianness is maintained because it is copied raw
    tagMap[key] = NBTBase(new NBTTagLongArray(data, size), TAG_LONG_ARRAY);
}


void NBTTagCompound::setCompoundTag(const std::string& key, NBTTagCompound* compoundTag) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(compoundTag, TAG_COMPOUND);
}


void NBTTagCompound::setListTag(const std::string& key, NBTTagList* listTag) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    tagMap[key] = NBTBase(listTag, TAG_LIST);
}


void NBTTagCompound::setBool(const std::string& key, u8 value) {
    if (tagMap.contains(key)) { tagMap[key].NbtFree(); }
    value = value != 0U ? 1 : 0;
    tagMap[key] = NBTBase(&value, 1, NBT_INT8);
}


NBTBase NBTTagCompound::getTag(const std::string& key) {
    if (c_auto iter = tagMap.find(key); iter != tagMap.end()) { return iter->second; }
    return {};
}


NBTType NBTTagCompound::getTagId(const std::string& key) {
    const NBTBase nbtBase = getTag(key);
    return nbtBase.getId();
}


void NBTTagCompound::deleteAll() {
    for (auto& [fst, snd]: tagMap) { snd.NbtFree(); }
    tagMap.clear();
}


bool NBTTagCompound::hasKey(const std::string& key) const {
    if (tagMap.empty()) {
        return false;
    }
    if (tagMap.contains(key)) {
        return true;
    }
    return false;
}


bool NBTTagCompound::hasKey(const std::string& key, c_int type) {
    if (hasKey(key)) {
        c_int tagID = getTagId(key);
        if (tagID == type) {
            return true;
        }
        if (type != TAG_PRIMITIVE) {
            return false;
        }
        return tagID == 1 || tagID == 2 || tagID == 3 || tagID == 4 || tagID == 5 || tagID == 6;
    }
    return false;
}


bool NBTTagCompound::hasKey(const std::string& key, const NBTType type) {
    if (hasKey(key)) {
        c_int tagID = getTagId(key);
        if (tagID == type) {
            return true;
        }
        if (type != TAG_PRIMITIVE) {
            return false;
        }
        return tagID == 1 || tagID == 2 || tagID == 3 || tagID == 4 || tagID == 5 || tagID == 6;
    }
    return false;
}


std::string NBTTagCompound::getString(const std::string& key) {
    if (hasKey(key, TAG_STRING)) {
        return NBTBase::toType<NBTTagString>(tagMap.at(key))->getString();
    }
    return "";
}


NBTTagByteArray* NBTTagCompound::getByteArray(const std::string& key) {
    if (hasKey(key, TAG_BYTE_ARRAY)) {
        const NBTBase byteArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagByteArray>(byteArrayBase);
    }
    return nullptr;
}


NBTTagIntArray* NBTTagCompound::getIntArray(const std::string& key) {
    if (hasKey(key, TAG_INT_ARRAY)) {
        const NBTBase intArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagIntArray>(intArrayBase);
    }
    return nullptr;
}


NBTTagLongArray* NBTTagCompound::getLongArray(const std::string& key) {
    if (hasKey(key, TAG_LONG_ARRAY)) {
        const NBTBase longArrayBase = tagMap.at(key);
        return NBTBase::toType<NBTTagLongArray>(longArrayBase);
    }
    return nullptr;
}


NBTTagCompound* NBTTagCompound::getCompoundTag(const std::string& key) {
    if (hasKey(key, TAG_COMPOUND)) {
        const NBTBase base = tagMap.at(key);
        return NBTBase::toType<NBTTagCompound>(base);
    }
    return nullptr;
}


NBTTagList* NBTTagCompound::getListTag(const std::string& key) {
    if (hasKey(key, TAG_LIST)) {
        return NBTBase::toType<NBTTagList>(tagMap.at(key));
    }
    return nullptr;
}


bool NBTTagCompound::getBool(const std::string& key) { return getPrimitive<bool>(key); }


void NBTTagCompound::removeTag(const std::string& key) {
    if (!hasKey(key)) { return; }
    tagMap.at(key).NbtFree();
    tagMap.erase(key);
}


bool NBTTagCompound::hasNoTags() const { return tagMap.empty(); }


void NBTTagCompound::merge(NBTTagCompound* other) {
    auto iter = other->tagMap.begin();
    c_int size = static_cast<int>(other->tagMap.size());
    for (int x = 0; x < size; x++) {
        std::string key = iter->first;
        if (NBTBase nbtBase = iter->second; nbtBase.getId() == TAG_COMPOUND) {
            if (hasKey(key, TAG_COMPOUND)) {
                NBTTagCompound* pNbtTagCompound = getCompoundTag(key);
                pNbtTagCompound->merge(NBTBase::toType<NBTTagCompound>(nbtBase));
            } else {
                setTag(key, nbtBase.copy());
            }
        } else {
            setTag(key, nbtBase.copy());
        }
        ++iter;
    }
}


void NBTTagList::appendTag(const NBTBase nbt) {
    if (tagType == NBT_NONE) {
        tagType = nbt.getId();
    } else if (tagType != nbt.getId()) {
        nbt.NbtFree();
        return;
    }
    tagList.push_back(nbt);
}


void NBTTagList::set(c_int index, const NBTBase nbt) {
    if (index >= 0 && index < tagList.size()) {
        tagList[index].NbtFree();
        tagList[index] = nbt;
    } else {
        nbt.NbtFree();
    }
}


void NBTTagList::insert(c_int index, const NBTBase nbt) {
    if (index >= 0 && index < tagList.size()) {
        tagList.insert(tagList.begin() + index, nbt);
    } else {
        nbt.NbtFree();
    }
}


void NBTTagList::removeTag(c_int index) {
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


NBTBase NBTTagList::get(c_int index) const {
    return (index >= 0 && index < tagList.size() ? tagList[index] : NBTBase(nullptr, NBT_NONE));
}


NBTTagByteArray* NBTTagList::getByteArrayAt(c_int index) const {
    if (tagType == TAG_BYTE_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagByteArray>(nbtBase);
        }
    }
    return nullptr;
}


std::string NBTTagList::getStringTagAt(c_int index) const {
    if (tagType == TAG_STRING) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagString>(nbtBase)->getString();
        }
    }
    return "";
}


NBTTagList* NBTTagList::getListTagAt(c_int index) const {
    if (tagType == TAG_LIST) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagList>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagCompound* NBTTagList::getCompoundTagAt(c_int index) const {
    if (tagType == TAG_COMPOUND) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagCompound>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagIntArray* NBTTagList::getIntArrayAt(c_int index) const {
    if (tagType == TAG_INT_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagIntArray>(nbtBase);
        }
    }
    return nullptr;
}


NBTTagLongArray* NBTTagList::getLongArrayAt(c_int index) const {
    if (tagType == TAG_LONG_ARRAY) {
        if (index >= 0 && index < tagList.size()) {
            const NBTBase nbtBase = tagList.at(index);
            return NBTBase::toType<NBTTagLongArray>(nbtBase);
        }
    }
    return nullptr;
}


int NBTTagList::tagCount() const {
    return static_cast<int>(tagList.size());
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


void NBT::writeTag(const NBTBase* tag, DataManager& output) {
    output.writeInt8(tag->getId());

    if (tag->getId() != NBT_NONE) {
        output.writeUTF("");
        tag->write(output);
    }
}


NBTBase* NBT::readTag(DataManager& input) {
    NBTBase* returnValue = nullptr;
    if (int id = input.readInt8(); id != 0) {
        const std::string key = input.readUTF();
        returnValue = readNBT(static_cast<NBTType>(id), key, input);
    }
    return returnValue;
}


NBTBase* NBT::readNBT(const NBTType tagID, const std::string& key, DataManager& input) {
    NBTBase* pNbtBase = createNewByType(tagID);
    pNbtBase->read(input);
    return pNbtBase;
}


/*
u8* NBT::writeCompressedTag(NBTBase* rootData, i64* outSize) {
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


u8* NBT::writeGZIPCompressedTag(NBTBase* rootData, i64* outSize) {
    DataOutputManager output;
    writeTag(rootData, output);
    int sizeOfUncompressed = (int) output.data.size();
    uLongf sizeOfCompressed = sizeOfUncompressed + 18;
    auto* compressedData = (u8*) malloc(sizeOfCompressed);
    def(output.data.data(), compressedData, sizeOfUncompressed, &sizeOfCompressed, 31);
    *outSize = sizeOfCompressed;
    return compressedData;
}
*/
