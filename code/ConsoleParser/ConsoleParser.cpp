#include "ConsoleParser.hpp"


int ConsoleParser::readListing(const Data &dataIn) {
    DataManager managerIn(dataIn, getConsoleEndian(myConsole));

    c_u32 indexOffset = managerIn.read<u32>();
    u32 fileCount = managerIn.read<u32>();
    myListingPtr->myReadSettings.setOldestVersion(managerIn.read<u16>());
    myListingPtr->myReadSettings.setCurrentVersion(managerIn.read<u16>());

    u32 FOOTER_ENTRY_SIZE = 144;
    if (myListingPtr->myReadSettings.getCurrentVersion() <= 1) {
        FOOTER_ENTRY_SIZE = 136;
        fileCount /= 136;
    }

    myListingPtr->myAllFiles.clear();

    MU u32 totalSize = 0;
    for (u32 fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * FOOTER_ENTRY_SIZE);

        std::string fileName = managerIn.readWAsString(WSTRING_SIZE);

        u32 fileSize = managerIn.read<u32>();
        c_u32 index = managerIn.read<u32>();
        u64 timestamp = 0;
        if (myListingPtr->myReadSettings.getCurrentVersion() <= 1) {
            timestamp = managerIn.read<u64>();
        }
        totalSize += fileSize;

        managerIn.seek(index);
        u8* data = managerIn.readBytes(fileSize);

        // TODO: make sure all files are set with the correct console
        myListingPtr->myAllFiles.emplace_back(myConsole, data, fileSize, timestamp);
        editor::LCEFile &file = myListingPtr->myAllFiles.back();

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
            file.SetFileNameAndType(fileName, lce::FILETYPE::STRUCTURE);

        } else if (fileName.ends_with(".grf")) {
            file.SetFileNameAndType(fileName, lce::FILETYPE::GRF);

        } else if (fileName.starts_with("players/") ||
                   fileName.find('/') == -1LLU) {
            file.SetFileNameAndType(fileName, lce::FILETYPE::PLAYER);

        } else {
            printf("Unknown File: %s\n", fileName.c_str());
        }

    }

    myListingPtr->updatePointers();
    printf("\n");

    return SUCCESS;
}


Data ConsoleParser::writeListing(const lce::CONSOLE consoleOut) const {

    // step 1: get the file count and size of all sub-files
    c_u32 fileCount = myListingPtr->myAllFiles.size();
    c_i32 currentVersion = myListingPtr->myReadSettings.getCurrentVersion();

    u32 fileDataSize = 0;
    for (const editor::LCEFile& file: myListingPtr->myAllFiles) {
        fileDataSize += file.data.getSize();
    }

    c_u32 fileInfoOffset = fileDataSize + 12;

    u32 FOOTER_ENTRY_SIZE = 144;
    if (currentVersion <= 1) {
        FOOTER_ENTRY_SIZE = 136;
    }

    // step 2: find total binary size and create its data buffer
    c_u32 totalFileSize = fileInfoOffset + FOOTER_ENTRY_SIZE * fileCount;

    Data dataOut;
    dataOut.allocate(totalFileSize);
    memset(dataOut.data, 0, dataOut.size);

    DataManager managerOut(dataOut, getConsoleEndian(consoleOut));

    // step 3: write start
    managerOut.write<u32>(fileInfoOffset);
    u32 innocuousVariableName = fileCount;
    if (currentVersion <= 1) {
        innocuousVariableName *= 136;
    }
    managerOut.write<u32>(innocuousVariableName);
    managerOut.write<u16>(myListingPtr->myReadSettings.getOldestVersion());
    managerOut.write<u16>(currentVersion);

    // step 4: write each files data
    // I am using additionalData as the offset into the file its data is at
    u32 index = FILELISTING_HEADER_SIZE;
    for (editor::LCEFile& fileIter : myListingPtr->myAllFiles) {
        fileIter.additionalData = index;
        index += fileIter.data.getSize();
        managerOut.writeBytes(fileIter.data.start(), fileIter.data.size);
    }

    // step 5: write file metadata
    for (const editor::LCEFile& fileIter: myListingPtr->myAllFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter
        // .additionalData, fileIter.size, fileIter.name.c_str());
        std::string fileIterName = fileIter.constructFileName(consoleOut,
                                                              myListingPtr->myReadSettings.getHasSepRegions());
        managerOut.writeWStringFromString(fileIterName, WSTRING_SIZE);
        managerOut.write<u32>(fileIter.data.getSize());
        managerOut.write<u32>(fileIter.additionalData);
        if (currentVersion > 1) {
            managerOut.write<u64>(fileIter.timestamp);
        }
    }

    return dataOut;
}


