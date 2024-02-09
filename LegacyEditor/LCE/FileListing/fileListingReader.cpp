#include "fileListing.hpp"

#include <cstdio>
#include <filesystem>

#include "headerUnion.hpp"

#include "LegacyEditor/utils/RLE/rle_vita.hpp"
#include <LegacyEditor/utils/RLE/rle_nsxps4.hpp>
#include "LegacyEditor/libs/tinf/tinf.h"
#include "LegacyEditor/libs/zlib-1.2.12/zlib.h"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/NBT.hpp"


static constexpr u32 CON_MAGIC = 0x434F4E20;
static constexpr u32 ZLIB_MAGIC = 0x789C;
MU static constexpr char error1[69]
    = "Could not allocate %d bytes of data for source file buffer, exiting\n";
MU static constexpr char error2[81]
    = "Could not allocate %d bytes of data for source and decompressed buffer, exiting\n";
static constexpr char error3[43]
    = "Not a Minecraft console savefile, exiting\n";


namespace editor {
    namespace fs = std::filesystem;

    i16 extractMapNumber(stringRef_t str) {
        static const std::string start = "map_";
        static const std::string end = ".dat";
        size_t startPos = str.find(start);
        const size_t endPos = str.find(end);

        if (startPos != std::string::npos && endPos != std::string::npos) {
            startPos += start.length();

            const std::string numberStr = str.substr(startPos, endPos - startPos);
            return static_cast<i16>(std::stoi(numberStr));
        }
        return 32767;
    }


    std::pair<int, int> extractRegionCoords(stringRef_t filename) {
        const size_t lastDot = filename.find_last_of('.');
        const std::string relevantPart = filename.substr(0, lastDot);

        std::istringstream iss(relevantPart);
        std::string part;
        std::vector<std::string> parts;

        while (std::getline(iss, part, '.')) {
            parts.push_back(part);
        }

        int num1 = std::stoi(parts[parts.size() - 2]);
        int num2 = std::stoi(parts[parts.size() - 1]);
        return {num1, num2};
    }


    void FileListing::readData(const Data &dataIn) {
        DataManager managerIn(dataIn, consoleIsBigEndian(console));

        const u32 indexOffset = managerIn.readInt32();
        const u32 fileCount = managerIn.readInt32();
        oldestVersion = managerIn.readInt16();
        currentVersion = managerIn.readInt16();

        allFiles.clear();
        // allFiles.reserve(fileCount);

        u32 total_size = 0;
        u32 non_empty_file_count = 0;
        for (int fileIndex = 0; fileIndex < fileCount; fileIndex++) {
            managerIn.seek(indexOffset + fileIndex * FILE_HEADER_SIZE);

            std::string fileName = managerIn.readWAsString(WSTRING_SIZE);

            u32 fileSize = managerIn.readInt32();
            total_size += fileSize;

            const u32 index = managerIn.readInt32();
            u64 timestamp = managerIn.readInt64();

            if (fileSize == 0U) {
                printf("Skipping empty file \"%s\"\n", fileName.c_str());
                continue;
            }

            managerIn.seek(index);
            u8* data = managerIn.readBytes(fileSize);

            allFiles.emplace_back(data, fileSize, timestamp);
            File &file = allFiles.back();

            // region file
            non_empty_file_count++;
            if (fileName.ends_with(".mcr")) {
                if (fileName.starts_with("DIM-1")) {
                    file.fileType = FileType::REGION_NETHER;
                } else if (fileName.starts_with("DIM1")) {
                    file.fileType = FileType::REGION_END;
                } else if (fileName.starts_with("r")) {
                    file.fileType = FileType::REGION_OVERWORLD;
                }
                auto* nbt = file.createNBTTagCompound();
                const auto [fst, snd]
                    = extractRegionCoords(fileName);
                nbt->setTag("x", createNBT_INT16(static_cast<i16>(fst)));
                nbt->setTag("z", createNBT_INT16(static_cast<i16>(snd)));
                continue;
            }

            if (fileName == "entities.dat") {
                file.fileType = FileType::ENTITY_OVERWORLD;
                continue;
            }
            if (fileName.ends_with("entities.dat")) {
                if (fileName.starts_with("DIM-1")) {
                    file.fileType = FileType::ENTITY_NETHER;
                } else if (fileName.starts_with("DIM1/")) {
                    file.fileType = FileType::ENTITY_END;
                }
                continue;
            }

            if (fileName == "level.dat") {
                file.fileType = FileType::LEVEL;
                continue;
            }

            if (fileName.starts_with("data/map_")) {
                file.fileType = FileType::MAP;
                auto* nbt = file.createNBTTagCompound();
                const i16 mapNumber = extractMapNumber(fileName);
                nbt->setTag("#", createNBT_INT16(mapNumber));
                continue;
            }

            if (fileName == "data/villages.dat") {
                file.fileType = FileType::VILLAGE;
                continue;
            }

            if (fileName == "data/largeMapDataMappings.dat") {
                file.fileType = FileType::DATA_MAPPING;
                continue;
            }

            if (fileName.starts_with("data/")) {
                file.fileType = FileType::STRUCTURE;
                auto* nbt = file.createNBTTagCompound();
                nbt->setString("filename", fileName);
                if (fileName.starts_with("data/villages_")
                    && (console == CONSOLE::SWITCH)) {
                    console = CONSOLE::PS4;
                }
                continue;
            }

            if (fileName.ends_with(".grf")) {
                file.fileType = FileType::GRF;
                continue;
            }

            if (fileName.starts_with("players/") || fileName.find('/') == -1) {
                file.fileType = FileType::PLAYER;
                auto* nbt = file.createNBTTagCompound();
                nbt->setString("filename", fileName);
                continue;
            }

            printf("Unknown File: %s\n", fileName.c_str());

        }

        updatePointers();
        printf("\n");
    }


