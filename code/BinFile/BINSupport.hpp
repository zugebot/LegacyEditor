#pragma once

#include <chrono>
#include <optional>

#include "common/dataManager.hpp"


namespace editor {

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
        // u8 reserved;
        u8 blockSeparation;
        u16 fileTableBlockCount;
        int fileTableBlockNum;
        // u8 topHashTableHash[0x14];
        u32 allocBlockCount;
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


    /// extractFile a file (by FileEntry) to a designated file path
    class StfsPackage {
    public:
        explicit StfsPackage(DataManager& input) : data(input) {}

        ND StfsFileListing getFileListing() { return fileListing; }

        /// Instead of taking a 'DataOutputManager', it now instead returns 'Data'.
        Data extractFile(StfsFileEntry* entry);

        ND u32 blockToAddress(u32 blockNum) const;

        ND u32 getHashAddressOfBlock(u32 blockNum);

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
        u32 tablesPerLvl[3]{};

        void readFileListing();
        void extractBlock(u32 blockNum, u8* inputData, u32 length = 0x1000) const;
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


    StfsFileEntry* findSavegameFileEntry(StfsFileListing& listing);

    // WorldOptions getTagsInImage(DataManager& image);

    // std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(u32 fat);

    // FileInfo extractSaveGameDat(u8* inputData, i64 inputSize);


    inline std::time_t to_utc_time(std::tm& tm) {
#ifdef _WIN32
        return _mkgmtime(&tm);
#else
        return timegm(&tm);
#endif
    }


    static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(uint32_t fat) {
        uint32_t year = (fat >> 25) + 1980;
        uint32_t month = 0xf & (fat >> 21);
        uint32_t day = 0x1f & (fat >> 16);
        uint32_t hour = 0x1f & (fat >> 11);
        uint32_t minute = 0x3f & (fat >> 5);
        uint32_t second = (0x1f & fat) * 2;

#if defined(__GNUC__)
        std::tm tm{};
        tm.tm_year = (int) year - 1900;
        tm.tm_mon = (int) month - 1;
        tm.tm_mday = (int) day;
        tm.tm_hour = (int) hour;
        tm.tm_min = (int) minute;
        tm.tm_sec = (int) second;
        tm.tm_isdst = 0;

        std::time_t t = to_utc_time(tm);

        if (t == (std::time_t) -1) { return std::nullopt; }

        return std::chrono::system_clock::from_time_t(t);
#else
        std::chrono::year_month_day ymd = std::chrono::year(year) / std::chrono::month(month) / std::chrono::day(day);
        if (!ymd.ok()) { return std::nullopt; }
        return std::chrono::sys_days(ymd) + std::chrono::hours(hour) + std::chrono::minutes(minute) +
               std::chrono::seconds(second);
#endif
    }


}
