#include "fileListing.hpp"

#include <filesystem>
#include <iostream>
#include <algorithm>

#include "LegacyEditor/utils/endian.hpp"

#include "ConsoleParser.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"


namespace fs = std::filesystem;


i16 extractMapNumber(const std::string& str) {
    static const std::string start = "map_";
    static const std::string end = ".dat";
    size_t startPos = str.find(start);
    size_t endPos = str.find(end);

    if (startPos != std::string::npos && endPos != std::string::npos) {
        startPos += start.length();

        std::string numberStr = str.substr(startPos, endPos - startPos);
        return (i16)std::stoi(numberStr);
    }
    return 32767;
}


std::pair<int, int> extractRegionCoords(const std::string& filename) {
    size_t lastDot = filename.find_last_of('.');
    std::string relevantPart = filename.substr(0, lastDot);

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


FileListing::FileListing(ConsoleParser& consoleParser) : console(consoleParser.console) {
    read(consoleParser);
}


MU FileListing::FileListing(ConsoleParser* consoleParser) : console(consoleParser->console) {
    read(*consoleParser);
}


void FileListing::read(Data &dataIn) {
    DataManager managerIn(dataIn, consoleIsBigEndian(console));

    u32 indexOffset = managerIn.readInt32();
    u32 fileCount = managerIn.readInt32();
    oldestVersion = managerIn.readInt16();
    currentVersion = managerIn.readInt16();

    allFiles.clear();
    allFiles.reserve(fileCount);

    u32 total_size = 0;
    for (int fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * FILE_HEADER_SIZE);

        std::string fileName = managerIn.readWAsString(64);

        u32 fileSize = managerIn.readInt32();
        total_size += fileSize;

        u32 index = managerIn.readInt32();
        u64 timestamp = managerIn.readInt64();

        if (!fileSize) {
            printf("Skipping empty file \"%s\"\n", fileName.c_str());
            continue;
        }

        managerIn.seek(index);
        u8* data = managerIn.readBytes(fileSize);

        allFiles.emplace_back(data, fileSize, timestamp);
        File &file = allFiles.back();
        printf("%s\n", fileName.c_str());

        // region file
        if (fileName.ends_with(".mcr")) {
            if (fileName.starts_with("DIM-1")) {
                file.fileType = FileType::REGION_NETHER;
            } else if (fileName.starts_with("DIM1")) {
                file.fileType = FileType::REGION_END;
            } else if (fileName.starts_with("r")) {
                file.fileType = FileType::REGION_OVERWORLD;
            }
            auto* nbt = file.createNBTTagCompound();
            auto pair = extractRegionCoords(fileName);
            nbt->setTag("x", createNBT_INT16((i16)pair.first));
            nbt->setTag("z", createNBT_INT16((i16)pair.second));
            continue;
        }

        if (fileName == "level.dat") {
            file.fileType = FileType::LEVEL;
            continue;
        }

        if (fileName.starts_with("data/map_")) {
            file.fileType = FileType::MAP;
            auto* nbt = file.createNBTTagCompound();
            i16 mapNumber = extractMapNumber(fileName);
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


MU void FileListing::convertRegions(CONSOLE consoleOut) {
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


MU void FileListing::deleteAllChunks() {
    RegionManager region(console);
    for (FileList* fileList : dimFileLists) {
        for (File* file: *fileList) {
            region.read(file);
            for (auto& chunk: region.chunks) {
                chunk.size = 0;
            }
            Data data = region.write(console);
            delete[] file->data.data;
            file->data = data;
        }
    }
}


void FileListing::saveToFolder(const std::string &folder) {

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

    for (File &file: allFiles) {
        std::string fullPath = folder + "\\" + file.constructFileName(console);
        fs::path path(fullPath);

        if (!fs::exists(path.parent_path())) {
            fs::create_directories(path.parent_path());
        }

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
    u32 totalFileSize = fileInfoOffset + FILE_HEADER_SIZE * fileCount;

    Data dataOut(totalFileSize);
    DataManager managerOut(dataOut, consoleIsBigEndian(consoleOut));

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
    for (File &fileIter: allFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.additionalData, fileIter.size, fileIter.name.c_str());
        std::string fileIterName = fileIter.constructFileName(consoleOut);
        managerOut.writeWString(fileIterName, 64);
        managerOut.writeInt32(fileIter.data.getSize());
        managerOut.writeInt32(fileIter.additionalData);
        managerOut.writeInt64(fileIter.timestamp);
    }

    return dataOut;
}


// TODO: this function probably doesn't work
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
    clearActionsRemove[fileType]();
    return collectedFiles;
}


void FileListing::deallocate() {
    for (File& file : allFiles) {
        delete[] file.data.data;
        file.data.data = nullptr;
    }
    clearPointers();
    allFiles.clear();
    oldestVersion = 0;
    currentVersion = 0;

}


void FileListing::clearPointers() {
    region_overworld.clear();
    region_nether.clear();
    region_end.clear();
    entity_overworld.clear();
    entity_nether.clear();
    entity_end.clear();
    maps.clear();
    structures.clear();
    players.clear();
    largeMapDataMappings = nullptr;
    level = nullptr;
    grf = nullptr;
    village = nullptr;
}


void FileListing::updatePointers() {
    clearPointers();
    for (File& file : allFiles) {
        switch(file.fileType) {
            case FileType::STRUCTURE:
                structures.push_back(&file);
                break;
            case FileType::VILLAGE:
                village = &file;
                break;
            case FileType::DATA_MAPPING:
                largeMapDataMappings = &file;
                break;
            case FileType::MAP:
                maps.push_back(&file);
                break;
            case FileType::REGION_NETHER:
                region_nether.push_back(&file);
                break;
            case FileType::REGION_OVERWORLD:
                region_overworld.push_back(&file);
                break;
            case FileType::REGION_END:
                region_end.push_back(&file);
                break;
            case FileType::PLAYER:
                players.push_back(&file);
                break;
            case FileType::LEVEL:
                level = &file;
                break;
            case FileType::GRF:
                grf = &file;
                break;
            case FileType::ENTITY_NETHER:
                entity_nether.push_back(&file);
                break;
            case FileType::ENTITY_OVERWORLD:
                entity_overworld.push_back(&file);
                break;
            case FileType::ENTITY_END:
                entity_end.push_back(&file);
                break;
            default:
                break;
        }
    }
}


void FileListing::removeFileTypes(const std::set<FileType>& typesToRemove) {

    for (int index = 0; index < allFiles.size();) {
        if (typesToRemove.contains(allFiles[index].fileType)) {
            delete[] allFiles[index].data.data;
            allFiles[index].data.data = nullptr;
            allFiles.erase(allFiles.begin() + index);
        } else {
            index++;
        }
    }

    for (const auto& fileType : typesToRemove) {
        clearActionsRemove[fileType]();
    }

    updatePointers();
}


MU void FileListing::addFiles(std::vector<File> filesIn) {
    allFiles.insert(allFiles.end(), filesIn.begin(), filesIn.end());
    updatePointers();
}