    int FileListing::read(stringRef_t inFileStr) {

        const int status = readFile(inFileStr);

        // create file path / root
        std::string filepath = filename;
        while (!(filepath.back() == '\\' || filepath.back() == '/')) {
            filepath.pop_back();
        }

        MU const int status2 = readFileInfo(filepath);

        return status;
    }


    int FileListing::readFile(stringRef_t inFilePath) {
        Data data;

        const char* inFileCStr = inFilePath.c_str();
        FILE *f_in = fopen(inFileCStr, "rb");
        if (f_in == nullptr) {
            printf("Cannot open infile %s\n\n", inFileCStr);
            return FILE_ERROR;
        }
        filename = inFilePath;

        fseek(f_in, 0, SEEK_END);
        const u64 source_bin_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);

        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, UNION_HEADER_SIZE, f_in);

        int result;

        if (headerUnion.getInt1() <= 2) {
            u32 file_size = headerUnion.getDestSize();
            /// if (int1 == 0) it is a WiiU savefile unless it's a massive file
            if (headerUnion.getZlibMagic() == ZLIB_MAGIC) {
                if (headerUnion.getSwitchFileSize() < file_size) {
                    file_size = headerUnion.getSwitchFileSize();
                    result = readNSXorPS4(f_in, data, source_bin_size, file_size);
                } else {
                    result = readWiiU(f_in, data, source_bin_size, file_size);
                }
            // TODO: change this to write custom checker for FILE_COUNT * 144 == diff. with
            // TODO: with custom vitaRLE decompress checker
            } else {
                const u32 indexFromSaveFile
                    = headerUnion.getVitaFileSize() - headerUnion.getVitaFileListing();
                if (indexFromSaveFile > 0 && indexFromSaveFile < 65536) {
                    file_size = headerUnion.getVitaFileSize();
                    result = readVita(f_in, data, source_bin_size, file_size);
               } else {
                   result = readPs3(f_in, data, source_bin_size, file_size);
               }
            }
        } else if (headerUnion.getInt2() <= 2) {
            /// if (int2 == 0) it is an xbox savefile unless it's a massive
            /// file, but there won't be 2 files in a savegame file for PS3
            const u32 file_size = headerUnion.getDATFileSize();
            const u32 src_size = headerUnion.getDATSrcSize();
            result = readXbox360DAT(f_in, data, file_size, src_size);
        } else if (headerUnion.getInt2() < 100) {
            /// otherwise if (int2) > 50 then it is a random file
            /// because likely ps3 won't have more than 50 files
            result = readRpcs3(f_in, data, source_bin_size);
        } else if (headerUnion.getInt1() == CON_MAGIC) {
            result = readXbox360BIN(f_in, data, source_bin_size);
        } else {
            printf("%s", error3);
            result = INVALID_SAVE;
        }

        fclose(f_in);
        if (result == SUCCESS) {
            readData(data);

            // sets corresponding state booleans
            switch (console) {
                case CONSOLE::SWITCH:
                case CONSOLE::PS4:
                    separateRegions = true;
                    separateEntities = true;
                    break;
                default:
                    break;

            }
        }
        return result;
    }


    int FileListing::readFileInfo(stringRef_t inFilePath) {
        std::string filepath = inFilePath;

        switch (console) {
            case CONSOLE::XBOX360:
                // IDK...
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
            case CONSOLE::PS4:
                filepath += "THUMB";
                break;
            case CONSOLE::VITA:
                filepath += "THUMBDATA.BIN";
                break;
            case CONSOLE::WIIU:
            case CONSOLE::SWITCH: {
                const u32 num = filename.size() - filepath.size();
                const std::string filenamedat
                    = filename.substr(filename.size(), num);
                filepath += filenamedat;
                filepath += ".ext";
                break;
            }
            case CONSOLE::XBOX1:
                return NOT_IMPLEMENTED;
            case CONSOLE::NONE:
            default:
                return INVALID_CONSOLE;
        }

        if (fs::exists(filepath)) {
            fileInfo.readFile(filepath);
        } else {
            printf("FileInfo file not found...\n");
        }

        return SUCCESS;
    }


    /**
     * \brief
     * \param inFilePath the directory containing the GAMEDATA files.
     * \return
     */
    int FileListing::readExternalRegions(stringRef_t inFilePath) {
        int fileIndex = -1;
        for (const auto& file :
            fs::directory_iterator(inFilePath)) {

            // TODO: place non-used files in a cache?
            if (is_directory(file)) { continue; }
            fileIndex++;

            // initiate filename and filepath
            std::string filepath_in = file.path().string();
            std::string filename = file.path().filename().string();

            // open the file
            DataManager manager_in;
            manager_in.setLittleEndian();
            manager_in.readFromFile(filepath_in);
            const uint32_t fileSize = manager_in.readInt32();

            Data dat_out(fileSize);
            const DataManager manager_out(dat_out);
            RLE_NSXPS4_DECOMPRESS(manager_in.ptr, manager_in.size - 4,
                                 manager_out.ptr, manager_out.size);

            // manager_out.writeToFile(
            // "C:\\Users\\Jerrin\\CLionProjects\\LegacyEditor\\out\\"
            // + filename);

            // TODO: get timestamp from file itself
            uint32_t timestamp = 0;
            allFiles.emplace_back(dat_out.data, fileSize, timestamp);
            File &lFile = allFiles.back();
            if (const char dimChar = filename.at(12) - 48; dimChar < 0
                || dimChar > 2) {
                lFile.fileType = FileType::NONE;
            } else {
                static constexpr FileType regDims[3] = {
                    FileType::REGION_OVERWORLD,
                    FileType::REGION_NETHER,
                    FileType::REGION_END
                };
                lFile.fileType = regDims[dimChar];
            }
            const int16_t rX = static_cast<int8_t>(strtol(
                filename.substr(13, 2).c_str(), nullptr, 16));
            const int16_t rZ = static_cast<int8_t>(strtol(
                filename.substr(15, 2).c_str(), nullptr, 16));
            auto* nbt = lFile.createNBTTagCompound();
            nbt->setTag("x", createNBT_INT16(rX));
            nbt->setTag("z", createNBT_INT16(rZ));
        }

        updatePointers();

        return SUCCESS;
    }


    /**
     * 0- 3 bytes: 00 00 00 00
     * 4- 7 bytes: file size
     * 8-11 bytes: file listing?
     * @return
     */
    int FileListing::readVita(FILE* f_in, Data& data,
        u64 source_binary_size, const u32 file_size) {
        printf("Detected Vita savefile, converting\n\n");
        console = CONSOLE::VITA;

        // total size of file
        source_binary_size -= 8;
        data.size = file_size;

        // allocate memory
        Data src(source_binary_size);
        data.allocate(data.size);

        // goto offset 8 for the data, read data into src
        fseek(f_in, 8, SEEK_SET);
        fread(src.data, 1, source_binary_size, f_in);

        RLEVITA_DECOMPRESS(src.data, src.size, data.data, data.size);

        src.deallocate();

        return SUCCESS;
    }


    int FileListing::readWiiU(FILE* f_in, Data& data,
        u64 source_binary_size, const u32 file_size) {
        printf("Detected WiiU savefile, converting\n\n");
        console = CONSOLE::WIIU;

        // total size of file
        source_binary_size -= 8;

        auto src = Data(source_binary_size * 2);

        if(!data.allocate(file_size)) {
            return MALLOC_FAILED;
        }

        fseek(f_in, 8, SEEK_SET);
        fread(src.start(), 1, source_binary_size, f_in);

        if (tinf_zlib_uncompress(data.start(), &data.size,
            src.start(), source_binary_size) != 0) {
            src.deallocate();
            return DECOMPRESS;
        }
        src.deallocate();
        return SUCCESS;
    }


    int FileListing::readNSXorPS4(FILE* f_in, Data& data,
        u64 source_binary_size, const u32 file_size) {
        printf("Detected Switch/Ps4 savefile, converting\n\n");
        console = CONSOLE::SWITCH;

        if(!data.allocate(file_size)) {
            return MALLOC_FAILED;
        }

        source_binary_size -= 8;
        auto src = Data(source_binary_size);
        fseek(f_in, 8, SEEK_SET);
        fread(src.start(), 1, source_binary_size, f_in);

        if (tinf_zlib_uncompress(data.start(), &data.size,
                                 src.start(), source_binary_size) != 0) {
            src.deallocate();
            return DECOMPRESS;
        }
        src.deallocate();
        return SUCCESS;
    }


    /// ps3 writeFile files don't need decompressing\n
    /// TODO: figure out if this comment is actually important or not
    /// TODO: check from regionFile chunk what console it is if uncompressed
    int FileListing::readPs3(FILE* f_in, Data& data,
        const u64 source_binary_size, u32 file_size) {
        printf("Detected compressed PS3 savefile, converting\n\n");
        console = CONSOLE::PS3;

        // source
        auto src = Data(source_binary_size);

        // destination
        if (!data.allocate(file_size)) {
            return MALLOC_FAILED;
        }

        // decompress src -> data
        fseek(f_in, UNION_HEADER_SIZE, SEEK_SET);
        src.size -= UNION_HEADER_SIZE;
        fread(src.start(), 1, src.size, f_in);
        tinf_uncompress(data.start(), &file_size, src.start(), src.getSize());
        src.deallocate();

        if (file_size == 0) {
            printf_err("%s", error3);
            return DECOMPRESS;
        }

        return SUCCESS;
    }


    int FileListing::readRpcs3(FILE* f_in, Data& data,
        const u64 source_binary_size) {
        printf("Detected uncompressed PS3 / RPCS3 savefile, converting\n\n");
        console = CONSOLE::RPCS3;
        if (!data.allocate(source_binary_size)) {
            return MALLOC_FAILED;
        }
        fseek(f_in, 0, SEEK_SET);
        fread(data.start(), 1, data.size, f_in);
        return SUCCESS;
    }


    int FileListing::readXbox360DAT(FILE* f_in, Data& data,
        const u32 file_size, const u32 src_size) {
        printf("Detected Xbox360 .dat savefile, converting\n\n");
        console = CONSOLE::XBOX360;

        // allocate source memory
        Data src(src_size - 8);

        // allocate destination memory
        if (!data.allocate(file_size)) {
            return MALLOC_FAILED;
        }

        // decompress src -> data
        fread(src.start(), 1, src.size, f_in);
        data.size = XDecompress(data.start(), &data.size,
            src.start(), src.getSize());
        src.deallocate();
        if (data.size == 0) {
            printf_err("%s", error3);
            return DECOMPRESS;
        }

        return SUCCESS;
    }


    int FileListing::readXbox360BIN(FILE* f_in, Data& data,
        u64 source_binary_size) {
        console = CONSOLE::XBOX360;
        /**
        printf("Detected Xbox360 .bin savefile, converting\n\n");

        fseek(f_in, 0, SEEK_SET);

        Data bin(source_binary_size);
        fread(bin.start(), 1, source_binary_size, f_in);
        fileInfo = extractSaveGameDat(bin.start(), static_cast<i64>(source_binary_size));
        bin.deallocate(); // TODO: IDK if it should but it is for now

        const u32 src_size = fileInfo.saveFileData.readInt32() - 8;

        // at offset 8
        data.size = fileInfo.saveFileData.readInt64();

        if (!data.allocate(data.size)) {
            return MALLOC_FAILED;
        }
        data.size = XDecompress(data.start(), &data.size, fileInfo.saveFileData.data, src_size);
        */
        return SUCCESS;
    }


}