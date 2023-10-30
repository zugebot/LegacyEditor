#include "dataManager.hpp"
#include <vector>

// SEEK

void DataManager::seekStart() {
    ptr = start();
}


void DataManager::seekEnd() {
    ptr = start() + getSize() - 1;
}


void DataManager::seek(i64 position) {
    seekStart();
    incrementPointer(position);
}

bool DataManager::isEndOfData() {
    return ptr == start() + getSize() - 1;
}

u32 DataManager::getPosition() {
    return ptr - start();
}


// READ


u8 DataManager::readByte() {
    u8 value = ptr[0];
    incrementPointer(1);
    return value;
}


u16 DataManager::readShort() {
    u16 value;
    if (isBig) {
        value = ((ptr[0] << 8) | (ptr[1]));
    } else {
        value = ((ptr[1] << 8) | (ptr[0]));
    }
    incrementPointer(2);
    return value;
}


i32 DataManager::readInt24() {
    uint32_t value = readInt();
    if (isBig) {
        value = (value & 0xFFFFFF00) >> 8;
    } else {
        value = value & 0x00FFFFFF;
    }
    incrementPointer(-1);
    return (i32) value;

}


// TODO: remove this function its bad
i32 DataManager::readInt24(bool isLittleIn) {
    bool originalEndianType = isBig;
    isBig = isLittleIn;
    uint32_t val = readInt();
    if (isLittleIn) {
        val = val & 0x00FFFFFF;
    } else {
        val = (val & 0xFFFFFF00) >> 8;
    }
    incrementPointer(-1); // 3 = 4 - 1
    isBig = originalEndianType;
    return (int) val;
}


u32 DataManager::readInt() {
    u32 value;
    if (isBig) {
        value = ((ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]));
    } else {
        value = ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]));
    }
    incrementPointer(4);
    return value;
}


u64 DataManager::readLong() {
    i64 val;
    if (isBig) {
        val = (i64) (
                ((u64) ptr[0] << 56) |
                ((u64) ptr[1] << 48) |
                ((u64) ptr[2] << 40) |
                ((u64) ptr[3] << 32) |
                ((u64) ptr[4] << 24) |
                ((u64) ptr[5] << 16) |
                ((u64) ptr[6] <<  8) |
                ((u64) ptr[7]));
    } else {
        val = (i64) (
                ((u64) ptr[7] << 56) |
                ((u64) ptr[6] << 48) |
                ((u64) ptr[5] << 40) |
                ((u64) ptr[4] << 32) |
                ((u64) ptr[3] << 24) |
                ((u64) ptr[2] << 16) |
                ((u64) ptr[1] <<  8) |
                ((u64) ptr[0]));
    }
    incrementPointer(8);
    return val;
}


bool DataManager::readBool() {
    return readByte() != 0;
}


std::string DataManager::readUTF() {
    u8 length = readShort();
    std::string return_string((char*) ptr, length);
    incrementPointer(length);
    return return_string;
}


std::string DataManager::readString(i32 length) {
    std::string returnString;
    std::vector<char> strVec;
    strVec.resize(length + 1);
    char* str = strVec.data();
    str[length] = 0;

    readOntoData(length, (u8*) str);
    returnString = std::string(str);
    return returnString;
}


std::wstring DataManager::readWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = readShort()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}


std::wstring DataManager::readWString(u32 length) {
    std::wstring returnString;
    for (u32 i = 0; i < length; i++) {
        auto c = static_cast<wchar_t>(this->readShort());
        if (c != 0) { returnString += c; }
    }
    return returnString;
}


std::string DataManager::readWAsString(u32 length) {
    u8 empty = 0;
    u8* letters = new u8[length + 1];
    letters[length] = 0;

    u32 i;
    for (i = 0; i < length; i++) {
        if (isBig) {
            readByte();
            letters[i] = readByte();
        } else {
            letters[i] = readByte();
            readByte();
        }
        if (letters[i] == empty) {
            incrementPointer(2 * i64(length - i - 1));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), i);
    delete[] letters;
    return result;
}



float DataManager::readFloat() {
    u32 val = readInt();
    return *(float*) &val;
}


double DataManager::readDouble() {
    u64 val = readLong();
    return *(double*) &val;
}


u8* DataManager::readWithOffset(i32 offset, i32 amount) {
    u8* val = new u8[amount];
    incrementPointer(offset);
    memcpy(val, start(), amount);
    incrementPointer(amount);
    return val;
}


u8* DataManager::readBytes(u32 length) {
    u8* val = new u8[length];
    memcpy(val, getPtr(), length);
    incrementPointer((i32) length);
    return val;
}


