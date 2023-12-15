#include "dataManager.hpp"

#include "LegacyEditor/utils/file.hpp"


// SEEK

void DataManager::seekStart() {
    ptr = data;
}


void DataManager::seekEnd() {
    ptr = data + size - 1;
}


void DataManager::seek(i64 position) {
    seekStart();
    incrementPointer((i32)position);
}

bool DataManager::isEndOfData() const {
    return ptr == data + size - 1;
}

u32 DataManager::getPosition() const {
    return ptr - data;
}


u8 DataManager::peekNextByte() const {
    return ptr[1];
}

u8 DataManager::peekPreviousByte() const {
    return ptr[-1];
}


void DataManager::incrementPointer(u32 amount) {
    ptr += amount;
}

void DataManager::decrementPointer(u32 amount) {
    ptr -= amount;
}


// READ


u8 DataManager::readInt8() {
    u8 value = ptr[0];
    incrementPointer1();
    return value;
}


u16 DataManager::readInt16() {
    u16 value;
    if (isBig) {
        value = ((ptr[0] << 8) | (ptr[1]));
    } else {
        value = ((ptr[1] << 8) | (ptr[0]));
    }
    incrementPointer2();
    return value;
}


i32 DataManager::readInt24() {
    u32 value = readInt32();
    if (isBig) {
        value = value & 0xFFFFFF00 >> 8;
    } else {
        value = (value & 0x00FFFFFF);
    }
    decrementPointer(1);
    return (i32) value;

}


// TODO: remove this function its bad
i32 DataManager::readInt24(bool isLittleIn) {
    bool originalEndianType = isBig;
    isBig = isLittleIn;
    u32 val = readInt32();
    if (isLittleIn) {
        val = val & 0x00FFFFFF;
    } else {
        val = (val & 0xFFFFFF00) >> 8;
    }
    decrementPointer(1); // 3 = 4 - 1
    isBig = originalEndianType;
    return (int) val;
}


u32 DataManager::readInt32() {
    u32 value;
    if (isBig) {
        value = ((ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]));
    } else {
        value = ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]));
    }
    incrementPointer4();
    return value;
}


u64 DataManager::readInt64() {
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
    incrementPointer8();
    return val;
}


u8 DataManager::readInt8AtOffset(u32 offset) const {
    u8 value = data[offset];
    return value;
}


u16 DataManager::readInt16AtOffset(u32 offset) const {
    u8* ptrOff = data + offset;
    u16 value;
    if (isBig) {
        value = ((ptrOff[0] << 8) | (ptrOff[1]));
    } else {
        value = ((ptrOff[1] << 8) | (ptrOff[0]));
    }
    return value;
}


u32 DataManager::readInt32AtOffset(u32 offset) const {
    u8* ptrOff = data + offset;
    u32 value;
    if (isBig) {
        value = ((ptrOff[0] << 24) | (ptrOff[1] << 16) | (ptrOff[2] << 8) | (ptrOff[3]));
    } else {
        value = ((ptrOff[3] << 24) | (ptrOff[2] << 16) | (ptrOff[1] << 8) | (ptrOff[0]));
    }
    return value;
}


u64 DataManager::readInt64AtOffset(u32 offset) const {
    u8* ptrOff = data + offset;
    i64 val;
    if (isBig) {
        val = (i64) (
                ((u64) ptrOff[0] << 56) |
                ((u64) ptrOff[1] << 48) |
                ((u64) ptrOff[2] << 40) |
                ((u64) ptrOff[3] << 32) |
                ((u64) ptrOff[4] << 24) |
                ((u64) ptrOff[5] << 16) |
                ((u64) ptrOff[6] <<  8) |
                ((u64) ptrOff[7]));
    } else {
        val = (i64) (
                ((u64) ptrOff[7] << 56) |
                ((u64) ptrOff[6] << 48) |
                ((u64) ptrOff[5] << 40) |
                ((u64) ptrOff[4] << 32) |
                ((u64) ptrOff[3] << 24) |
                ((u64) ptrOff[2] << 16) |
                ((u64) ptrOff[1] <<  8) |
                ((u64) ptrOff[0]));
    }
    return val;
}


bool DataManager::readBool() {
    return readInt8() != 0;
}


