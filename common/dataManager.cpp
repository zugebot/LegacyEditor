#include "dataManager.hpp"

#include <cstring>

#include "code/LCEFile/LCEFile.hpp"


static constexpr u8 FF_MASK = 0xFF;






void DataManager::take(const Data& dataIn) {
    m_data = dataIn.data;
    m_size = dataIn.size;
    m_ptr = m_data;
}


void DataManager::take(const Data* dataIn) {
    m_data = dataIn->data;
    m_size = dataIn->size;
    m_ptr = m_data;
}






// READ


i32 DataManager::readInt24() {
    int val;
    if (m_order == Endian::Native) {
        val = ((m_ptr[2] << 16) | (m_ptr[1] << 8) | m_ptr[0]);
    } else {
        val = ((m_ptr[0] << 16) | (m_ptr[1] << 8) | m_ptr[2]);
    }
    skip<3>();
    return val;
}



MU void DataManager::writeInt24(c_u32 intIn) {
    if (m_order == Endian::Native) {
        // Write the least significant 3 bytes for little-endian
        m_ptr[0] = intIn & FF_MASK;
        m_ptr[1] = (intIn >> 8) & FF_MASK;
        m_ptr[2] = (intIn >> 16) & FF_MASK;
    } else {
        // Write the most significant 3 bytes for big-endian
        m_ptr[0] = (intIn >> 16) & FF_MASK;
        m_ptr[1] = (intIn >> 8) & FF_MASK;
        m_ptr[2] = intIn & FF_MASK;
    }
    skip<3>();
}



std::string DataManager::readString(c_u32 length) {
    const char* start = reinterpret_cast<const char*>(m_ptr);
    // Look for a NUL in the next `length` bytes (nullptr if none found).
    const char* nul = static_cast<const char*>(std::memchr(start, 0, length));
    const char* end = nul ? nul : start + length;
    std::string str{start, end};
    skip(length);
    return str;
}



MU std::wstring DataManager::readWString(c_u32 length) {
    std::wstring returnString;
    for (u32 i = 0; i < length; i++) {
        if (c_auto c = static_cast<wchar_t>(this->read<u16>()); c != 0) {
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
        if (m_order == Endian::Native) {
            letters[iter] = read<u8>();
            skip<1>();
        } else {
            skip<1>();
            letters[iter] = read<u8>();
        }
        if (constexpr u8 empty = 0; letters[iter] == empty) {
            skip(2 * (length - iter - 1));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), iter);
    delete[] letters;
    return result;
}


/*
std::string DataManager::readUTF() {
    c_u8 length = read<u16>();
    std::string return_string(reinterpret_cast<char*>(ptr), length);
    skip(length);
    return return_string;
}
*/

MU std::string DataManager::readNullTerminatedString() {
    std::string returnString;
    u8 nextChar;
    while ((nextChar = read<u8>()) != 0) {
        returnString += static_cast<char>(nextChar);
    }
    return returnString;
}



std::wstring DataManager::readNullTerminatedWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = read<u16>()) != 0) {
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
        nextChar1 = read<u16>();
        nextChar2 = read<u16>();
        if (nextChar1 == 0 && nextChar2 == 0) {
            break;
        }
        returnString += nextChar1;
    }
    return returnString;
}











u8_vec DataManager::readIntoVector(c_u32 amount) {
    if EXPECT_FALSE(tell() + amount > m_size) {
        return u8_vec(amount, 0);
    }
    u8_vec returnVector(m_ptr, m_ptr + amount);
    skip(amount);
    return returnVector;
}


u8* DataManager::readBytes(c_u32 length) {
    auto *val = new u8[length];
    std::memcpy(val, m_ptr, length);
    skip(length);
    return val;
}


void DataManager::readBytes(c_u32 length, u8* dataIn) {
    std::memcpy(dataIn, m_ptr, length);
    skip(length);
}









void DataManager::writeBytes(c_u8* dataPtrIn, c_u32 length) {
    std::memcpy(m_ptr, dataPtrIn, length);
    skip(length);
}


void DataManager::writeString(std::string str) {
    c_u32 str_size = str.size();
    write<u16>(str_size);
    writeBytes(reinterpret_cast<u8*>(str.data()), str_size);
}


void DataManager::writeWString(const std::wstring& wstr, c_u32 upperbounds) {
    c_u32 wstr_size_min = std::min(static_cast<u32>(wstr.size()), upperbounds);
    for (u32 i = 0; i < upperbounds && i < wstr_size_min; ++i) {
        write<u16>(wstr[i]);
    }
    // hack, write null char if there is space, and fill rest of space with null as well
    if (wstr_size_min < upperbounds) {
        u32 count = upperbounds - wstr_size_min;
        for (u32 i = 0; i < count; i++) {
            write<u16>(0);
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
        write<u16>(wstr[i]);
        write<u16>(0);
    }
    // hack, write null char if there is space
    if (wstr_size_min < upperbounds) {
        write<u32>(0);
    }
}



void DataManager::writeWStringFromString(const std::string& str, c_u32 upperbounds) {
    constexpr u8 empty = 0;
    c_u8* emptyPtr = &empty;

    for (u32 i = 0;
         i < upperbounds &&
         i < std::min(static_cast<u32>(str.size()), upperbounds);
         ++i) {
        if (m_order == Endian::Native) {
            write<u8>(str[i]);
            writeBytes(emptyPtr, 1);
        } else {
            writeBytes(emptyPtr, 1);
            write<u8>(str[i]);
        }
    }

    // If the given length is greater than the string size, add padding
    for (size_t i = str.size(); i < upperbounds; ++i) {
        writeBytes(emptyPtr, 1);
        writeBytes(emptyPtr, 1);
    }
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

    m_data = new u8[newSize];
    m_ptr = m_data;
    m_size = newSize;

    /*
    if (!status) {
        printf("failed to allocate %llu bytes of memory", newSize);
        fclose(file);
        return -1;
    }
     */

    fread(m_data, 1, newSize, file);

    fclose(file);
    return 0;
}


int DataManager::writeToFile(const fs::path& inFilePath) const {
    std::string inFileStr = inFilePath.string();

    FILE *f_out = fopen(inFileStr.c_str(), "wb");
    if (f_out == nullptr) {
        printf("Failed to write data to output file \"%s\"\n", inFileStr.c_str());
        return -1;
    }
    fwrite(m_data, 1, m_size, f_out);
    fclose(f_out);


    return 0;
}



