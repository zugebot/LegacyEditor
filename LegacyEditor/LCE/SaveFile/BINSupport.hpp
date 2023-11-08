#pragma once

#include <chrono>
#include <iostream>
#include <optional>

#include "FileInfo.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


struct StfsFileEntry {
    u32 entryIndex{};
    std::string name;
    u8 nameLen{};
    u8 flags{};
    int blocksForFile{};
    int startingBlockNum{};
    u16 pathIndicator{};
    u32 fileSize{};
    u32 createdTimeStamp{};
    u32 accessTimeStamp{};
    u32 fileEntryAddress{};
    std::vector<int> blockChain;
};


struct StfsFileListing {
    std::vector<StfsFileEntry> fileEntries;
    std::vector<StfsFileListing> folderEntries;
    StfsFileEntry folder;
};


struct StfsVD {
    u8 size;
    //u8 reserved;
    u8 blockSeparation;
    u16 fileTableBlockCount;
    int fileTableBlockNum;
    //u8 topHashTableHash[0x14];
    u32 allocatedBlockCount;
    u32 unallocatedBlockCount;

    void readStfsVD(DataManager& input);
};


#pragma pack(push, 1)
struct HashEntry {
    u8 blockHash[0x14];
    u8 status;
    u32 nextBlock;
};
#pragma pack(pop)


struct HashTable {
    u8 level;
    u32 trueBlockNumber;
    u32 entryCount;
    HashEntry entries[0xAA];
    u32 addressInFile;
};


class BINHeader {
public:
    u32 headerSize{};
    StfsVD stfsVD{};
    std::wstring displayName;
    DataManager thumbnailImage = DataManager(nullptr, 0); // TODO: lol

    int readHeader(DataManager& binFile);
};


/// extract a file (by FileEntry) to a designated file path
class StfsPackage {
public:
    explicit StfsPackage(DataManager& input) : data(input) {}
    
    StfsFileListing getFileListing() { return fileListing; }
    void extract(StfsFileEntry* entry, DataManager& out) {
        if (entry->nameLen == 0) { entry->name = "default"; }

        // get the file size that we are extracting
        u32 fileSize = entry->fileSize;
        if (fileSize == 0) { return; }

        // check if all the blocks are consecutive
        if (entry->flags & 1) {
            // allocate 0xAA blocks of memory, for maximum efficiency, yo
            auto* buffer = new u8[0xAA000];

            // seek to the beginning of the file
            u32 startAddress = blockToAddress(entry->startingBlockNum);
            data.seek(startAddress);

            // calculateOffset the number of blocks to read before we hit a table
            u32 blockCount = (computeLevel0BackingHashBlockNumber(entry->startingBlockNum) + blockStep[0]) -
                                  ((startAddress - firstHashTableAddress) >> 0xC);

            // pick up the change at the beginning, until we hit a hash table
            if ((u32) entry->blocksForFile <= blockCount) {
                data.readOntoData(entry->fileSize, buffer);
                out.write(buffer, entry->fileSize);

                //out.Close();

                // free the temp buffer
                delete[] buffer;
                return;
            } else {
                data.readOntoData(blockCount << 0xC, buffer);
                out.write(buffer, blockCount << 0xC);
            }

            // extract the blocks inbetween the tables
            u32 tempSize = (entry->fileSize - (blockCount << 0xC));
            while (tempSize >= 0xAA000) {
                // skip past the hash table(s)
                u32 currentPos = data.getPosition();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // read in the 0xAA blocks between the tables
                data.readOntoData(0xAA000, buffer);

                // Write the bytes to the out file
                out.write(buffer, 0xAA000);

                tempSize -= 0xAA000;
                blockCount += 0xAA;
            }

            // pick up the change at the end
            if (tempSize != 0) {
                // skip past the hash table(s)
                u32 currentPos = data.getPosition();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // read in the extra crap
                data.readOntoData(tempSize, buffer);

                // Write it to the out file
                out.write(buffer, tempSize);
            }

            // free the temp buffer
            delete[] buffer;
        } else {
            // generate the blockchain which we have to extract
            u32 fullReadCounts = fileSize / 0x1000;

            fileSize -= (fullReadCounts * 0x1000);

            u32 block = entry->startingBlockNum;

            // allocate data for the blocks
            u8 buffer[0x1000];

            // read all the full blocks the file allocates
            for (u32 i = 0; i < fullReadCounts; i++) {
                extractBlock(block, buffer);
                out.write(buffer, 0x1000);

                block = getBlockHashEntry(block).nextBlock;
            }

            // read the remaining data
            if (fileSize != 0) {
                extractBlock(block, buffer, fileSize);
                out.write(buffer, (int) fileSize);
            }
        }

        // cleanup
        //out.Close();
    }
    ND u32 blockToAddress(u32 blockNum);
    ND u32 getHashAddressOfBlock(u32 blockNum);
    ~StfsPackage() = default;
    ND BINHeader getMetaData() { return metaData; }
    
    /// parse the file
    void parse();

private:
    BINHeader metaData;
    StfsFileListing fileListing;
    DataManager& data;
    /// 0 female, 1 male
    u8 packageSex{};
    u32 blockStep[2]{};
    u32 firstHashTableAddress{};
    u8 topLevel{};
    HashTable topTable{};
    u32 tablesPerLevel[3]{};
    
    void readFileListing();
    void extractBlock(u32 blockNum, u8* inputData, u32 length = 0x1000);
    ND u32 computeBackingDataBlockNumber(u32 blockNum) const;
    HashEntry getBlockHashEntry(u32 blockNum);
    ND u32 computeLevelNBackingHashBlockNumber(u32 blockNum, u8 level);
    ND u32 computeLevel0BackingHashBlockNumber(u32 blockNum);
    ND u32 computeLevel1BackingHashBlockNumber(u32 blockNum);
    ND u32 computeLevel2BackingHashBlockNumber(u32 blockNum);
    void addToListing(StfsFileListing* fullListing, StfsFileListing* out);
    ND int calculateTopLevel() const;
    ND u32 GetHashTableSkipSize(u32 tableAddress);
};


struct TextChunk {
    std::string keyword;
    std::string text;
};


StfsFileEntry* findSavegameFileEntry(StfsFileListing& listing);


u32 c2n(char c);
i64 stringToHex(const std::string& str);
i64 stringToInt64(const std::string& str);



WorldOptions getTagsInImage(DataManager& image);



std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(u32 fat);



static FileInfo extractSaveGameDat(u8* inputData, i64 inputSize);
