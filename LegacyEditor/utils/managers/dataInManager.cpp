#include "dataInManager.hpp"

#include <iostream>
#include <vector>


int DataInManager::saveToFile(const std::string& fileName) {
    FILE* f_out = fopen(fileName.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write to output file '%s'", fileName.c_str());
        return 1;
    }
    fwrite(getStartPtr(), 1, getSize(), f_out);
    fclose(f_out);
    return 0;
}



int DataInManager::readFromFile(const std::string& fileStrIn) {
    FILE* file = fopen(fileStrIn.c_str(), "rb");
    if (file == nullptr) {
        printf("Cannot open infile %s", fileStrIn.c_str());
        return -1;
    }

    fseek(file, 0, SEEK_END);
    u64 newSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    bool status = allocate(newSize);
    if (!status) {
        printf("failed to allocate %llu bytes of memory", newSize);
        fclose(file);
        return -1;
    }

    fread(getStartPtr(), 1, newSize, file);

    fclose(file);
    return 0;
}



void DataInManager::seekStart() {
    ptr = getStartPtr();
}


void DataInManager::seekEnd() {
    seekStart();
    ptr = getStartPtr() + getSize() - 1;
}

void DataInManager::seek(u32 position) {
    ptr = getStartPtr();
    incrementPointer(position);
}


bool DataInManager::isEndOfData() {
    return ptr == getStartPtr() + getSize() - 1;
}


u32 DataInManager::getPosition() {
    return ptr - getStartPtr();
}


u8 DataInManager::readByte() {
    u8 value = ptr[0];
    incrementPointer(1);
    return value;
}


u16 DataInManager::readShort() {
    u16 value;
    if (isLittle) {
        value = ((ptr[1] << 8) | (ptr[0]));
    } else {
        value = ((ptr[0] << 8) | (ptr[1]));
    }
    incrementPointer(2);
    return value;
}


i32 DataInManager::readInt24() {
    uint32_t value = readInt();
    if (isLittle) {
        value = value & 0x00FFFFFF;
    } else {
        value = (value & 0xFFFFFF00) >> 8;
    }
    incrementPointer(-1);
    return (i32) value;

}


i32 DataInManager::readInt24(bool isLittleIn) {
    bool originalEndianType = isLittle;
    isLittle = isLittleIn;
    uint32_t val = readInt();
    if (isLittleIn) {
        val = val & 0x00FFFFFF;
    } else {
        val = (val & 0xFFFFFF00) >> 8;
    }
    incrementPointer(-1); // 3 = 4 - 1
    isLittle = originalEndianType;
    return (int) val;
}


u32 DataInManager::readInt() {
    uint32_t value;
    if (isLittle) {
        value = ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]));
    } else {
        value = ((ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]));
    }
    incrementPointer(4);
    return value;
}


u64 DataInManager::readLong() {
    int64_t val;
    if (isLittle) {
        val = (i64) (
                ((u64) ptr[7] << 56) |
                ((u64) ptr[6] << 48) |
                ((u64) ptr[5] << 40) |
                ((u64) ptr[4] << 32) |
                ((u64) ptr[3] << 24) |
                ((u64) ptr[2] << 16) |
                ((u64) ptr[1] <<  8) |
                ((u64) ptr[0]));
    } else {
        val = (i64) (
                ((u64) ptr[0] << 56) |
                ((u64) ptr[1] << 48) |
                ((u64) ptr[2] << 40) |
                ((u64) ptr[3] << 32) |
                ((u64) ptr[4] << 24) |
                ((u64) ptr[5] << 16) |
                ((u64) ptr[6] <<  8) |
                ((u64) ptr[7]));
    }
    incrementPointer(8);
    return val;
}


bool DataInManager::readBool() {
    return readByte() != 0;
}


std::string DataInManager::readUTF() {
    u8 length = readShort();
    std::string return_string((char*) ptr, length);
    incrementPointer(length);
    return return_string;
}


std::string DataInManager::readString(i32 amount) {
    std::string returnString;
    std::vector<char> strVec;
    strVec.resize(amount + 1);
    char* str = strVec.data();
    str[amount] = 0;

    readOntoData(amount, (uint8_t*) str);
    returnString = std::string(str);
    return returnString;
}


std::wstring DataInManager::readWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = readShort()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}


std::wstring DataInManager::readWString(i32 amount) {
    std::wstring returnString;
    for (int i = 0; i < amount; i++) {
        auto c = static_cast<wchar_t>(this->readShort());
        if (c != 0) { returnString += c; }
    }
    return returnString;
}


std::string DataInManager::readWAsString(size_t length, bool isLittleIn) {
    u8 empty = 0;
    u8 letter1;
    u8 letter2;
    u8* letters = new u8[length + 1];
    letters[length] = 0;

    size_t i;
    for (i = 0; i < length; i++) {
        if (isLittleIn) {
            letter2 = readByte();
            letter1 = readByte();
        } else {
            letter1 = readByte();
            letter2 = readByte();
        }
        letters[i] = letter1;
        if (letter1 == empty) {
            incrementPointer(2 * i64(length - i - 1));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), i);
    delete[] letters;
    return result;
}

/*
 u8 empty = 0;
   u8* emptyPtr = &empty;

for (size_t i = 0; i < length && i < std::min(str.size(), length); ++i) {
   if (isLittle) {
       writeByte(str[i]);
       write(emptyPtr, 1);
   } else {
       write(emptyPtr, 1);
       writeByte(str[i]);
   }
}

// If the given length is greater than the string size, add padding
for (size_t i = str.size(); i < length; ++i) {
   write(emptyPtr, 1);
   write(emptyPtr, 1);
}
 */


float DataInManager::readFloat() {
    u32 val = readInt();
    return *(float*) &val;
}


double DataInManager::readDouble() {
    u64 val = readLong();
    return *(double*) &val;
}


u8* DataInManager::readWithOffset(i32 offset, i32 amount) {
    auto* val = new u8[amount];
    incrementPointer(offset);
    memcpy(val, getStartPtr(), amount);
    incrementPointer(amount);
    return val;
}


u8* DataInManager::readBytes(u32 amount) {
    auto* val = new u8[amount];
    memcpy(val, getStartPtr(), amount);
    incrementPointer((i32) amount);
    return val;
}


void DataInManager::readOntoData(u32 amount, u8* dataIn) {
    memcpy(dataIn, getStartPtr(), amount);
    incrementPointer(amount);
}

