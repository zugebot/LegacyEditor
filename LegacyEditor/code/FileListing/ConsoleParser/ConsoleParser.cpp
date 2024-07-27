#include "ConsoleParser.hpp"


int ConsoleParser::readListing(editor::FileListing* theListing, const Data &dataIn) {
    DataManager managerIn(dataIn, consoleIsBigEndian(myConsole));

    c_u32 indexOffset = managerIn.readInt32();
    u32 fileCount = managerIn.readInt32();
    theListing->myOldestVersion = managerIn.readInt16();
    theListing->myCurrentVersion = managerIn.readInt16();

    u32 FOOTER_ENTRY_SIZE = 144;
    if (theListing->myCurrentVersion <= 1) {
        FOOTER_ENTRY_SIZE = 136;
        fileCount /= 136;
    }

    theListing->myAllFiles.clear();

    MU u32 totalSize = 0;
    for (u32 fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * FOOTER_ENTRY_SIZE);

        std::string fileName = managerIn.readWAsString(WSTRING_SIZE);

        u32 fileSize = managerIn.readInt32();
        c_u32 index = managerIn.readInt32();
        u64 timestamp = 0;
        if (theListing->myCurrentVersion <= 1) {
            timestamp = managerIn.readInt64();
        }
        totalSize += fileSize;

        // if (fileSize == 0U) {
        //     printf("Skipping empty file \"%s\"\n", fileName.c_str());
        //     continue;
        // }

        managerIn.seek(index);
        u8* data = managerIn.readBytes(fileSize);

        // TODO: make sure all files are set with the correct console
        theListing->myAllFiles.emplace_back(myConsole, data, fileSize, timestamp);
        editor::LCEFile &file = theListing->myAllFiles.back();

        if (fileName.ends_with(".mcr")) {
            if (fileName.starts_with("DIM-1")) {
                file.fileType = lce::FILETYPE::REGION_NETHER;
            } else if (fileName.starts_with("DIM1")) {
                file.fileType = lce::FILETYPE::REGION_END;
            } else if (fileName.starts_with("r")) {
                file.fileType = lce::FILETYPE::REGION_OVERWORLD;
            }
            c_auto [fst, snd] = extractRegionCoords(fileName);
            file.setRegionX(static_cast<i16>(fst));
            file.setRegionZ(static_cast<i16>(snd));

        } else if (fileName == "entities.dat") {
            file.fileType = lce::FILETYPE::ENTITY_OVERWORLD;


        } else if (fileName.ends_with("entities.dat")) {
            if (fileName.starts_with("DIM-1")) {
                file.fileType = lce::FILETYPE::ENTITY_NETHER;
            } else if (fileName.starts_with("DIM1/")) {
                file.fileType = lce::FILETYPE::ENTITY_END;
            }

        } else if (fileName == "level.dat") {
            file.fileType = lce::FILETYPE::LEVEL;

        } else if (fileName.starts_with("data/map_")) {
            file.fileType = lce::FILETYPE::MAP;
            c_i16 mapNumber = extractMapNumber(fileName);
            file.setMapNumber(mapNumber);

        }else if (fileName == "data/villages.dat") {
            file.fileType = lce::FILETYPE::VILLAGE;

        } else if (fileName == "data/largeMapDataMappings.dat") {
            file.fileType = lce::FILETYPE::DATA_MAPPING;

        } else if (fileName.starts_with("data/")) {
            file.fileType = lce::FILETYPE::STRUCTURE;
            file.setFileName(fileName);

        } else if (fileName.ends_with(".grf")) {
            file.fileType = lce::FILETYPE::GRF;

        } else if (fileName.starts_with("players/") ||
                   fileName.find('/') == -1LLU) {
            file.fileType = lce::FILETYPE::PLAYER;
            file.setFileName(fileName);

        } else {
            printf("Unknown File: %s\n", fileName.c_str());
        }

    }

    theListing->updatePointers();
    printf("\n");

    return SUCCESS;
}


