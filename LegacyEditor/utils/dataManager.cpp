#include "dataManager.hpp"

#include <cstring>

#include "LegacyEditor/code/LCEFile/LCEFile.hpp"


static constexpr u8 FF_MASK = 0xFF;


void DataManager::take(const Data& dataIn) {
    data = dataIn.data;
    size = dataIn.size;
    ptr = data;
}


void DataManager::take(const Data* dataIn) {
    data = dataIn->data;
    size = dataIn->size;
    ptr = data;
}


// SEEK

void DataManager::seekStart() {
    ptr = data;
}


void DataManager::seekEnd() {
    ptr = data + size - 1;
}


void DataManager::seek(c_i64 position) {
    seekStart();
    incrementPointer(static_cast<i32>(position));
}

void DataManager::seek(c_u32 position) {
    seekStart();
    incrementPointer(static_cast<i32>(position));
}

bool DataManager::isEndOfData() const {
    return ptr >= data + size - 1;
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
    c_u8 value = ptr[0];
    incrementPointer1();
    return value;
}


u16 DataManager::readInt16() {
    u16 value;
    if (isBig) {
        value = ptr[0] << 8 | ptr[1];
    } else {
        value = ptr[1] << 8 | ptr[0];
    }
    incrementPointer2();
    return value;
}


i32 DataManager::readInt24() {
    u32 value = readInt32();
    if (isBig) {
        value = value & 0xFFFFFF00 >> 8;
    } else {
        value = value & 0x00FFFFFF;
    }
    decrementPointer(1);
    return static_cast<i32>(value);

}


// TODO: remove this function its bad
i32 DataManager::readInt24(c_bool isLittleIn) {
    int val;
    if (isLittleIn) {
        val = ((ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);
    } else {
        val = ((ptr[0] << 16) | (ptr[1] << 8) | ptr[2]);
    }
    incrementPointer(3);
    return val;
}
/*
i32 DataManager::readInt24(bool isLittleIn) {
    c_bool originalEndianType = isBig;
    isBig = isLittleIn;
    u32 val = readInt32();
    if (isLittleIn) {
        val = val & 0x00FFFFFF;
    } else {
        val = (val & 0xFFFFFF00) >> 8;
    }
    decrementPointer(1);
    isBig = originalEndianType;
    return static_cast<int>(val);
}
*/


u32 DataManager::readInt32() {
    u32 value;
    if (isBig) {
        value = ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
    } else {
        value = ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
    }
    incrementPointer4();
    return value;
}


u64 DataManager::readInt64() {
    i64 val;
    if (isBig) {
        val = static_cast<i64>(static_cast<u64>(ptr[0]) << 56 |
                               static_cast<u64>(ptr[1]) << 48 |
                               static_cast<u64>(ptr[2]) << 40 |
                               static_cast<u64>(ptr[3]) << 32 |
                               static_cast<u64>(ptr[4]) << 24 |
                               static_cast<u64>(ptr[5]) << 16 |
                               static_cast<u64>(ptr[6]) << 8 |
                               static_cast<u64>(ptr[7]));
    } else {
        val = static_cast<i64>(static_cast<u64>(ptr[7]) << 56 |
                               static_cast<u64>(ptr[6]) << 48 |
                               static_cast<u64>(ptr[5]) << 40 |
                               static_cast<u64>(ptr[4]) << 32 |
                               static_cast<u64>(ptr[3]) << 24 |
                               static_cast<u64>(ptr[2]) << 16 |
                               static_cast<u64>(ptr[1]) << 8 |
                               static_cast<u64>(ptr[0]));
    }
    incrementPointer8();
    return val;
}


u8 DataManager::readInt8AtOffset(c_u32 offset) const {
    u8 value = data[offset];
    return value;
}


u16 DataManager::readInt16AtOffset(c_u32 offset) const {
    c_u8* ptrOff = data + offset;
    u16 value;
    if (isBig) {
        value = ptrOff[0] << 8 | ptrOff[1];
    } else {
        value = ptrOff[1] << 8 | ptrOff[0];
    }
    return value;
}


u32 DataManager::readInt32AtOffset(c_u32 offset) const {
    c_u8* ptrOff = data + offset;
    u32 value;
    if (isBig) {
        value = ptrOff[0] << 24 | ptrOff[1] << 16 | ptrOff[2] << 8 | ptrOff[3];
    } else {
        value = ptrOff[3] << 24 | ptrOff[2] << 16 | ptrOff[1] << 8 | ptrOff[0];
    }
    return value;
}


u64 DataManager::readInt64AtOffset(c_u32 offset) const {
    c_u8* ptrOff = data + offset;
    i64 val;
    if (isBig) {
        val = static_cast<i64>(static_cast<u64>(ptrOff[0]) << 56 |
                               static_cast<u64>(ptrOff[1]) << 48 |
                               static_cast<u64>(ptrOff[2]) << 40 |
                               static_cast<u64>(ptrOff[3]) << 32 |
                               static_cast<u64>(ptrOff[4]) << 24 |
                               static_cast<u64>(ptrOff[5]) << 16 |
                               static_cast<u64>(ptrOff[6]) << 8 |
                               static_cast<u64>(ptrOff[7]));
    } else {
        val = static_cast<i64>(static_cast<u64>(ptrOff[7]) << 56 |
                               static_cast<u64>(ptrOff[6]) << 48 |
                               static_cast<u64>(ptrOff[5]) << 40 |
                               static_cast<u64>(ptrOff[4]) << 32 |
                               static_cast<u64>(ptrOff[3]) << 24 |
                               static_cast<u64>(ptrOff[2]) << 16 |
                               static_cast<u64>(ptrOff[1]) << 8 |
                               static_cast<u64>(ptrOff[0]));
    }
    return val;
}


bool DataManager::readBool() {
    return readInt8() != 0;
}


std::string DataManager::readUTF() {
    c_u8 length = readInt16();
    std::string return_string(reinterpret_cast<char*>(ptr), length);
    incrementPointer(length);
    return return_string;
}


std::string DataManager::readNullTerminatedString() {
    std::string returnString;
    u8 nextChar;
    while ((nextChar = readInt8()) != 0) {
        returnString += static_cast<char>(nextChar);
    }
    return returnString;
}


std::string DataManager::readString(c_i32 length) {
    std::vector<char> strVec;
    strVec.resize(length + 1);
    char* str = strVec.data();
    str[length] = 0;

    readOntoData(length, reinterpret_cast<u8*>(str));
    auto returnString = std::string(str);
    return returnString;
}


std::wstring DataManager::readNullTerminatedWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = readInt16()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}