void DataManager::readOntoData(u32 length, u8* dataIn) {
    memcpy(dataIn, start(), length);
    incrementPointer(length);
}

int DataManager::readFromFile(const std::string& fileStrIn) {
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

    fread(start(), 1, newSize, file);

    fclose(file);
    return 0;
}


void DataManager::writeByte(u8 byteIn) {
    ptr[0] = byteIn;
    incrementPointer(1);
}


void DataManager::writeShort(u16 shortIn) {
    if (isBig) {
        ptr[0] = (shortIn >> 8) & 0xff;
        ptr[1] =  shortIn       & 0xff;
    } else {
        ptr[0] =  shortIn       & 0xff;
        ptr[1] = (shortIn >> 8) & 0xff;
    }
    incrementPointer(2);
}



void DataManager::writeInt24(u32 intIn) {
    if (isBig) {
        // Write the most significant 3 bytes for big-endian
        ptr[0] = (intIn >> 16) & 0xff;
        ptr[1] = (intIn >>  8) & 0xff;
        ptr[2] =  intIn        & 0xff;
    } else {
        // Write the least significant 3 bytes for little-endian
        ptr[0] =  intIn        & 0xff;
        ptr[1] = (intIn >>  8) & 0xff;
        ptr[2] = (intIn >> 16) & 0xff;
    }
    incrementPointer(3);
}



void DataManager::writeInt(u32 intIn) {
    if (isBig) {
        ptr[0] = (intIn >> 24) & 0xff;
        ptr[1] = (intIn >> 16) & 0xff;
        ptr[2] = (intIn >> 8)  & 0xff;
        ptr[3] =  intIn        & 0xff;
    } else {
        ptr[0] =  intIn        & 0xff;
        ptr[1] = (intIn >>  8) & 0xff;
        ptr[2] = (intIn >> 16) & 0xff;
        ptr[3] = (intIn >> 24) & 0xff;
    }
    incrementPointer(4);
}


void DataManager::writeLong(u64 longIn) {
    if (isBig) {
        ptr[0] = (longIn >> 56) & 0xff;
        ptr[1] = (longIn >> 48) & 0xff;
        ptr[2] = (longIn >> 40) & 0xff;
        ptr[3] = (longIn >> 32) & 0xff;
        ptr[4] = (longIn >> 24) & 0xff;
        ptr[5] = (longIn >> 16) & 0xff;
        ptr[6] = (longIn >>  8) & 0xff;
        ptr[7] =  longIn        & 0xff;
    } else {
        ptr[0] =  longIn        & 0xff;
        ptr[1] = (longIn >>  8) & 0xff;
        ptr[2] = (longIn >> 16) & 0xff;
        ptr[3] = (longIn >> 24) & 0xff;
        ptr[4] = (longIn >> 32) & 0xff;
        ptr[5] = (longIn >> 40) & 0xff;
        ptr[6] = (longIn >> 48) & 0xff;
        ptr[7] = (longIn >> 56) & 0xff;
    }
    incrementPointer(8);
}


void DataManager::writeFloat(float floatIn) {
    writeInt(*(u32*) &floatIn);
}


void DataManager::writeDouble(double doubleIn) {
    writeLong(*(u64*) &doubleIn);
}


void DataManager::write(u8* dataPtrIn, u32 length) {
    memcpy(ptr, dataPtrIn, length);
    incrementPointer(length);
}


void DataManager::writeData(Data* dataIn) {
    write(dataIn->start(), dataIn->getSize());
}


void DataManager::writeFile(File* fileIn) {
    write(fileIn->start(), fileIn->getSize());
}


void DataManager::writeFile(File& fileIn) {
    write(fileIn.start(), fileIn.getSize());
}


void DataManager::writeWString(const std::string& str, u32 length) {
    u8 empty = 0;
    u8* emptyPtr = &empty;

    for (u32 i = 0; i < length && i < std::min((u32)str.size(), length); ++i) {
        if (isBig) {
            write(emptyPtr, 1);
            writeByte(str[i]);
        } else {
            writeByte(str[i]);
            write(emptyPtr, 1);
        }
    }

    // If the given length is greater than the string size, add padding
    for (size_t i = str.size(); i < length; ++i) {
        write(emptyPtr, 1);
        write(emptyPtr, 1);
    }
}

int DataManager::writeToFile(const std::string& fileName) {
    FILE* f_out = fopen(fileName.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write to output file '%s'", fileName.c_str());
        return 1;
    }
    fwrite(start(), 1, getSize(), f_out);
    fclose(f_out);
    return 0;
}


