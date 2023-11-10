#include "fileListing.hpp"

#include <filesystem>
#include <iostream>
#include <algorithm>

#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"




void FileListing::read(Data &dataIn) {
    DataManager managerIn(dataIn);

    if (console == CONSOLE::VITA) {
        managerIn.setLittleEndian();
    }

    u32 indexOffset = managerIn.readInt32();
    u32 fileCount = managerIn.readInt32();
    oldestVersion = managerIn.readInt16();
    currentVersion = managerIn.readInt16();
    u32 total_size = 0;

    allFiles.clear();
    allFiles.reserve(fileCount);

    for (int fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * 144);

        std::string fileName = managerIn.readWAsString(64);

        u32 fileSize = managerIn.readInt32();
        total_size += fileSize;

        u8 *data = nullptr;
        u32 index = managerIn.readInt32();
        u64 timestamp = managerIn.readInt64();

        if (!fileSize) {
            printf("Skipping empty file \"%s\"\n", fileName.c_str());
            continue;
        }

        managerIn.seek(index);
        data = managerIn.readBytes(fileSize);

        allFiles.emplace_back(data, fileSize, fileName, timestamp);

        File &file = allFiles.back();

        if (fileName.ends_with(".mcr")) {
            if (fileName.starts_with("DIM-1")) {
                file.fileType = FileType::REGION_NETHER;
            } else if (fileName.starts_with("DIM1")) {
                file.fileType = FileType::REGION_END;
            } else if (fileName.starts_with("r")) {
                file.fileType = FileType::REGION_OVERWORLD;
            } else {
                printf("File '%s' is not from any dimension!\n", fileName.c_str());
            }
        } else if (fileName == "level.dat") {
            file.fileType = FileType::LEVEL;
        } else if (fileName.starts_with("data/map_")) {
            file.fileType = FileType::MAP;
        } else if (fileName == "data/villages.dat") {
            file.fileType = FileType::VILLAGE;
        } else if (fileName == "data/largeMapDataMappings.dat") {
            file.fileType = FileType::DATA_MAPPING;
        } else if (fileName.starts_with("data/")) {
            file.fileType = FileType::STRUCTURE;
        } else if (fileName.ends_with(".grf")) {
            file.fileType = FileType::GRF;
        } else if (fileName.starts_with("players/") || fileName.find('/') == -1) {
            file.fileType = FileType::PLAYER;
        } else {
            printf("Unknown File: %s\n", fileName.c_str());
        }
    }
    updatePointers();
}


void FileListing::convertRegions(CONSOLE consoleOut) {
    for (FileList* fileList : dimFileLists) {
        for (File* file : *fileList) {
            RegionManager region(console);
            region.read(file);
            Data data = region.write(consoleOut);
            delete[] file->data.data;
            file->data = data;
        }
    }
}



void FileListing::deleteAllChunks() {
    RegionManager region(console);
    for (FileList* fileList : dimFileLists) {
        for (File* file: *fileList) {
            region.read(file);
            for (auto& chunk: region.chunks) {
                chunk.sectors = 0;
            }
            Data data = region.write(console);
            delete[] file->data.data;
            file->data = data;
        }
    }
}


void FileListing::saveToFolder(const std::string &folder) {

    namespace fs = std::filesystem;
    if (folder.length() < 10) {
        printf("tried to delete short directory, will not risk");
        return;
    }

    fs::path _dir_path{folder};
    if (fs::exists(_dir_path) && fs::is_directory(_dir_path)) {
        for (const auto &entry: fs::directory_iterator(_dir_path)) {
            try {
                fs::remove_all(entry.path());
            } catch (const fs::filesystem_error &e) {
                std::cerr << "Filesystem error: " << e.what() << '\n';
            }
        }
    }


    for (const File &file: allFiles) {
        std::string fullPath = folder + "\\" + file.name;
        fs::path path(fullPath);
        if (!fs::exists(path.parent_path())) {
            fs::create_directories(path.parent_path());
        }
    }

    // step 2: write files to correct locations
    for (File &file: allFiles) {
        std::string fullPath = folder + "\\" + file.name;
        DataManager fileOut(file.data);
        fileOut.writeToFile(fullPath);
    }
}


