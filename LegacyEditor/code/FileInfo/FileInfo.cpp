#include "FileInfo.hpp"
#include <filesystem>

#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/error_status.hpp"

#include "include/png/crc.hpp"


static u32 c2n(const char chara) {
    if (chara >= '0' && chara <= '9') { return chara - '0'; }
    if (chara >= 'a' && chara <= 'f') { return chara - 'a' + 10; }
    if (chara >= 'A' && chara <= 'F') { return chara - 'A' + 10; }
    return 0;
}

static i64 stringToHex(const std::string& str) {
    i64 result = 0;
    c_int stringSize = static_cast<int>(str.size());
    for (size_t i = 0; i < stringSize; i++) {
        result = result * 16 + c2n(str[i]);
    }
    return result;
}

static i64 stringToInt64(const std::string& str) {
    i64 result = 0;
    int sign = 1;
    size_t index = 0;

    if (str[0] == '-') {
        sign = -1;
        index++;
    }

    for (c_int stringSize = static_cast<int>(str.size()); index < stringSize; index++) {
        result = result * 10 + (str[index] - '0');
    }

    return result * sign;
}

static char n2c(c_u32 num) {
    if (num <= 9) return static_cast<char>('0' + num);
    return static_cast<char>('a' + (num - 10));
}

static std::string hexToString(i64 hex) {
    if (hex == 0) {
        return "0";
    }

    std::string result;
    while (hex > 0) {
        result.insert(result.begin(), n2c(hex % 16));
        hex /= 16;
    }
    return result;
}


static std::wstring stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}


static std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}


static std::string int64ToString(i64 num) {
    if (num == 0) {
        return "0";
    }

    c_int sign = num < 0 ? -1 : 1;
    num = std::abs(num);

    std::string result;
    while (num > 0) {
        result.insert(result.begin(),
                      static_cast<char>('0' + (num % 10)));
        num /= 10;
    }

    if (sign == -1) {
        result = "-" + result;
    }
    return result;
}