Data ConsoleParser::writeListing(editor::FileListing* theListing, const lce::CONSOLE consoleOut) const {

    // step 1: get the file count and size of all sub-files
    c_u32 fileCount = theListing->myAllFiles.size();
    u32 fileDataSize = 0;
    for (const editor::LCEFile& file: theListing->myAllFiles) {
        fileDataSize += file.data.getSize();
    }

    c_u32 fileInfoOffset = fileDataSize + 12;

    u32 FOOTER_ENTRY_SIZE = 144;
    if (theListing->myCurrentVersion <= 1) {
        FOOTER_ENTRY_SIZE = 136;
    }

    // step 2: find total binary size and create its data buffer
    c_u32 totalFileSize = fileInfoOffset + FOOTER_ENTRY_SIZE * fileCount;

    Data dataOut;
    dataOut.allocate(totalFileSize);
    memset(dataOut.data, 0, dataOut.size);

    DataManager managerOut(dataOut, consoleIsBigEndian(consoleOut));

    // step 3: write start
    managerOut.writeInt32(fileInfoOffset);
    u32 innocuousVariableName = fileCount;
    if (theListing->myCurrentVersion <= 1) {
        innocuousVariableName *= 136;
    }
    managerOut.writeInt32(innocuousVariableName);
    managerOut.writeInt16(theListing->myOldestVersion);
    managerOut.writeInt16(theListing->myCurrentVersion);

    // step 4: write each files data
    // I am using additionalData as the offset into the file its data is at
    u32 index = FILELISTING_HEADER_SIZE;
    for (editor::LCEFile& fileIter : theListing->myAllFiles) {
        fileIter.additionalData = index;
        index += fileIter.data.getSize();
        managerOut.writeFile(fileIter);
    }

    // step 5: write file metadata
    for (const editor::LCEFile& fileIter: theListing->myAllFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter
        // .additionalData, fileIter.size, fileIter.name.c_str());
        std::string fileIterName = fileIter.constructFileName(consoleOut, theListing->myHasSeparateRegions);
        managerOut.writeWStringFromString(fileIterName, WSTRING_SIZE);
        managerOut.writeInt32(fileIter.data.getSize());
        managerOut.writeInt32(fileIter.additionalData);
        if (theListing->myCurrentVersion > 1) {
            managerOut.writeInt64(fileIter.timestamp);
        }
    }

    return dataOut;
}


int ConsoleParser::readFileInfo(editor::FileListing* theListing) const {
    fs::path filePath = myFilePath.parent_path();

    bool has_external_fileinfo = true;
    switch (myConsole) {
        case lce::CONSOLE::PS3:
        case lce::CONSOLE::RPCS3:
        case lce::CONSOLE::PS4:
            filePath /= "THUMB";
            break;
        case lce::CONSOLE::VITA:
            filePath /= "THUMBDATA.BIN";
            break;
        case lce::CONSOLE::WIIU:
        case lce::CONSOLE::SWITCH: {
            filePath = myFilePath;
            filePath += ".ext";
            break;
        }
        case lce::CONSOLE::XBOX360:
            has_external_fileinfo = false;
            break;
        case lce::CONSOLE::NONE:
        default:
            return INVALID_CONSOLE;
    }

    int status;
    if (has_external_fileinfo) {
        if (fs::exists(filePath)) {
            theListing->fileInfo.readFile(filePath, myConsole);
            status = SUCCESS;
        } else {
            printf("FileInfo file not found/corrupt, setting defaulted data.\n");
            status = FILE_ERROR;
        }
    }

    if (!theListing->fileInfo.isLoaded) {
        theListing->fileInfo.defaultSettings();
        theListing->fileInfo.loadIngameThumbnail("assets/LegacyEditor/world-icon.png");
        status = SUCCESS;
    }

    return status;
}


/**
 * \brief Ps4 / Switch / Xbox1
 * \param inDirPath the directory containing the GAMEDATA files.
 * \return
 */
int ConsoleParser::readExternalFolder(editor::FileListing* theListing, const fs::path& inDirPath) {
    MU int fileIndex = -1;
    for (c_auto& file : fs::directory_iterator(inDirPath)) {

        // TODO: place non-used files in a cache?
        if (is_directory(file)) { continue; }
        fileIndex++;

        // initiate filename and filepath
        std::string filePathStr = file.path().string();
        std::string fileNameStr = file.path().filename().string();

        // open the file
        DataManager manager_in;
        manager_in.setLittleEndian(); // all of newgen is little endian
        manager_in.readFromFile(filePathStr);
        c_u32 fileSize = manager_in.readInt32();

        Data dat_out;
        dat_out.allocate(fileSize);
        const DataManager manager_out(dat_out);
        RLE_NSX_OR_PS4_DECOMPRESS(manager_in.ptr, manager_in.size - 4,
                                  manager_out.ptr, manager_out.size);

        // manager_out.writeToFile("C:\\Users\\Jerrin\\CLionProjects\\LegacyEditor\\out\\" + a_filename);

        // TODO: get timestamp from file itself / make one up
        u32 timestamp = 0;
        theListing->myAllFiles.emplace_back(theListing->myConsole, dat_out.data, fileSize, timestamp);
        editor::LCEFile &lFile = theListing->myAllFiles.back();
        if (c_auto dimChar = static_cast<char>(static_cast<int>(fileNameStr.at(12)) - 48);
            dimChar < 0 || dimChar > 2) {
            lFile.fileType = lce::FILETYPE::NONE;
        } else {
            static constexpr lce::FILETYPE regDims[3] = {
                    lce::FILETYPE::REGION_OVERWORLD,
                    lce::FILETYPE::REGION_NETHER,
                    lce::FILETYPE::REGION_END
            };
            lFile.fileType = regDims[static_cast<int>(static_cast<u8>(dimChar))];
        }
        c_i16 rX = static_cast<i8>(strtol(
                fileNameStr.substr(13, 2).c_str(), nullptr, 16));
        c_i16 rZ = static_cast<i8>(strtol(
                fileNameStr.substr(15, 2).c_str(), nullptr, 16));
        lFile.setRegionX(rX);
        lFile.setRegionZ(rZ);
    }

    theListing->updatePointers();

    return SUCCESS;
}