Data FileListing::write(CONSOLE consoleOut) {

    // step 1: get the file count and size of all sub-files
    u32 fileCount = 0, fileDataSize = 0;
    for (const File &file: allFiles) {
        fileCount++;
        fileDataSize += file.data.getSize();
    }
    u32 fileInfoOffset = fileDataSize + 12;

    // step 2: find total binary size and create its data buffer
    u32 totalFileSize = fileInfoOffset + 144 * fileCount;

    Data dataOut(totalFileSize);
    DataManager managerOut(dataOut);

    if (consoleOut == CONSOLE::VITA) {
        managerOut.setLittleEndian();
    }

    // step 3: write start
    managerOut.writeInt32(fileInfoOffset);
    managerOut.writeInt32(fileCount);
    managerOut.writeInt16(oldestVersion);
    managerOut.writeInt16(currentVersion);


    // step 4: write each files data
    // I am using additionalData as the offset into the file its data is at
    u32 index = 12;
    for (File &fileIter: allFiles) {
        fileIter.additionalData = index;
        index += fileIter.data.getSize();
        managerOut.writeFile(fileIter);
    }

    // step 5: write file metadata
    int count = 0;
    for (File &fileIter: allFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.
        // additionalData, fileIter.size, fileIter.name.c_str());

        managerOut.writeWString(fileIter.name, 64);
        managerOut.writeInt32(fileIter.data.getSize());
        managerOut.writeInt32(fileIter.additionalData);
        managerOut.writeInt64(fileIter.timestamp);

        count++;
    }

    return dataOut;
}




std::vector<File> FileListing::collectFiles(FileType fileType) {
    std::vector<File> collectedFiles;
    allFiles.erase(
            std::remove_if(
                    allFiles.begin(),
                    allFiles.end(),
                    [&collectedFiles, &fileType](const File& file) {
                        bool isType = file.fileType == fileType;
                        if (isType) {
                            collectedFiles.push_back(file);

                        }
                        return isType;
                    }
                    ),
            allFiles.end()
    );
    clearActions[fileType]();
    return collectedFiles;
}




void FileListing::clear() {
    for (File& file : allFiles) {
        delete[] file.data.data;
    }
    clearPointers();
    allFiles.clear();
    oldestVersion = 0;
    currentVersion = 0;

}


void FileListing::clearPointers() {
    overworldFilePtrs.clear();
    netherFilePtrs.clear();
    endFilePtrs.clear();
    mapFilePtrs.clear();
    structureFilePtrs.clear();
    playerFilePtrs.clear();
    largeMapDataMappingsFilePtr = nullptr;
    levelFilePtr = nullptr;
    grfFilePtr = nullptr;
    villageFilePtr = nullptr;
}

void FileListing::updatePointers() {
    clearPointers();
    for (File& file : allFiles) {
        switch(file.fileType) {
            case FileType::STRUCTURE:
                structureFilePtrs.push_back(&file);
                break;
            case FileType::VILLAGE:
                villageFilePtr = &file;
                break;
            case FileType::DATA_MAPPING:
                largeMapDataMappingsFilePtr = &file;
                break;
            case FileType::MAP:
                mapFilePtrs.push_back(&file);
                break;
            case FileType::REGION_NETHER:
                netherFilePtrs.push_back(&file);
                break;
            case FileType::REGION_OVERWORLD:
                overworldFilePtrs.push_back(&file);
                break;
            case FileType::REGION_END:
                endFilePtrs.push_back(&file);
                break;
            case FileType::PLAYER:
                playerFilePtrs.push_back(&file);
                break;
            case FileType::LEVEL:
                levelFilePtr = &file;
                break;
            case FileType::GRF:
                grfFilePtr = &file;
                break;
            default:
                break;
        }
    }
}

















