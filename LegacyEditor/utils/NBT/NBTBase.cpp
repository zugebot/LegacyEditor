#include "NBTBase.hpp"

#include "LegacyEditor/utils/dataManager.hpp"
#include "NBTTagCompound.hpp"
#include "NBT.hpp"


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
            auto* val = toType<NBTTagByteArray>();
            output.writeInt32(val->size);
            output.writeBytes(val->array, val->size);
            return;
        }
        case TAG_STRING: {
            auto* val = toType<NBTTagString>();
            output.writeUTF(val->getString());
            return;
        }
        case TAG_LIST: {
            auto* val = toType<NBTTagList>();
            output.writeInt8(static_cast<int>(val->tagType));
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

            output.writeInt8(0);
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
            u8 readData = input.readInt8();
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
            val->tagType = static_cast<NBTType>(input.readInt8());
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

            while (b0 = input.readInt8(), b0 != 0) {
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


NBTBase convertType(NBTBase baseData, NBTType toType) {
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


NBTBase* createNewByType(NBTType id) {
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
