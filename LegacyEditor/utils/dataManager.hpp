#pragma once

#include <bit>
#include <string>
#include <memory>

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"


/**
 * Starts writing in "Big Endian".
 */
class DataManager {
public:
    mutable bool isBig = true;
    u8* data = nullptr;
    u8* ptr = nullptr;
    u32 size = 0;

    DataManager() = default;

    explicit DataManager(Data& dataIn) : data(dataIn.start()), size(dataIn.size), ptr(data) {}
    explicit DataManager(Data& dataIn, bool isBig) : data(dataIn.start()), size(dataIn.size), ptr(data), isBig(isBig) {}

    explicit DataManager(Data* dataIn) : data(dataIn->start()), size(dataIn->size), ptr(data) {}
    explicit DataManager(Data* dataIn, bool isBig) : data(dataIn->start()), size(dataIn->size), ptr(data), isBig(isBig) {}

    explicit DataManager(u8* dataIn, u32 sizeIn) : data(dataIn), size(sizeIn), ptr(dataIn) {}
    explicit DataManager(u8* dataIn, u32 sizeIn, bool isBig) : data(dataIn), size(sizeIn), ptr(dataIn), isBig(isBig) {}

    inline void setBigEndian() { isBig = true; }
    inline void setLittleEndian() { isBig = false; }

    ND inline u8* start() const { return data; }


    void seekStart();
    void seekEnd();
    void seek(i64 position);
    bool isEndOfData() const;
    u32 getPosition() const;
    u8 peekNextByte() const;
    ND u8 peekPreviousByte() const;
    void incrementPointer(i32 amount);

    inline void incrementPointer1() { ptr += 1; }
    inline void incrementPointer2() { ptr += 2; }
    inline void incrementPointer4() { ptr += 4; }
    inline void incrementPointer8() { ptr += 8; }

    // READING SECTION

    u8 readInt8();
    u16 readInt16();
    i32 readInt24();
    i32 readInt24(bool isLittleIn);
    u32 readInt32();
    u64 readInt64();
    bool readBool();
    float readFloat();
    double readDouble();

    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    u8 readInt8AtOffset(u32 offset);
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    u16 readInt16AtOffset(u32 offset);
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    u32 readInt32AtOffset(u32 offset);
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    u64 readInt64AtOffset(u32 offset);

    std::string readUTF();
    std::string readString(i32 length);

    std::wstring readWString();
    std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    u8_vec readIntoVector(i32 amount);

    u8* readWithOffset(i32 offset, i32 amount);
    u8* readBytes(u32 length);
    void readOntoData(u32 length, u8* dataIn);
    int readFromFile(const std::string& fileStrIn);


    // WRITING SECTION

    void writeInt8(u8 byteIn);
    void writeInt16(u16 shortIn);
    void writeInt24(u32 intIn);
    void writeInt32(u32 intIn);
    void writeInt64(u64 longIn);
    void writeFloat(float floatIn);
    void writeDouble(double doubleIn);

    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    void writeInt8AtOffset(u32 offset, u8 byteIn);
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    void writeInt16AtOffset(u32 offset, u16 shortIn);
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    void writeInt32AtOffset(u32 offset, u32 intIn);
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    void writeInt64AtOffset(u32 offset, u64 longIn);

    void writeData(Data* dataIn);
    void writeFile(File* fileIn);
    void writeFile(File& fileIn);
    void writeBytes(u8* dataPtrIn, u32 length);

    void writeUTF(std::string str);
    void writeWString(const std::string& str, u32 length);

    int writeToFile(const std::string& fileName) const;
    int writeToFile(u8* ptrIn, u32 sizeIn, const std::string& fileName) const;
};
