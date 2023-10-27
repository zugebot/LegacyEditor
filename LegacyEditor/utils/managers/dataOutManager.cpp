#include "dataOutManager.hpp"


int DataOutManager::saveToFile(const std::string& fileName) {
    FILE* f_out = fopen(fileName.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write to output file '%s'", fileName.c_str());
        return 1;
    }
    fwrite(start(), 1, getSize(), f_out);
    fclose(f_out);
    return 0;
}


void DataOutManager::seekStart() {
    ptr = start();
}


void DataOutManager::seekEnd() {
    ptr = start() + getSize() - 1;
}


void DataOutManager::seek(i64 position) {
    seekStart();
    incrementPointer(position);
}


void DataOutManager::writeByte(u8 byteIn) {
    ptr[0] = byteIn;
    incrementPointer(1);
}


void DataOutManager::writeShort(u16 shortIn) {
    if (isLittle) {
        ptr[0] =  shortIn       & 0xff;
        ptr[1] = (shortIn >> 8) & 0xff;
    } else {
        ptr[0] = (shortIn >> 8) & 0xff;
        ptr[1] =  shortIn       & 0xff;
    }
    incrementPointer(2);
}


void DataOutManager::writeInt(u32 intIn) {
    if (isLittle) {
        ptr[0] =  intIn        & 0xff;
        ptr[1] = (intIn >>  8) & 0xff;
        ptr[2] = (intIn >> 16) & 0xff;
        ptr[3] = (intIn >> 24) & 0xff;
    } else {
        ptr[0] = (intIn >> 24) & 0xff;
        ptr[1] = (intIn >> 16) & 0xff;
        ptr[2] = (intIn >> 8)  & 0xff;
        ptr[3] =  intIn        & 0xff;
    }
    incrementPointer(4);
}


void DataOutManager::writeLong(u64 longIn) {
    if (isLittle) {
        ptr[0] =  longIn        & 0xff;
        ptr[1] = (longIn >>  8) & 0xff;
        ptr[2] = (longIn >> 16) & 0xff;
        ptr[3] = (longIn >> 24) & 0xff;
        ptr[4] = (longIn >> 32) & 0xff;
        ptr[5] = (longIn >> 40) & 0xff;
        ptr[6] = (longIn >> 48) & 0xff;
        ptr[7] = (longIn >> 56) & 0xff;
    } else {
        ptr[0] = (longIn >> 56) & 0xff;
        ptr[1] = (longIn >> 48) & 0xff;
        ptr[2] = (longIn >> 40) & 0xff;
        ptr[3] = (longIn >> 32) & 0xff;
        ptr[4] = (longIn >> 24) & 0xff;
        ptr[5] = (longIn >> 16) & 0xff;
        ptr[6] = (longIn >>  8) & 0xff;
        ptr[7] =  longIn        & 0xff;
    }
    incrementPointer(8);
}


void DataOutManager::writeFloat(float floatIn) {
    writeInt(*(u32*) &floatIn);
}


void DataOutManager::writeDouble(double doubleIn) {
    writeLong(*(u64*) &doubleIn);
}


void DataOutManager::write(u8* dataPtrIn, u32 amount) {
    memcpy(ptr, dataPtrIn, amount);
    incrementPointer(amount);
}


void DataOutManager::writeData(Data* dataIn) {
    write(dataIn->start(), dataIn->getSize());
}


void DataOutManager::writeFile(File* fileIn) {
    write(fileIn->start(), fileIn->getSize());
}


void DataOutManager::writeFile(File& fileIn) {
    write(fileIn.start(), fileIn.getSize());
}


void DataOutManager::writeWString(const std::string& str, size_t length, bool isLittleIn) {
    u8 empty = 0;
    u8* emptyPtr = &empty;

    for (size_t i = 0; i < length && i < std::min(str.size(), length); ++i) {
        if (isLittleIn) {
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


