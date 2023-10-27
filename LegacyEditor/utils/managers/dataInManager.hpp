#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/file.hpp"

#include <bit>
#include <string>

// int num = 1;
// bool isSystemLittleEndian = *(char*) &num == 1;


class DataInManager : public Data {
public:
    bool isLittle = false;

    DataInManager() {}
    explicit DataInManager(u32 sizeIn) : Data(sizeIn) {}
    explicit DataInManager(u8* dataPtrIn, u32 sizeIn) : Data(dataPtrIn, sizeIn) {}
    explicit DataInManager(Data& dataIn) : Data(dataIn.start(), dataIn.getSize()) {
        dataIn.using_memory = false;
    }

    inline void setLittleEndian() { isLittle = true; }
    inline void setBigEndian() { isLittle = false; }


    void seekStart();
    void seekEnd();
    void seek(u32 position);
    bool isEndOfData();
    u32 getPosition();

    u8 readByte();
    u16 readShort();
    i32 readInt24();
    i32 readInt24(bool isLittleIn);
    u32 readInt();
    u64 readLong();
    bool readBool();
    float readFloat();
    double readDouble();

    std::string readUTF();
    std::string readString(i32 amount);
    std::wstring readWString();
    std::wstring readWString(i32 amount);
    std::string readWAsString(size_t length, bool isLittleIn);

    u8* readWithOffset(i32 offset, i32 amount);
    u8* readBytes(u32 amount);
    void readOntoData(u32 amount, u8* dataIn);
    int readFromFile(const std::string& fileStrIn);
    int saveToFile(const std::string& fileName);


};
