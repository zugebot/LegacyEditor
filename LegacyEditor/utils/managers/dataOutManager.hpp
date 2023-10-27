#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/file.hpp"

#include <bit>
#include <string>





class DataOutManager : public Data {
private:
    int priv_num = 1;
    bool isSystemLittleEndian = *(char*) &priv_num == 1;

public:
    bool isLittle = true;


    DataOutManager() = delete;

    explicit DataOutManager(u8* dataIn, u32 sizeIn): Data(dataIn, sizeIn) {
        setLittleEndian();
    }

    explicit DataOutManager(Data& dataIn): Data(dataIn.start(), dataIn.getSize()) {
        setLittleEndian();
        dataIn.using_memory = false;
    }

    explicit DataOutManager(u32 sizeIn): Data(sizeIn) {
        setLittleEndian();
    }

    void setLittleEndian() {
        isLittle = false;
        if (!isSystemLittleEndian) {
            isLittle = true;
        }
    }
    void setBigEndian() {
        isLittle = true;
        if (!isSystemLittleEndian) {
            isLittle = false;
        }
    }

    int saveToFile(const std::string& fileName);

    void seekStart();
    void seekEnd();
    void seek(i64 position);

    void writeByte(u8 byteIn);
    void writeShort(u16 shortIn);
    void writeInt(u32 intIn);
    void writeLong(u64 longIn);
    void writeFloat(float floatIn);
    void writeDouble(double doubleIn);

    void writeData(Data* dataIn);
    void writeFile(File* fileIn);
    void writeFile(File& fileIn);
    void write(u8* dataPtrIn, u32 amount);

    void writeWString(const std::string& str, size_t length, bool isLittleIn);






};