bool isPngHeader(DataManager& manager) {
    static u8_vec PNG_HEADER{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    const u8_vec fileHeader = manager.readIntoVector(8);
    manager.decrementPointer(8);
    return fileHeader == PNG_HEADER;
}


namespace editor {

    /**
     * \brief Gives FileInfo a default ico for creation
     * TODO: write the code
     */
    void FileInfo::defaultThumbnail() {

    }


    /**
     * \brief presumes that the tEXt header is located second to last inside the png.
     * \param inFilePath
     */
    // TODO: idk formatting of header for nintendo consoles
    void FileInfo::readFile(const fs::path& inFilePath, const lce::CONSOLE inConsole) {
        DataManager manager;
        manager.readFromFile(inFilePath.string());

        // TODO: needs to be rewritten for different consoles

        switch (inConsole) {
            case lce::CONSOLE::WIIU: {
                basesavename = manager.readNullTerminatedWString();
                u32 diff = 256 - (u32)(manager.ptr - manager.data);
                manager.incrementPointer(diff);
                break;
            }
            case lce::CONSOLE::SWITCH: {
                basesavename = manager.readNullTerminatedWWWString();
                u32 diff = 512 - (u32)(manager.ptr - manager.data);
                manager.incrementPointer(diff);
                // there is a random u32, then a null u32
                manager.incrementPointer(8);
                break;
            }
            case lce::CONSOLE::VITA:
            default:
                break;

        }

        if (!isPngHeader(manager)) {
            return;
        }

        isLoaded = true;

        c_u8* PNG_START = manager.ptr;

        manager.incrementPointer(8);

        while (!manager.isEndOfData()) {
            c_u8* PNG_END;
            c_u32 chunkLength = manager.readInt32();

            if (std::string chunkType = manager.readString(4);
                    chunkType != "tEXt") {

                // this may not work
                if (chunkType == "IEND") {
                    manager.incrementPointer4();
                    PNG_END = manager.ptr - 8;
                    c_u32 PNG_SIZE = PNG_END - PNG_START;
                    thumbnail.allocate(PNG_SIZE + 8);
                    memcpy(thumbnail.data, PNG_START, PNG_SIZE);
                    return;
                }

                manager.incrementPointer(chunkLength + 4);
                continue;
            }

            PNG_END = manager.ptr - 8;
            {
                c_u32 PNG_SIZE = PNG_END - PNG_START;
                thumbnail.allocate(PNG_SIZE + 12);
                memcpy(thumbnail.data, PNG_START, PNG_SIZE);
                memcpy(thumbnail.data + PNG_SIZE, &IEND_DAT[0], 12);
            }


            // add thumbnail

            size_t length = 0;

            while (true) {
                std::string key;
                std::string text;


                u8 nextChar;
                while ((nextChar = manager.readInt8()) != 0) {
                    key += static_cast<char>(nextChar);
                }
                length += key.size() + 1;

                while ((nextChar = manager.readInt8()) != 0) {
                    text += static_cast<char>(nextChar);
                    length++;
                    if (chunkLength - 1 == length) {
                        break;
                    }
                }

                if (key == "4J_SEED") {
                    seed = stringToInt64(text);
                } else if (key == "4J_HOSTOPTIONS") {
                    hostoptions = stringToHex(text);
                } else if (key == "4J_TEXTUREPACK") {
                    texturepack = stringToHex(text);
                } else if (key == "4J_EXTRADATA") {
                    extradata = stringToHex(text);
                } else if (key == "4J_#LOADS") {
                    loads = stringToInt64(text);
                } else if (key == "4J_EXPLOREDCHUNKS") {
                    exploredchunks = stringToInt64(text);
                } else if (key == "4J_BASESAVENAME") {
                    basesavename = stringToWstring(text);
                    manager.incrementPointer1();
                }

                if (chunkLength - 1 != length) {
                    length++;
                } else {
                    break;
                }
            }
            // break early, no need to read the IEND thingy,
            // since tEXT is always placed at the end
            break;
        }
    }


    int FileInfo::writeFile(const fs::path& outFilePath,
                            const lce::CONSOLE outConsole) const {
        if (thumbnail.data == nullptr) {
            return FILE_ERROR;
        }

        DataManager header;

        switch(outConsole) {
            // TODO: test switch edition writing
            case lce::CONSOLE::SWITCH: {
                const Data fileHeader(528 + 8);
                header.take(fileHeader);
                header.writeWWWString(basesavename, 128);
                // TODO: figure out what this number is
                c_u32 value = 0;
                header.writeInt32(value);
                header.writeInt32(0);
                break;
            }
            case lce::CONSOLE::WIIU: {
                const Data fileHeader(256);
                header.take(fileHeader);
                header.writeWString(basesavename, 128);
                break;
            }
            default:
                break;
        }


        std::vector<u8> tEXt_chunk;
        {
            auto appendString = [&](const std::string& str) {
                tEXt_chunk.insert(tEXt_chunk.end(), str.begin(), str.end());
            };
            auto addNull = [&]() {
                tEXt_chunk.push_back('\0');
            };

            appendString("tEXt");

            appendString("4J_SEED");
            addNull();
            appendString(int64ToString(seed));

            addNull();

            appendString("4J_HOSTOPTIONS");
            addNull();
            appendString(hexToString(hostoptions));

            addNull();

            appendString("4J_TEXTUREPACK");
            addNull();
            appendString(hexToString(texturepack));

            addNull();

            appendString("4J_EXTRADATA");
            addNull();
            appendString(hexToString(extradata));

            addNull();

            appendString("4J_#LOADS");
            addNull();
            appendString(int64ToString(loads));


            if (exploredchunks != 0) {
                addNull();
                appendString("4J_EXPLOREDCHUNKS");
                addNull();
                appendString(int64ToString(exploredchunks));
            }

            // TODO: find the full list, i think it's only used by xbox but idk
            if (outConsole != lce::CONSOLE::WIIU
                && outConsole != lce::CONSOLE::SWITCH
                && outConsole != lce::CONSOLE::VITA) {
                addNull();
                appendString("4J_BASESAVENAME");
                addNull();
                appendString(wstringToString(basesavename));
            }

        }

        c_u32 out_size = header.size + (thumbnail.size - 12) + 4 + tEXt_chunk.size() + 4 + 12;
        const Data out(out_size);
        DataManager manager(out);

        // write header
        if (header.size != 0) {
            memcpy(manager.ptr, header.data, header.size);
            manager.incrementPointer(header.size);
        }

        // write png data (excluding IEND)
        memcpy(manager.ptr, thumbnail.data, thumbnail.size - 12);
        manager.incrementPointer(thumbnail.size - 12);

        // write tEXt chunk size
        manager.writeInt32(tEXt_chunk.size() - 4);

        // write tEXt chunk data
        memcpy(manager.ptr, tEXt_chunk.data(), tEXt_chunk.size());
        manager.incrementPointer(tEXt_chunk.size());

        // write tEXt chunk crc
        c_auto* chunkPtr = reinterpret_cast<const char*>(tEXt_chunk.data());
        c_int sizeIn = static_cast<int>(tEXt_chunk.size());
        c_u32 crc_val = crc(chunkPtr, sizeIn);
        manager.writeInt32(crc_val);

        // write IEND png chunk
        memcpy(manager.ptr, &IEND_DAT[0], 12);

        c_int status = manager.writeToFile(outFilePath);
        return status;

    }
}