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
    DataManager thumbnailImage = DataManager(nullptr, 0, true); // TODO: lol

    int readHeader(DataManager& binFile);
};


/// extract a file (by FileEntry) to a designated file path
class StfsPackage {
public:
    explicit StfsPackage(DataManager& input) : data(input) {}
    
    StfsFileListing getFileListing() { return fileListing; }
    void extract(StfsFileEntry* entry, DataManager& out);
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
    ND u32 computeLevel2BackingHashBlockNumber();
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

FileInfo extractSaveGameDat(u8* inputData, i64 inputSize);