/**
 * Only used by nintendo switch edition...
 * @return
 */
std::wstring DataManager::readNullTerminatedWWWString() {
    std::wstring returnString;
    wchar_t nextChar1;
    wchar_t nextChar2;
    while (true) {
        nextChar1 = readInt16();
        nextChar2 = readInt16();
        if (nextChar1 == 0 && nextChar2 == 0) {
            break;
        }
        returnString += nextChar1;
    }
    return returnString;
}



std::wstring DataManager::readWString(c_u32 length) {
    std::wstring returnString;
    for (u32 i = 0; i < length; i++) {
        if (c_auto c = static_cast<wchar_t>(this->readInt16()); c != 0) {
            returnString += c;
        }
    }
    return returnString;
}


std::string DataManager::readWAsString(c_u32 length) {
    auto *const letters = new u8[length + 1];
    letters[length] = 0;

    u32 iter;
    for (iter = 0; iter < length; iter++) {
        if (isBig) {
            readInt8();
            letters[iter] = readInt8();
        } else {
            letters[iter] = readInt8();
            readInt8();
        }
        if (constexpr u8 empty = 0; letters[iter] == empty) {
            incrementPointer(static_cast<i32>(2 * (length - iter - 1)));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), iter);
    delete[] letters;
    return result;
}



u8_vec DataManager::readIntoVector(c_u32 amount) {
    if EXPECT_FALSE(getPosition() + amount > size) {
        return u8_vec(amount, 0);
    }
    u8_vec returnVector(ptr, ptr + amount);
    incrementPointer(amount);
    return returnVector;
}


float DataManager::readFloat() {
    u32 val = readInt32();
    return *reinterpret_cast<float*>(&val);
}


double DataManager::readDouble() {
    u64 val = readInt64();
    return *reinterpret_cast<double*>(&val);
}


u8* DataManager::readWithOffset(c_i32 offset, c_i32 amount) {
    auto *const val = new u8[amount];
    incrementPointer(offset);
    std::memcpy(val, data, amount);
    incrementPointer(amount);
    return val;
}


u8* DataManager::readBytes(c_u32 length) {
    auto *val = new u8[length];
    std::memcpy(val, ptr, length);
    incrementPointer(static_cast<i32>(length));
    return val;
}


void DataManager::readOntoData(c_u32 length, u8* dataIn) {
    std::memcpy(dataIn, ptr, length);
    incrementPointer(static_cast<i32>(length));
}