void ConsoleParser::readFileInfo() const {
    fs::path filePath = myFilePath.parent_path();
    fs::path cachePathVita = myFilePath.parent_path().parent_path();
    cachePathVita /= "CACHE.BIN";

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
            goto XBOX360_SKIP_READING_FILEINFO;
        case lce::CONSOLE::NONE:
        default:
            return;
    }

    if (fs::exists(filePath)) {
        myListingPtr->fileInfo.readFile(filePath, myConsole);

    } else if (myConsole == lce::CONSOLE::VITA && fs::exists(cachePathVita)) {
        std::string folderName = myFilePath.parent_path().filename().string();
        myListingPtr->fileInfo.readCacheFile(cachePathVita, folderName);

    } else {
        printf("FileInfo file not found/corrupt, setting defaulted data.\n");
    }

XBOX360_SKIP_READING_FILEINFO:
    if (!myListingPtr->fileInfo.isLoaded) {
        myListingPtr->fileInfo.defaultSettings();
        myListingPtr->fileInfo.loadFileAsThumbnail("assets/LegacyEditor/world-icon.png");
    }

    if (myListingPtr->fileInfo.baseSaveName.empty()) {
        myListingPtr->fileInfo.baseSaveName = L"New World";
    }
}


/**
 * \brief Ps4 / Switch / Xbox1
 * \param inDirPath the directory containing the GAMEDATA files.
 * \return
 */
int ConsoleParser::readExternalFolder(const fs::path& inDirPath) {
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
        manager_in.setEndian(Endian::Little); // all of newgen is little endian
        manager_in.readFromFile(filePathStr);
        c_u32 fileSize = manager_in.read<u32>();

        Data dat_out;
        dat_out.allocate(fileSize);
        const DataManager manager_out(dat_out);
        RLE_NSX_OR_PS4_DECOMPRESS(manager_in.ptr(), manager_in.size() - 4,
                                  manager_out.ptr(), manager_out.size());

        // manager_out.writeToFile("C:\\Users\\Jerrin\\CLionProjects\\LegacyEditor\\out\\" + a_filename);

        // TODO: get timestamp from file itself / make one up
        u32 timestamp = 0;
        myListingPtr->myAllFiles.emplace_back(myListingPtr->myReadSettings.getConsole(), dat_out.data, fileSize, timestamp);
        editor::LCEFile &lFile = myListingPtr->myAllFiles.back();
        static constexpr lce::FILETYPE REGION_DIMENSIONS[3] = {
                lce::FILETYPE::REGION_OVERWORLD,
                lce::FILETYPE::REGION_NETHER,
                lce::FILETYPE::REGION_END
        };
        if (c_auto dimChar = static_cast<char>(static_cast<int>(fileNameStr.at(12)) - 48);
            dimChar < 0 || dimChar > 2) {
            lFile.fileType = lce::FILETYPE::NONE;
        } else {
            lFile.fileType = REGION_DIMENSIONS[static_cast<int>(static_cast<u8>(dimChar))];
        }
        c_i16 rX = static_cast<i8>(strtol(
                fileNameStr.substr(13, 2).c_str(), nullptr, 16));
        c_i16 rZ = static_cast<i8>(strtol(
                fileNameStr.substr(15, 2).c_str(), nullptr, 16));
        lFile.setRegionX(rX);
        lFile.setRegionZ(rZ);
    }

    myListingPtr->updatePointers();

    return SUCCESS;
}
