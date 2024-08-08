#pragma once

#include <filesystem>
#include <string>

#include "lce/processor.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/data.hpp"


namespace editor {
    class LCEFile;
}

/// Starts writing in "Big Endian".
class DataManager {
public:
    mutable bool isBig = true;
    u8 *data = nullptr, *ptr = nullptr;
    u32 size = 0;

    DataManager() = default;

    explicit DataManager(const Data& dataIn) : data(dataIn.start()), ptr(data), size(dataIn.size) {}
    explicit DataManager(const Data& dataIn, c_bool isBig) : isBig(isBig), data(dataIn.start()), ptr(data), size(dataIn.size) {}

    explicit DataManager(const Data* dataIn) : data(dataIn->start()), ptr(data), size(dataIn->size) {}
    explicit DataManager(const Data* dataIn, c_bool isBig) : isBig(isBig), data(dataIn->start()), ptr(data), size(dataIn->size) {}

    explicit DataManager(u8* dataIn, c_u32 sizeIn) : data(dataIn), ptr(dataIn), size(sizeIn) {}
    explicit DataManager(u8* dataIn, c_u32 sizeIn, c_bool isBig) : isBig(isBig), data(dataIn), ptr(dataIn), size(sizeIn) {}

    void setBigEndian() const { isBig = true; }
    void setLittleEndian() const { isBig = false; }

    ND u8* start() const { return data; }

    void take(const Data& dataIn);
    void take(const Data* dataIn);

    void seekStart();
    MU void seekEnd();
    void seek(i64 position);
    void seek(u32 position);
    bool isEndOfData() const;
    u32 getPosition() const;
    MU u8 peekNextByte() const;
    MU ND u8 peekPreviousByte() const;
    void incrementPointer(u32 amount);
    void decrementPointer(u32 amount);
    MU bool canReadSize(u32 amount) const;

    void incrementPointer1() { ptr += 1; }
    void incrementPointer2() { ptr += 2; }
    void incrementPointer4() { ptr += 4; }
    void incrementPointer8() { ptr += 8; }

    // READING SECTION

    u8 readInt8();
    char readChar();
    u16 readInt16();
    i32 readInt24();
    u32 readInt32();
    u64 readInt64();
    MU bool readBool();
    float readFloat();
    double readDouble();

    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    MU u8 readInt8AtOffset(u32 offset) const;
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    MU u16 readInt16AtOffset(u32 offset) const;
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    MU u32 readInt32AtOffset(u32 offset) const;
    /// reads at offset from .data, not .ptr! Does not increment .ptr.
    MU u64 readInt64AtOffset(u32 offset) const;

    std::string readUTF();
    std::string readString(i32 length);
    std::string readNullTerminatedString();


    std::wstring readNullTerminatedWString();
    std::wstring readNullTerminatedWWWString();
    std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    u8_vec readIntoVector(u32 amount);

    u8* readWithOffset(i32 offset, i32 amount);
    u8* readBytes(u32 length);
    void readBytes(u32 length, u8* dataIn);
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
    MU void writeInt8AtOffset(u32 offset, u8 byteIn) const;
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    MU void writeInt16AtOffset(u32 offset, u16 shortIn) const;
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    MU void writeInt32AtOffset(u32 offset, u32 intIn) const;
    /// writes at offset from .data, not .ptr! Does not increment .ptr.
    MU void writeInt64AtOffset(u32 offset, u64 longIn) const;

    MU void writeData(const Data* dataIn);
    MU void writeFile(const editor::LCEFile* fileIn);
    void writeFile(const editor::LCEFile& fileIn);
    void writeBytes(c_u8* dataPtrIn, u32 length);

    void writeUTF(std::string str);
    /**
     * \brief note that upperbounds is per 2 bytes, so use 64 for 128 bytes.
     * \param wstr
     * \param upperbounds
     */
    void writeWString(const std::wstring& wstr, u32 upperbounds);
    void writeWWWString(const std::wstring& wstr, u32 upperbounds);
    void writeWStringFromString(const std::string& str, u32 upperbounds);

    int writeToFile(const fs::path& inFilePath) const;
    MU int writeToFile(c_u8* ptrIn, uint32_t sizeIn, const fs::path& inFilePath) const;
};