int DataManager::readFromFile(const std::string& fileStrIn) {
    FILE* file = fopen(fileStrIn.c_str(), "rb");
    if (file == nullptr) {
        printf("Cannot open infile '%s'", fileStrIn.c_str());
        return -1;
    }

    fseek(file, 0, SEEK_END);
    c_u64 newSize = ftell(file);
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


void DataManager::writeInt8(c_u8 byteIn) {
    ptr[0] = byteIn;
    incrementPointer1();
}


void DataManager::writeInt16(c_u16 shortIn) {
    if (isBig) {
        ptr[0] = (shortIn >> 8) & FF_MASK;
        ptr[1] =  shortIn       & FF_MASK;
    } else {
        ptr[0] =  shortIn       & FF_MASK;
        ptr[1] = (shortIn >> 8) & FF_MASK;
    }
    incrementPointer2();
}



void DataManager::writeInt24(c_u32 intIn) {
    if (isBig) {
        // Write the most significant 3 bytes for big-endian
        ptr[0] = (intIn >> 16) & FF_MASK;
        ptr[1] = (intIn >>  8) & FF_MASK;
        ptr[2] =  intIn        & FF_MASK;
    } else {
        // Write the least significant 3 bytes for little-endian
        ptr[0] =  intIn        & FF_MASK;
        ptr[1] = (intIn >>  8) & FF_MASK;
        ptr[2] = (intIn >> 16) & FF_MASK;
    }
    incrementPointer(3);
}



void DataManager::writeInt32(c_u32 intIn) {
    if (isBig) {
        ptr[0] = (intIn >> 24) & FF_MASK;
        ptr[1] = (intIn >> 16) & FF_MASK;
        ptr[2] = (intIn >> 8)  & FF_MASK;
        ptr[3] =  intIn        & FF_MASK;
    } else {
        ptr[0] =  intIn        & FF_MASK;
        ptr[1] = (intIn >>  8) & FF_MASK;
        ptr[2] = (intIn >> 16) & FF_MASK;
        ptr[3] = (intIn >> 24) & FF_MASK;
    }
    incrementPointer4();
}


auto DataManager::writeInt64(c_u64 longIn) -> void {
    if (isBig) {
        ptr[0] = (longIn >> 56) & FF_MASK;
        ptr[1] = (longIn >> 48) & FF_MASK;
        ptr[2] = (longIn >> 40) & FF_MASK;
        ptr[3] = (longIn >> 32) & FF_MASK;
        ptr[4] = (longIn >> 24) & FF_MASK;
        ptr[5] = (longIn >> 16) & FF_MASK;
        ptr[6] = (longIn >>  8) & FF_MASK;
        ptr[7] =  longIn        & FF_MASK;
    } else {
        ptr[0] =  longIn        & FF_MASK;
        ptr[1] = (longIn >>  8) & FF_MASK;
        ptr[2] = (longIn >> 16) & FF_MASK;
        ptr[3] = (longIn >> 24) & FF_MASK;
        ptr[4] = (longIn >> 32) & FF_MASK;
        ptr[5] = (longIn >> 40) & FF_MASK;
        ptr[6] = (longIn >> 48) & FF_MASK;
        ptr[7] = (longIn >> 56) & FF_MASK;
    }
    incrementPointer8();
}


void DataManager::writeInt8AtOffset(c_u32 offset, c_u8 byteIn) const {
    u8* ptrOff = data + offset;
    ptrOff[0] = byteIn;
}


void DataManager::writeInt16AtOffset(c_u32 offset, c_u16 shortIn) const {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (shortIn >> 8) & FF_MASK;
        ptrOff[1] =  shortIn       & FF_MASK;
    } else {
        ptrOff[0] =  shortIn       & FF_MASK;
        ptrOff[1] = (shortIn >> 8) & FF_MASK;
    }
}


void DataManager::writeInt32AtOffset(c_u32 offset, c_u32 intIn) const {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (intIn >> 24) & FF_MASK;
        ptrOff[1] = (intIn >> 16) & FF_MASK;
        ptrOff[2] = (intIn >> 8)  & FF_MASK;
        ptrOff[3] =  intIn        & FF_MASK;
    } else {
        ptrOff[0] =  intIn        & FF_MASK;
        ptrOff[1] = (intIn >>  8) & FF_MASK;
        ptrOff[2] = (intIn >> 16) & FF_MASK;
        ptrOff[3] = (intIn >> 24) & FF_MASK;
    }
}


