#include "fileListing.hpp"

#include <filesystem>
#include <iostream>

#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"


inline bool endswith(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


inline bool startswith(const std::string &value, const std::string &prefix) {
    if (prefix.size() > value.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), value.begin());
}


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
    // printf("\nFile Count: %u\n", fileCount);
    // printf("Buffer Size: %u\n", managerIn.size);

    for (int fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * 144);


        std::string fileName = managerIn.readWAsString(64); // m 128 bytes / 2 per char
        std::string originalFileName = fileName;

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

        if (endswith(fileName, ".mcr")) {
            if (startswith(fileName, "DIM-1")) {
                dimensionFilePtrs.push_back(&file);
                netherFilePtrs.push_back(&file);
            } else if (startswith(fileName, "DIM1")) {
                dimensionFilePtrs.push_back(&file);
                endFilePtrs.push_back(&file);
            } else if (startswith(fileName, "r")) {
                dimensionFilePtrs.push_back(&file);
                overworldFilePtrs.push_back(&file);
            } else {
                printf("File '%s' is not from any dimension!\n", fileName.c_str());
                continue;
            }
        } else if (fileName == "level.dat") {
            levelFilePtr = &file;
        } else if (startswith(fileName, "data/map_")) {
            mapFilePtrs.push_back(&file);
        } else if (fileName == "data/villages.dat") {
            villageFilePtr = &file;
        } else if (fileName == "largeMapDataMappings.dat") {
            largeMapDataMappingsFilePtr = &file;
        } else if (startswith(fileName, "data/")) {
            structureFilePtrs.push_back(&file);
        } else if (endswith(fileName, ".grf")) {
            grfFilePtr = &file;
        } else if (startswith(fileName, "players/")) {
            playerFilePtrs.push_back(&file);
        } else if (fileName.find('/') == -1) {
            playerFilePtrs.push_back(&file);
        } else {
            printf("Unknown File: %s\n", fileName.c_str());
        }
        // printf("%2u. (@%7u)[%7u]  - %s\n", fileIndex + 1, index, file.size, originalFileName.c_str());
    }
    // printf("Total File Size: %u\n\n", total_size);

}


void swapInt32(u8* ptr) {
    u32 value;
    memcpy(&value, ptr, sizeof(u32));
    value = (value >> 24) |
            ((value << 8) & 0x00FF0000) |
            ((value >> 8) & 0x0000FF00) |
            (value << 24);
    memcpy(ptr, &value, sizeof(u32));
}


void FileListing::convertRegions(CONSOLE consoleOut) {
    for (File* file : dimensionFilePtrs) {
        RegionManager region(console);
        region.read(file);
        Data data = region.write(consoleOut);
        delete[] file->data.data;
        file->data = data;
    }
}


void FileListing::deleteAllChunks() {
    RegionManager region(console);
    for (File* file : dimensionFilePtrs) {
        region.read(file);
        for (auto& chunk : region.chunks) {
            chunk.sectors = 0;
        }
        Data data = region.write(console);
        delete[] file->data.data;
        file->data = data;
    }
}


void FileListing::saveToFolder(const std::string &folder) {

    namespace fs = std::filesystem;
    if (folder.length() < 10) {
        printf("tried to delete short directory, will not risk");
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
    u32 fileCount = 0;
    u32 fileDataSize = 0;
    for (const File &file: allFiles) {
        fileCount++;
        fileDataSize += file.data.getSize();
    }
    u32 fileInfoOffset = fileDataSize + 12;

    // printf("\nTotal File Count: %u\n", fileCount);
    // printf("Total File Size: %u\n", fileDataSize);

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
    // std::cout << "\nwriting back to file" << std::endl;
    int count = 0;
    for (File &fileIter: allFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.additionalData, fileIter.size, fileIter.name.c_str());

        managerOut.writeWString(fileIter.name, 64);
        managerOut.writeInt32(fileIter.data.getSize());

        // if (!fileIter.isEmpty()) {
        managerOut.writeInt32(fileIter.additionalData);
        managerOut.writeInt64(fileIter.timestamp);
        // }
        count++;
    }

    // printf("Buffer Size: %u\n", data.size);

    return dataOut;
}