std::string DataManager::readUTF() {
    u8 length = readInt16();
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
    while ((nextChar = readInt16()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}


std::wstring DataManager::readWString(u32 length) {
    std::wstring returnString;
    for (u32 i = 0; i < length; i++) {
        auto c = static_cast<wchar_t>(this->readInt16());
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
            readInt8();
            letters[i] = readInt8();
        } else {
            letters[i] = readInt8();
            readInt8();
        }
        if (letters[i] == empty) {
            incrementPointer(i32(2 * (length - i - 1)));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), i);
    delete[] letters;
    return result;
}



u8_vec DataManager::readIntoVector(u32 amount) {
    if EXPECT_FALSE(getPosition() + amount > size) {
        return u8_vec(amount, 0);
    }
    u8_vec returnVector(ptr, ptr + amount);
    incrementPointer(amount);
    return returnVector;
}


float DataManager::readFloat() {
    u32 val = readInt32();
    return *(float*) &val;
}


double DataManager::readDouble() {
    u64 val = readInt64();
    return *(double*) &val;
}


u8* DataManager::readWithOffset(i32 offset, i32 amount) {
    u8* val = new u8[amount];
    incrementPointer(offset);
    memcpy(val, data, amount);
    incrementPointer(amount);
    return val;
}


u8* DataManager::readBytes(u32 length) {
    u8* val = new u8[length];
    memcpy(val, ptr, length);
    incrementPointer((i32) length);
    return val;
}


void DataManager::readOntoData(u32 length, u8* dataIn) {
    memcpy(dataIn, ptr, length);
    incrementPointer((i32)length);
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

    data = new u8[newSize];
    ptr = data;
    size = newSize;

    /*
    if (!status) {
        printf("failed to allocate %llu bytes of memory", newSize);
        fclose(file);
        return -1;
    }
     */

    fread(data, 1, newSize, file);

    fclose(file);
    return 0;
}


void DataManager::writeInt8(u8 byteIn) {
    ptr[0] = byteIn;
    incrementPointer1();
}


void DataManager::writeInt16(u16 shortIn) {
    if (isBig) {
        ptr[0] = (shortIn >> 8) & 0xff;
        ptr[1] =  shortIn       & 0xff;
    } else {
        ptr[0] =  shortIn       & 0xff;
        ptr[1] = (shortIn >> 8) & 0xff;
    }
    incrementPointer2();
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



void DataManager::writeInt32(u32 intIn) {
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
    incrementPointer4();
}


void DataManager::writeInt64(u64 longIn) {
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
    incrementPointer8();
}


void DataManager::writeInt8AtOffset(u32 offset, u8 byteIn) {
    u8* ptrOff = data + offset;
    ptrOff[0] = byteIn;
}


void DataManager::writeInt16AtOffset(u32 offset, u16 shortIn) {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (shortIn >> 8) & 0xff;
        ptrOff[1] =  shortIn       & 0xff;
    } else {
        ptrOff[0] =  shortIn       & 0xff;
        ptrOff[1] = (shortIn >> 8) & 0xff;
    }
}


void DataManager::writeInt32AtOffset(u32 offset, u32 intIn) {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (intIn >> 24) & 0xff;
        ptrOff[1] = (intIn >> 16) & 0xff;
        ptrOff[2] = (intIn >> 8)  & 0xff;
        ptrOff[3] =  intIn        & 0xff;
    } else {
        ptrOff[0] =  intIn        & 0xff;
        ptrOff[1] = (intIn >>  8) & 0xff;
        ptrOff[2] = (intIn >> 16) & 0xff;
        ptrOff[3] = (intIn >> 24) & 0xff;
    }
}


void DataManager::writeInt64AtOffset(u32 offset, u64 longIn) {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (longIn >> 56) & 0xff;
        ptrOff[1] = (longIn >> 48) & 0xff;
        ptrOff[2] = (longIn >> 40) & 0xff;
        ptrOff[3] = (longIn >> 32) & 0xff;
        ptrOff[4] = (longIn >> 24) & 0xff;
        ptrOff[5] = (longIn >> 16) & 0xff;
        ptrOff[6] = (longIn >>  8) & 0xff;
        ptrOff[7] =  longIn        & 0xff;
    } else {
        ptrOff[0] =  longIn        & 0xff;
        ptrOff[1] = (longIn >>  8) & 0xff;
        ptrOff[2] = (longIn >> 16) & 0xff;
        ptrOff[3] = (longIn >> 24) & 0xff;
        ptrOff[4] = (longIn >> 32) & 0xff;
        ptrOff[5] = (longIn >> 40) & 0xff;
        ptrOff[6] = (longIn >> 48) & 0xff;
        ptrOff[7] = (longIn >> 56) & 0xff;
    }
}






void DataManager::writeFloat(float floatIn) {
    writeInt32(*(u32*) &floatIn);
}


void DataManager::writeDouble(double doubleIn) {
    writeInt64(*(u64*) &doubleIn);
}


void DataManager::writeBytes(u8* dataPtrIn, u32 length) {
    memcpy(ptr, dataPtrIn, length);
    incrementPointer((i32)length);
}


void DataManager::writeData(Data* dataIn) {
    writeBytes(dataIn->start(), dataIn->size);
}


void DataManager::writeFile(File* fileIn) {
    writeBytes(fileIn->data.start(), fileIn->data.size);
}


void DataManager::writeFile(File& fileIn) {
    writeBytes(fileIn.data.start(), fileIn.data.size);
}


void DataManager::writeUTF(std::string str) {
    u32 str_size = str.size();
    writeInt16(str_size);
    writeBytes((u8*) str.data(), str_size);
}


void DataManager::writeWString(const std::string& str, u32 length) {
    u8 empty = 0;
    u8* emptyPtr = &empty;

    for (u32 i = 0; i < length && i < std::min((u32)str.size(), length); ++i) {
        if (isBig) {
            writeBytes(emptyPtr, 1);
            writeInt8(str[i]);
        } else {
            writeInt8(str[i]);
            writeBytes(emptyPtr, 1);
        }
    }

    // If the given length is greater than the string size, add padding
    for (size_t i = str.size(); i < length; ++i) {
        writeBytes(emptyPtr, 1);
        writeBytes(emptyPtr, 1);
    }
}

int DataManager::writeToFile(const std::string& fileName) const {
    FILE* f_out = fopen(fileName.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to writeBytes to output file '%s'", fileName.c_str());
        return 1;
    }
    fwrite(data, 1, size, f_out);
    fclose(f_out);
    return 0;
}


int DataManager::writeToFile(u8* ptrIn, u32 sizeIn, const std::string& fileName) const {
    FILE* f_out = fopen(fileName.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to writeBytes to output file '%s'", fileName.c_str());
        return 1;
    }
    if (ptrIn < data || ptrIn + sizeIn > data + size) {
        printf("Tried to writeBytes data from out of bounds '%s'", fileName.c_str());
        return 1;
    }

    fwrite(ptrIn, 1, sizeIn, f_out);
    fclose(f_out);
    return 0;
}






