void DataManager::writeInt64AtOffset(c_u32 offset, c_u64 longIn) const {
    u8* ptrOff = data + offset;
    if (isBig) {
        ptrOff[0] = (longIn >> 56) & FF_MASK;
        ptrOff[1] = (longIn >> 48) & FF_MASK;
        ptrOff[2] = (longIn >> 40) & FF_MASK;
        ptrOff[3] = (longIn >> 32) & FF_MASK;
        ptrOff[4] = (longIn >> 24) & FF_MASK;
        ptrOff[5] = (longIn >> 16) & FF_MASK;
        ptrOff[6] = (longIn >>  8) & FF_MASK;
        ptrOff[7] =  longIn        & FF_MASK;
    } else {
        ptrOff[0] =  longIn        & FF_MASK;
        ptrOff[1] = (longIn >>  8) & FF_MASK;
        ptrOff[2] = (longIn >> 16) & FF_MASK;
        ptrOff[3] = (longIn >> 24) & FF_MASK;
        ptrOff[4] = (longIn >> 32) & FF_MASK;
        ptrOff[5] = (longIn >> 40) & FF_MASK;
        ptrOff[6] = (longIn >> 48) & FF_MASK;
        ptrOff[7] = (longIn >> 56) & FF_MASK;
    }
}






void DataManager::writeFloat(float floatIn) {
    writeInt32(*reinterpret_cast<u32*>(&floatIn));
}


void DataManager::writeDouble(double doubleIn) {
    writeInt64(*reinterpret_cast<u64*>(&doubleIn));
}


void DataManager::writeBytes(c_u8* dataPtrIn, c_u32 length) {
    std::memcpy(ptr, dataPtrIn, length);
    incrementPointer(static_cast<i32>(length));
}


void DataManager::writeData(const Data* dataIn) {
    writeBytes(dataIn->start(), dataIn->size);
}


void DataManager::writeFile(const editor::LCEFile* fileIn) {
    writeBytes(fileIn->data.start(), fileIn->data.size);
}


void DataManager::writeFile(const editor::LCEFile& fileIn) {
    writeBytes(fileIn.data.start(), fileIn.data.size);
}


void DataManager::writeUTF(std::string str) {
    c_u32 str_size = str.size();
    writeInt16(str_size);
    writeBytes(reinterpret_cast<u8*>(str.data()), str_size);
}


void DataManager::writeWString(const std::wstring& wstr, c_u32 upperbounds) {
    c_u32 wstr_size_min = std::min(static_cast<u32>(wstr.size()), upperbounds);
    for (u32 i = 0; i < upperbounds && i < wstr_size_min; ++i) {
        writeInt16(wstr[i]);
    }
    // hack, write null char if there is space, and fill rest of space with null as well
    if (wstr_size_min < upperbounds) {
        u32 count = upperbounds - wstr_size_min;
        for (u32 i = 0; i < count; i++) {
            writeInt16(0);
        }
    }
}


/**
 * Only used by nintendo switch edition...
 * @param wstr
 * @param upperbounds
 */
void DataManager::writeWWWString(const std::wstring& wstr, c_u32 upperbounds) {
    c_u32 wstr_size_min = std::min(static_cast<u32>(wstr.size()), upperbounds);
    for (u32 i = 0; i < upperbounds && i < wstr_size_min; ++i) {
        writeInt16(wstr[i]);
        writeInt16(0);
    }
    // hack, write null char if there is space
    if (wstr_size_min < upperbounds) {
        writeInt32(0);
    }
}



void DataManager::writeWStringFromString(const std::string& str, c_u32 upperbounds) {
    constexpr u8 empty = 0;
    c_u8* emptyPtr = &empty;

    for (u32 i = 0; i < upperbounds && i < std::min(static_cast<u32>(str.size()), upperbounds); ++i) {
        if (isBig) {
            writeBytes(emptyPtr, 1);
            writeInt8(str[i]);
        } else {
            writeInt8(str[i]);
            writeBytes(emptyPtr, 1);
        }
    }

    // If the given length is greater than the string size, add padding
    for (size_t i = str.size(); i < upperbounds; ++i) {
        writeBytes(emptyPtr, 1);
        writeBytes(emptyPtr, 1);
    }
}

int DataManager::writeToFile(const fs::path& inFilePath) const {
    std::string inFileStr = inFilePath.string();

    FILE *f_out = fopen(inFileStr.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write data to output file \"%s\"\n", inFileStr.c_str());
        return -1;
    }
    fwrite(data, 1, size, f_out);
    fclose(f_out);


    return 0;
}


int DataManager::writeToFile(c_u8* ptrIn, c_u32 sizeIn, const fs::path& inFilePath) const {
    std::string inFileStr = inFilePath.string();

    FILE* f_out = fopen(inFileStr.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write data to output file \"%s\"\n", inFileStr.c_str());
        return 1;
    }
    if (ptrIn < data || ptrIn + sizeIn > data + size) {
        printf("Tried to write data out of bounds \"%s\"\n", inFileStr.c_str());
        return 1;
    }

    fwrite(ptrIn, 1, sizeIn, f_out);
    fclose(f_out);
    return 0;
}






















