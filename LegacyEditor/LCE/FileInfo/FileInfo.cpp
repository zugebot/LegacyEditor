#include "LegacyEditor/libs/png/crc.hpp"
#include "LegacyEditor/LCE/MC/enums.hpp"

#include "FileInfo.hpp"


static u32 c2n(const char chara) {
    if (chara >= '0' && chara <= '9') { return chara - '0'; }
    if (chara >= 'a' && chara <= 'f') { return chara - 'a' + 10; }
    if (chara >= 'A' && chara <= 'F') { return chara - 'A' + 10; }
    return 0;
}

static i64 stringToHex(const std::string& str) {
    i64 result = 0;
    const int stringSize = static_cast<int>(str.size());
    for (size_t i = 0; i < stringSize; i++) { result = result * 16 + c2n(str[i]); }
    return result;
}

static i64 stringToInt64(const std::string& str) {
    i64 result = 0;
    int sign = 1;
    size_t i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    const int stringSize = static_cast<int>(str.size());
    for (; i < stringSize; i++) { result = result * 10 + (str[i] - '0'); }

    return result * sign;
}

static char n2c(const u32 num) {
    if (num <= 9) {
        return static_cast<char>('0' + num);
    }
    if (num >= 10 && num <= 15) {
        return static_cast<char>('a' + (num - 10));
    }
    // Return a default value if num is out of the 0-15 range
    return '0';
}

static std::string hexToString(i64 hex) {
    if (hex == 0) {
        return "0";
    }

    std::string result;
    while (hex > 0) {
        result = n2c(hex % 16) + result;
        hex /= 16;
    }
    return result;
}

static std::string int64ToString(i64 num) {
    if (num == 0) {
        return "0";
    }

    std::string result;
    const int sign = num < 0 ? -1 : 1;
    num = std::abs(num);

    while (num > 0) {
        result = static_cast<char>('0' + num % 10) + result;
        num /= 10;
    }

    if (sign == -1) {
        result = "-" + result;
    }
    return result;
}

bool isPngHeader(DataManager& manager) {
    const u8_vec PNGHeader = manager.readIntoVector(8);
    manager.decrementPointer(8);
    return PNGHeader == u8_vec{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
}


namespace editor {

    /**
     * \brief presumes that the tEXt header is located second to last inside the png.
     * \param inFileStr
     */
    // TODO: idk formatting of header for nintendo consoles
    void FileInfo::readFile(const std::string& inFileStr) {
        DataManager manager;
        manager.readFromFile(inFileStr);

        // needs to be rewritten for different consoles
        if (!isPngHeader(manager)) {
            basesavename = manager.readWString(128);
            if (!isPngHeader(manager)) {
                return;
            }
        }

        isLoaded = true;

        const u8* PNG_START = manager.ptr;

        manager.incrementPointer(8);

        while (!manager.isEndOfData()) {
            const u8* PNG_END = nullptr;
            const u32 chunkLength = manager.readInt32();
            std::string chunkType = manager.readString(4);

            if (chunkType != "tEXt") {

                // this may not work
                if (chunkType == "IEND") {
                    manager.incrementPointer4();
                    PNG_END = manager.ptr - 8;
                    const u32 PNG_SIZE = PNG_END - PNG_START;
                    thumbnail.allocate(PNG_SIZE + 8);
                    memcpy(thumbnail.data, PNG_START, PNG_SIZE);
                    return;
                }

                manager.incrementPointer(chunkLength + 4);
                continue;
            }

            PNG_END = manager.ptr - 8;
            {
                const u32 PNG_SIZE = PNG_END - PNG_START;
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

                while ((nextChar = manager.readInt8()) != 0 && chunkLength != length) {
                    text += static_cast<char>(nextChar);
                    length++;
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
                    loads = stringToHex(text);
                } else if (key == "4J_EXPLOREDCHUNKS") {
                    exploredchunks = stringToHex(text);
                }
                // } else if (key == "4J_BASESAVENAME") {
                    // basesavename = text;

                if (chunkLength != length) {
                    length++;
                } else {
                    manager.decrementPointer(1);
                    break;
                }
            }
        }
    }


    int FileInfo::writeFile(const std::string& outFileStr, const CONSOLE console) const {
        if (thumbnail.data == nullptr) {
            return FILE_ERROR;
        }

        DataManager header;
        if (console == CONSOLE::SWITCH) {
            const Data _(528);
            header.take(_);
        }

        else if (console == CONSOLE::WIIU) {
            const Data _(256);
            header.take(_);
            header.writeWString(basesavename, 128);

        } else {
            // ps3, psvita,
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


            if (loads != 0) {
                addNull();
                appendString("4J_#LOADS");
                addNull();
                appendString(hexToString(loads));
            }

            if (exploredchunks != 0) {
                addNull();
                appendString("4J_EXPLOREDCHUNKS");
                addNull();
                appendString(hexToString(exploredchunks));
            }

            // addNull();
            // null count might not be right
            // appendString("4J_BASESAVENAME");
            // addNull();
            // appendString(basesavename);
        }

        const u32 out_size = header.size + (thumbnail.size - 12) + 4 + tEXt_chunk.size() + 4 + 12;
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
        const auto* chunkPtr = reinterpret_cast<const char*>(tEXt_chunk.data());
        const int sizeIn = static_cast<int>(tEXt_chunk.size());
        const u32 crc_val = crc(chunkPtr, sizeIn);
        manager.writeInt32(crc_val);

        // write IEND png chunk
        memcpy(manager.ptr, &IEND_DAT[0], 12);

        const int status = manager.writeToFile(outFileStr);
        return status;

    }



}