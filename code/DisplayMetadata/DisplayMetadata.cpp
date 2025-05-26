#include "DisplayMetadata.hpp"


#include "include/png/crc.hpp"

#include "common/DataWriter.hpp"

#include "common/error_status.hpp"
#include "common/utils.hpp"



static u32 c2n(const char chara) {
    if (chara >= '0' && chara <= '9') { return chara - '0'; }
    if (chara >= 'a' && chara <= 'f') { return chara - 'a' + 10; }
    if (chara >= 'A' && chara <= 'F') { return chara - 'A' + 10; }
    return 0;
}

static i64 stringToHex(const std::string& str) {
    i64 result = 0;
    c_int stringSize = static_cast<int>(str.size());
    for (int i = 0; i < stringSize; i++) {
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

    for (size_t stringSize = str.size(); index < stringSize; index++) {
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


// Function to append std::wstring to std::string
void appendWStringToString(std::string& str, const std::wstring& wstr) {
    std::string convertedStr = wStringToString(wstr);
    str.append(convertedStr);
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



static constexpr std::array<u8, 12> IEND_DAT = {
        0x00, 0x00, 0x00, 0x00, // size = 0
        0x49, 0x45, 0x4E, 0x44, // "IEND"
        0xAE, 0x42, 0x60, 0x82  // crc
};


bool isPngHeader(DataReader& reader) {
    static constexpr std::array<u8, 8> PNG_HEADER{
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    const auto savedPos = reader.tell();

    std::array<u8, 8> buf{};
    if (!reader.canRead(8)) {
        reader.seek(savedPos);
        return false;
    }

    reader.readBytes(8, buf.data());
    reader.seek(savedPos);
    return buf == PNG_HEADER;
}


namespace editor {

    void DisplayMetadata::defaultSettings() {
        seed = 0;
        loads = 0;
        hostOptions = 0;
        texturePack = 0;
        extraData = 0;
        exploredChunks = 0;
        worldName = L"converted by LCEditor";
        isLoaded = true;
    }


    /**
     * \brief loads a new default thumbnail
     */
    void DisplayMetadata::loadFileAsThumbnail(const std::string& inFilePath) {
        Buffer buffer = DataReader::readFile(inFilePath); // takes ownership hopefully
        if (DataReader reader(buffer.span()); isPngHeader(reader)) {
            thumbnail = std::move(buffer);
        }
    }


    /**
     * \brief presumes that the tEXt header is located second to last inside the png.
     */
    bool DisplayMetadata::read(Buffer& buffer, lce::CONSOLE c) {
        isLoaded = false;

        DataReader reader(buffer.span());

        bool status1 = readHeader(reader, c);
        if (!status1) { return false; }

        bool status2 = readPNG(reader, c);
        if (!status2) { return false; }

        return isLoaded;
    }


    bool DisplayMetadata::readHeader(DataReader& reader, lce::CONSOLE c) {
        switch (c) {
            case lce::CONSOLE::WIIU: {
                if (!reader.canRead(256)) return false;
                worldName = reader.readNullTerminatedWString();
                reader.skip(256 - reader.tell());
                break;
            }
            case lce::CONSOLE::SWITCH: {
                if (!reader.canRead(520)) return false;
                reader.setEndian(Endian::Little);
                worldName = reader.readNullTerminatedWWWString();
                reader.skip(512 - reader.tell());
                reader.skip(8); // unknown u32, then a null u32'
                reader.setEndian(Endian::Big);
                break;
            }
            default:
                break;
        }
        return true;
    }


    bool DisplayMetadata::readPNG(DataReader& reader, MU lce::CONSOLE c) {
        if (!isPngHeader(reader)) {
            return false;
        }

        c_u8* pngStart = reader.ptr();
        c_u8* pngEnd;

        reader.skip(8);

        while (!reader.eof()) {
            c_u32 chunkLength = reader.read<u32>();
            std::string chunkType = reader.readString(4);


            if (chunkType == "tEXt") {
                pngEnd = reader.ptr() - 8;
                c_u32 bytesUpTo = pngEnd - pngStart;
                thumbnail.allocate(bytesUpTo + 12);
                std::memcpy(thumbnail.data(), pngStart, bytesUpTo);
                std::memcpy(thumbnail.data() + bytesUpTo, IEND_DAT.data(), 12);

                u32 endOfChunk = reader.tell() + chunkLength;

                while (reader.tell() < endOfChunk) {
                    std::string key;
                    std::string text;

                    char nextChar;
                    while ((nextChar = reader.read<char>()) != 0) {
                        key += nextChar;
                    }

                    while ((nextChar = reader.read<char>()) != 0) {
                        text += nextChar;
                        if (reader.tell() >= endOfChunk) {
                            break;
                        }
                    }

                    if (key == "4J_SEED") {
                        seed = stringToInt64(text);
                    } else if (key == "4J_HOSTOPTIONS") {
                        hostOptions = stringToHex(text);
                    } else if (key == "4J_TEXTUREPACK") {
                        texturePack = stringToHex(text);
                    } else if (key == "4J_EXTRADATA") {
                        extraData = stringToHex(text);
                    } else if (key == "4J_#LOADS") {
                        loads = stringToInt64(text);
                    } else if (key == "4J_EXPLOREDCHUNKS") {
                        exploredChunks = stringToInt64(text);
                    } else if (key == "4J_BASESAVENAME") {
                        appendWStringToString(text, worldName);
                        reader.skip<1>();
                    }
                }
            } else if (chunkType == "IEND") {
                reader.skip<4>();
                pngEnd = reader.ptr() - 8;
                c_u32 PNG_SIZE = pngEnd - pngStart;
                thumbnail.allocate(PNG_SIZE + 8);
                std::memcpy(thumbnail.data(), pngStart, PNG_SIZE);
                break;
            } else {
                reader.skip(chunkLength + 4);
            }
        }

        isLoaded = true;
        return true;
    }


    Buffer DisplayMetadata::write(lce::CONSOLE c) {
        if (thumbnail.empty()) return {};
        
        DataWriter writer;

        writeHeader(writer, c);
        writePNG(writer, c);
        
        return std::move(writer.take());
    }


    void DisplayMetadata::writeHeader(DataWriter& writer, lce::CONSOLE c) const {
        switch (c) {
            case lce::CONSOLE::SWITCH:
                writer = DataWriter(528 + 8);
                writer.writeWWWString(worldName, 128);
                writer.write<u32>(0);
                writer.write<u32>(0);
                break;

            case lce::CONSOLE::WIIU:
                writer = DataWriter(256);
                writer.writeUTF16(worldName, 128);
                break;

            default:
                break;
        }

    }


    void DisplayMetadata::writePNG(DataWriter& writer, lce::CONSOLE c) {
        std::vector<u8> tEXt_chunk;
        
        std::vector<u8> tEXt;
        tEXt.reserve(128);
        auto put  = [&](std::string_view s)
            { tEXt.insert(tEXt.end(), s.begin(), s.end()); };
        auto nul  = [&]{ tEXt.push_back('\0'); };

        put("tEXt");
        put("4J_SEED");        nul(); put(int64ToString(seed));      nul();
        put("4J_HOSTOPTIONS"); nul(); put(hexToString(hostOptions)); nul();
        put("4J_TEXTUREPACK"); nul(); put(hexToString(texturePack)); nul();
        put("4J_EXTRADATA");   nul(); put(hexToString(extraData));   nul();
        put("4J_#LOADS");      nul(); put(int64ToString(loads));


        if (exploredChunks != 0) {
            nul(); put("4J_EXPLOREDCHUNKS"); nul();
            put(int64ToString(exploredChunks));
        }
        if (c != lce::CONSOLE::WIIU &&
            c != lce::CONSOLE::SWITCH &&
            c != lce::CONSOLE::PS4 &&
            c != lce::CONSOLE::VITA &&
            c != lce::CONSOLE::PS3 &&
            c != lce::CONSOLE::RPCS3)
        {
            nul(); put("4J_BASESAVENAME"); nul();
            put(wStringToString(worldName));
        }


        MU const std::size_t outSize =
                writer.tell()           // header size
                + thumbnail.size() - 12 // PNG w/o IEND
                + 4                     // tEXt length field
                + tEXt.size()
                + 4                     // CRC
                + 12;                   // IEND


        writer.writeBytes(thumbnail.data(), thumbnail.size() - 12); // strip IEND

        // write tEXt chunk
        writer.write<u32>(tEXt.size() - 4);
        writer.writeBytes(tEXt.data(), tEXt.size());

        const u32 crcVal = crc(tEXt.data(), tEXt.size());
        writer.write<u32>(crcVal);

        // write IEND png chunk
        writer.writeBytes(IEND_DAT.data(), 12);
    }






    /**
     * It is assumed that the console is PSVita, otherwise this function shouldn't be called.
     * The file is called "CACHE.BIN" and appears in early versions of PSVita.
     * @param inFilePath
     * @return
     */
    int DisplayMetadata::readCacheFile(const fs::path& inFilePath, MU const std::string& folderName) {
        isLoaded = false;


        auto buf = DataReader::readFile(inFilePath);
        DataReader writer(buf.span(), Endian::Little);

        bool foundInfo = false;
        u32 pngOffset = 0;
        u16 filesFound = writer.read<u16>();
        for (u16 _ = 0; _ < filesFound; _++) {
            MU u16 var0 = writer.read<u16>(); // lies in the range ``0+x`` to ``fileFound+x``
            MU u32 var1 = writer.read<u32>(); // probably a CRC
            MU u32 iterImageSize = writer.read<u32>();
            std::string iterFolderName = writer.readString(64);
            std::string iterWorldName = writer.readString(128);

            if (folderName == iterFolderName) {
                foundInfo = true;
            }

            if (!foundInfo) {
                pngOffset += iterImageSize;
            }
        }

        writer.skip(pngOffset);

        writer.setEndian(Endian::Big);
        int status = readPNG(writer, lce::CONSOLE::VITA);
        if (status == 0) {
            isLoaded = true;
        }
        return status;
    }
}