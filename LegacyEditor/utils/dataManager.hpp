#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/file.hpp"

#include <bit>
#include <string>




/**
 * Starts writing in "Little Endian".
 * This class is NOT responsible for the de-allocation of memory PASSED to it.
 */
class DataManager : public Data {
public:
    bool isBig = true;


    DataManager() = default;


    explicit DataManager(u32 sizeIn): Data(sizeIn) {
        setBigEndian();
    }

    explicit DataManager(Data& dataIn): Data(dataIn.start(), dataIn.getSize()) {
        setBigEndian();
        using_memory = false;
    }

    explicit DataManager(File& fileIn): Data(fileIn.start(), fileIn.getSize()) {
        setBigEndian();
        using_memory = false;
    }

    explicit DataManager(File* fileIn): Data(fileIn->start(), fileIn->getSize()) {
        setBigEndian();
        using_memory = false;
    }

    explicit DataManager(u8* dataIn, u32 sizeIn): Data(dataIn, sizeIn) {
        setBigEndian();
        using_memory = false;
    }


    static bool isSystemLittleEndian() {
        u32 num = 1;
        return (*(u8 *)&num == 1);
    }

    void setBigEndian() {
        isBig = true;
    }

    void setLittleEndian() {
        isBig = false;
    }


    void seekStart();
    void seekEnd();
    void seek(i64 position);
    bool isEndOfData();
    u32 getPosition();

    // READING SECTION

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
    std::string readString(i32 length);

    std::wstring readWString();
    std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    u8* readWithOffset(i32 offset, i32 amount);
    u8* readBytes(u32 length);
    void readOntoData(u32 length, u8* dataIn);
    int readFromFile(const std::string& fileStrIn);


    // WRITING SECTION

    void writeByte(u8 byteIn);
    void writeShort(u16 shortIn);
    void writeInt24(u32 intIn);
    void writeInt(u32 intIn);
    void writeLong(u64 longIn);
    void writeFloat(float floatIn);
    void writeDouble(double doubleIn);

    void writeData(Data* dataIn);
    void writeFile(File* fileIn);
    void writeFile(File& fileIn);
    void write(u8* dataPtrIn, u32 length);

    void writeWString(const std::string& str, u32 length);

    int writeToFile(const std::string& fileName);





};
