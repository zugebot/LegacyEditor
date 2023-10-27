#include "fileListing.hpp"

#include <iostream>


inline bool endswith(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


inline bool startswith(const std::string& value, const std::string& prefix) {
    if (prefix.size() > value.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), value.begin());
}


void FileListing::read(DataInManager& managerIn) {
    // std::cout << "Data Pointer: " << reinterpret_cast<void *>(managerIn.ptr) << std::endl;

    u32 indexOffset = managerIn.readInt();
    u32 fileCount = managerIn.readInt();
    oldestVersion = managerIn.readShort();
    currentVersion = managerIn.readShort();
    u32 total_size = 0;

    allFiles.reserve(fileCount);
    printf("\nFile Count: %u\n", fileCount);
    printf("Buffer Size: %u\n", managerIn.size);

    for (int fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        managerIn.seek(indexOffset + fileIndex * 144);


        std::string fileName = managerIn.readWAsString(64, true); // m 128 bytes / 2 per char
        std::string originalFileName = fileName;

        u32 fileSize = managerIn.readInt();
        total_size += fileSize;

        u8* data = nullptr;
        u32 index = managerIn.readInt();
        u64 timestamp = managerIn.readLong();
        if (fileSize) {
            managerIn.seek(index);
            data = managerIn.readBytes(fileSize);
        }

        allFiles.emplace_back(data, fileSize, fileName, timestamp);

        File& file = allFiles.back();

        if (endswith(fileName, ".mcr")) {
            if (startswith(fileName, "DIM-1")) {
                if (fileName.find_first_of('/') != 5) {
                    fileName = fileName.substr(0, 5) + "/" + fileName.substr(5);
                }
                file.name = fileName;
                netherFilePtrs.push_back(&file);
            } else if (startswith(fileName, "DIM1")) {
                endFilePtrs.push_back(&file);
            } else if (startswith(fileName, "r")) {
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
        printf("%2u. [%7u] - %s\n", fileIndex + 1, file.size, originalFileName.c_str());
    }
    printf("Total File Size: %u\n\n", total_size);

}


DataOutManager FileListing::write() {
    std::cout << "SaveFileGroupListing::write()" << std::endl;

    // step 1: get the file count and size of all sub-files

    // calculate totals
    u32 fileCount = 0;
    u32 fileDataSize = 0;
    for (const File& file: allFiles) {
        fileCount++;
        fileDataSize += file.getSize();
    }
    u32 indexOffset = fileDataSize + 12;

    // debugging
    printf("\nTotal File Count: %u\n", fileCount);
    printf("Total File Size: %u\n", fileDataSize);

    // step 2: find total binary size and create its data buffer
    u32 totalFileSize = indexOffset + 144 * fileCount;

    DataOutManager data(totalFileSize);
    // data.setBigEndian();

    // step 3: write start
    data.writeInt(indexOffset);
    data.writeInt(fileCount);
    data.writeShort(oldestVersion);
    data.writeShort(currentVersion);

    // step 4: write each files data
    // I am using additionalData as the offset into the file its data is at
    u32 index;
    for (File& fileIter: allFiles) {
        index = data.getSize();
        fileIter.additionalData = index;
        data.writeFile(fileIter);
    }

    // step 5: write file metadata
    // std::cout << "\nwriting back to file" << std::endl;
    int count = 0;
    for (File& fileIter: allFiles) {
        printf("%2u. [%7u] - %s\n", count + 1, fileIter.size, fileIter.name.c_str());

        data.writeWString(fileIter.name, 64, false);
        data.writeInt(fileIter.getSize());

        // if (!fileIter.isEmpty()) {
            data.writeInt(fileIter.additionalData);
            data.writeLong(fileIter.timestamp);
        // }
        count++;
    }

    printf("Buffer Size: %u\n", data.size);

    return data;
}
