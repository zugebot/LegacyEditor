#pragma once

#include <chrono>
#include <iostream>
#include <optional>

#include "LegacyEditor/LCE/FileInfo.hpp"
#include "LegacyEditor/utils/managers/dataInManager.hpp"
#include "LegacyEditor/utils/managers/dataOutManager.hpp"


struct StfsFileEntry {
    uint32_t entryIndex{};
    std::string name;
    uint8_t nameLen{};
    uint8_t flags{};
    int blocksForFile{};
    int startingBlockNum{};
    uint16_t pathIndicator{};
    uint32_t fileSize{};
    uint32_t createdTimeStamp{};
    uint32_t accessTimeStamp{};
    uint32_t fileEntryAddress{};
    std::vector<int> blockChain;
};


struct StfsFileListing {
    std::vector<StfsFileEntry> fileEntries;
    std::vector<StfsFileListing> folderEntries;
    StfsFileEntry folder;
};


struct StfsVD {
    uint8_t size;
    //uint8_t reserved;
    uint8_t blockSeparation;
    uint16_t fileTableBlockCount;
    int fileTableBlockNum;
    //uint8_t topHashTableHash[0x14];
    uint32_t allocatedBlockCount;
    uint32_t unallocatedBlockCount;

    void readStfsVD(DataInManager& input) {
        this->size = input.readByte();
        input.readByte();// reserved
        this->blockSeparation = input.readByte();
        input.setLittleEndian();
        this->fileTableBlockCount = input.readShort();
        this->fileTableBlockNum = input.readInt24();
        input.incrementPointer(0x14);// skip the hash
        input.setBigEndian();
        this->allocatedBlockCount = input.readInt();
        this->unallocatedBlockCount = input.readInt();
    }
};


#pragma pack(push, 1)
struct HashEntry {
    uint8_t blockHash[0x14];
    uint8_t status;
    uint32_t nextBlock;
};
#pragma pack(pop)


struct HashTable {
    uint8_t level;
    uint32_t trueBlockNumber;
    uint32_t entryCount;
    HashEntry entries[0xAA];
    uint32_t addressInFile;
};


class BINHeader {
public:
    uint32_t headerSize{};
    StfsVD stfsVD{};
    std::wstring displayName;
    DataInManager thumbnailImage = DataInManager();


    int readHeader(DataInManager& binFile) {

        binFile.seek(0x340);
        this->headerSize = binFile.readInt();

        //content type, 1 is savegame
        if (binFile.readInt() != 1) {
            printf(".bin file is not a savegame, exiting\n");
            return 0;
        }

        //file system
        binFile.seek(0x3A9);
        if (binFile.readInt()) {
            printf(".bin file is not in STFS format, exiting\n");
            return 0;
        }

        binFile.seek(0x0379);
        this->stfsVD.readStfsVD(binFile);
        binFile.seek(0x0411);

        //readBytes the savegame name
        displayName = binFile.readWString();

        //skip all the irrelevant data to extract the savegame
        binFile.seek(0x1712);
        //get thumbnail image, if not present, use the title one if present
        uint8_t* thumbnail = nullptr;
        uint32_t thumbnailImageSize = binFile.readInt();
        if (thumbnailImageSize) {
            binFile.incrementPointer(4);//readBytes the other size but it will not be used
            uint8_t* thumbnailImageData = binFile.readBytes(thumbnailImageSize);
            this->thumbnailImage = DataInManager(thumbnailImageData, thumbnailImageSize);
        } else {
            uint32_t titleThumbnailImageSize = binFile.readInt();
            if (titleThumbnailImageSize) {
                binFile.seek(0x571A);
                uint8_t* titleThumbnailImageData = binFile.readBytes(thumbnailImageSize);
                this->thumbnailImage = DataInManager(titleThumbnailImageData, titleThumbnailImageSize);
            }
        }
        return 1;
    }
};


/// extract a file (by FileEntry) to a designated file path
class StfsPackage {
public:
    explicit StfsPackage(DataInManager& input) : data(input) {}

    StfsFileListing GetFileListing() { return fileListing; }

    void Extract(StfsFileEntry* entry, DataOutManager& out) {
        if (entry->nameLen == 0) { entry->name = "default"; }

        // get the file size that we are extracting
        uint32_t fileSize = entry->fileSize;
        if (fileSize == 0) { return; }

        // check if all the blocks are consecutive
        if (entry->flags & 1) {
            // allocate 0xAA blocks of memory, for maximum efficiency, yo
            auto* buffer = new uint8_t[0xAA000];

            // seek to the beginning of the file
            uint32_t startAddress = BlockToAddress(entry->startingBlockNum);
            data.seek(startAddress);

            // calculateOffset the number of blocks to readBytes before we hit a table
            uint32_t blockCount = (ComputeLevel0BackingHashBlockNumber(entry->startingBlockNum) + blockStep[0]) -
                                  ((startAddress - firstHashTableAddress) >> 0xC);

            // pick up the change at the beginning, until we hit a hash table
            if ((uint32_t) entry->blocksForFile <= blockCount) {
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

            // extract the blocks in between the tables
            uint32_t tempSize = (entry->fileSize - (blockCount << 0xC));
            while (tempSize >= 0xAA000) {
                // skip past the hash table(s)
                uint32_t currentPos = data.getPosition();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // readBytes in the 0xAA blocks between the tables
                data.readOntoData(0xAA000, buffer);

                // Write the bytes to the out file
                out.write(buffer, 0xAA000);

                tempSize -= 0xAA000;
                blockCount += 0xAA;
            }

            // pick up the change at the end
            if (tempSize != 0) {
                // skip past the hash table(s)
                uint32_t currentPos = data.getPosition();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // readBytes in the extra crap
                data.readOntoData(tempSize, buffer);

                // Write it to the out file
                out.write(buffer, tempSize);
            }

            // free the temp buffer
            delete[] buffer;
        } else {
            // generate the blockchain which we have to extract
            uint32_t fullReadCounts = fileSize / 0x1000;

            fileSize -= (fullReadCounts * 0x1000);

            uint32_t block = entry->startingBlockNum;

            // allocate data for the blocks
            uint8_t block_data[0x1000];

            // readBytes all the full blocks the file allocates
            for (uint32_t i = 0; i < fullReadCounts; i++) {
                ExtractBlock(block, block_data);
                out.write(block_data, 0x1000);

                block = GetBlockHashEntry(block).nextBlock;
            }

            // readBytes the remaining data
            if (fileSize != 0) {
                ExtractBlock(block, block_data, fileSize);
                out.write(block_data, (int) fileSize);
            }
        }
    }

    /// convert a block into an address in the file
    uint32_t BlockToAddress(uint32_t blockNum) {
        // check for invalid block number
        if (blockNum >= 0xFFFFFF) throw std::runtime_error("STFS: Block number must be less than 0xFFFFFF.\n");
        return (ComputeBackingDataBlockNumber(blockNum) << 0x0C) + firstHashTableAddress;
    }

    /// get the address of a hash for a data block
    uint32_t GetHashAddressOfBlock(uint32_t blockNum) {
        if (blockNum >= metaData.stfsVD.allocatedBlockCount)
            throw std::runtime_error("STFS: Reference to illegal block number.\n");

        uint32_t hashAddr = (ComputeLevel0BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress;
        hashAddr += (blockNum % 0xAA) * 0x18;

        switch (topLevel) {
            case 0:
                hashAddr += ((metaData.stfsVD.blockSeparation & 2) << 0xB);
                break;
            case 1:
                hashAddr += ((topTable.entries[blockNum / 0xAA].status & 0x40) << 6);
                break;
            case 2:
                uint32_t level1Off = ((topTable.entries[blockNum / 0x70E4].status & 0x40) << 6);
                uint32_t pos =
                        ((ComputeLevel1BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress + level1Off) +
                        ((blockNum % 0xAA) * 0x18);
                data.seek(pos + 0x14);
                hashAddr += ((data.readByte() & 0x40) << 6);
                break;
        }
        return hashAddr;
    }

    ~StfsPackage() = default;

    BINHeader GetMetaData() { return metaData; }

private:
    BINHeader metaData;

    StfsFileListing fileListing;
    //StfsFileListing writtenToFile;
    DataInManager& data;

    uint8_t packageSex{};//0 female, 1 male
    uint32_t blockStep[2]{};
    uint32_t firstHashTableAddress{};
    uint8_t topLevel{};
    HashTable topTable{};
    uint32_t tablesPerLevel[3]{};

    /// readBytes the file listing from the file
    void ReadFileListing() {
        fileListing.fileEntries.clear();
        fileListing.folderEntries.clear();

        // set up the entry for the blockchain
        StfsFileEntry entry;
        entry.startingBlockNum = metaData.stfsVD.fileTableBlockNum;
        entry.fileSize = (metaData.stfsVD.fileTableBlockCount * 0x1000);

        // generate a blockchain for the full file listing
        uint32_t block = entry.startingBlockNum;

        StfsFileListing fl;
        uint32_t currentAddr;
        for (uint32_t x = 0; x < metaData.stfsVD.fileTableBlockCount; x++) {
            currentAddr = BlockToAddress(block);
            data.seek(currentAddr);

            for (uint32_t i = 0; i < 0x40; i++) {
                StfsFileEntry fe;

                // set the current position
                fe.fileEntryAddress = currentAddr + (i * 0x40);

                // calculateOffset the entry index (in the file listing)
                fe.entryIndex = (x * 0x40) + i;

                // readBytes the name, if the length is 0 then break
                fe.name = data.readString(0x28);

                // readBytes the name length
                fe.nameLen = data.readByte();
                if ((fe.nameLen & 0x3F) == 0) {
                    data.seek(currentAddr + ((i + 1) * 0x40));
                    continue;
                } else if (fe.name.length() == 0) {
                    break;
                }

                // check for a mismatch in the total allocated blocks for the file
                fe.blocksForFile = data.readInt24(true);
                data.incrementPointer(3);

                // readBytes more information
                fe.startingBlockNum = data.readInt24(true);
                fe.pathIndicator = data.readShort();
                fe.fileSize = data.readInt();
                fe.createdTimeStamp = data.readInt();
                fe.accessTimeStamp = data.readInt();

                // get the flags
                fe.flags = fe.nameLen >> 6;

                // bits 6 and 7 are flags, clear them
                fe.nameLen &= 0x3F;

                fl.fileEntries.push_back(fe);
            }

            block = GetBlockHashEntry(block).nextBlock;
        }

        // sort the file listing
        AddToListing(&fl, &fileListing);
        //writtenToFile = fileListing;
    }

    /// extract a block's data
    void ExtractBlock(uint32_t blockNum, uint8_t* inputData, uint32_t length = 0x1000) {
        if (blockNum >= metaData.stfsVD.allocatedBlockCount)
            throw std::runtime_error("STFS: Reference to illegal block number.\n");

        // check for an invalid block length
        if (length > 0x1000) throw std::runtime_error("STFS: length cannot be greater 0x1000.\n");

        // go to the block's position
        data.seek(BlockToAddress(blockNum));

        // readBytes the data, and return
        data.readOntoData(length, inputData);
    }

    /// convert a block number into a true block number, where the first block is the first hash table
    ND u32 ComputeBackingDataBlockNumber(uint32_t blockNum) const {
        u32 toReturn = (((blockNum + 0xAA) / 0xAA) << packageSex) + blockNum;
        if (blockNum < 0xAA) return toReturn;
        else if (blockNum < 0x70E4)
            return toReturn + (((blockNum + 0x70E4) / 0x70E4) << packageSex);
        else
            return (1 << packageSex) + (toReturn + (((blockNum + 0x70E4) / 0x70E4) << packageSex));
    }

    /// get a block's hash entry
    HashEntry GetBlockHashEntry(uint32_t blockNum) {
        if (blockNum >= metaData.stfsVD.allocatedBlockCount) {
            throw std::runtime_error("STFS: Reference to illegal block number.\n");
        }

        // go to the position of the hash address
        data.seek(GetHashAddressOfBlock(blockNum));

        // readBytes the hash entry
        HashEntry he{};
        data.readOntoData(0x14, he.blockHash);
        he.status = data.readByte();
        he.nextBlock = data.readInt24();

        return he;
    }

    /// get the true block number for the hash table that hashes the block at the level passed in
    uint32_t ComputeLevelNBackingHashBlockNumber(uint32_t blockNum, uint8_t level) {
        switch (level) {
            case 0:
                return ComputeLevel0BackingHashBlockNumber(blockNum);

            case 1:
                return ComputeLevel1BackingHashBlockNumber(blockNum);

            case 2:
                return ComputeLevel2BackingHashBlockNumber(blockNum);

            default:
                throw std::runtime_error("STFS: Invalid level.\n");
        }
    }

    /// get the true block number for the hash table that hashes the block at level 0
    uint32_t ComputeLevel0BackingHashBlockNumber(uint32_t blockNum) {
        if (blockNum < 0xAA) return 0;

        uint32_t num = (blockNum / 0xAA) * blockStep[0];
        num += ((blockNum / 0x70E4) + 1) << ((uint8_t) packageSex);

        if (blockNum / 0x70E4 == 0) return num;

        return num + (1 << (uint8_t) packageSex);
    }

    /// get the true block number for the hash table that hashes the block at level 1 (female)
    uint32_t ComputeLevel1BackingHashBlockNumber(uint32_t blockNum) {
        if (blockNum < 0x70E4) return blockStep[0];
        return (1 << (uint8_t) packageSex) + (blockNum / 0x70E4) * blockStep[1];
    }

    /// get the true block number for the hash table that hashes the block at level 2
    uint32_t ComputeLevel2BackingHashBlockNumber(uint32_t blockNum) { return blockStep[1]; }

    /// add the file entry to the file listing
    void AddToListing(StfsFileListing* fullListing, StfsFileListing* out) {
        for (auto &fileEntry: fullListing->fileEntries) {
            // check if the file is a directory
            bool isDirectory = (fileEntry.flags & 2);

            // make sure the file belongs to the current folder
            if (fileEntry.pathIndicator == out->folder.entryIndex) {
                // add it if it's a file
                if (!isDirectory) out->fileEntries.push_back(fileEntry);
                // if it's a directory and not the current directory, then add it
                else if (isDirectory && fileEntry.entryIndex != out->folder.entryIndex) {
                    StfsFileListing fl;
                    fl.folder = fileEntry;
                    out->folderEntries.push_back(fl);
                }
            }
        }

        // for every folder added, add the files to them
        for (auto &folderEntry: out->folderEntries) {
            AddToListing(fullListing, &folderEntry);
        }
    }

    /// calculateOffset the level of the topmost hash table
    int CalculateTopLevel() const {
        if (metaData.stfsVD.allocatedBlockCount <= 0xAA) return 0;
        else if (metaData.stfsVD.allocatedBlockCount <= 0x70E4)
            return 1;
        else if (metaData.stfsVD.allocatedBlockCount <= 0x4AF768)
            return 2;
        else
            throw std::runtime_error("STFS: Invalid number of allocated blocks.\n");
    }

    /// get the number of bytes to skip over the hash table
    uint32_t GetHashTableSkipSize(uint32_t tableAddress) {
        // convert the address to a true block number
        uint32_t trueBlockNumber = (tableAddress - firstHashTableAddress) >> 0xC;

        // check if it's the first hash table
        if (trueBlockNumber == 0) return (0x1000 << packageSex);

        // check if it's the level 2 table, or above
        if (trueBlockNumber == blockStep[1]) return (0x3000 << packageSex);
        else if (trueBlockNumber > blockStep[1])
            trueBlockNumber -= (blockStep[1] + (1 << packageSex));

        // check if it's at a level 1 table
        if (trueBlockNumber == blockStep[0] ||
            trueBlockNumber % blockStep[1] == 0)
            return (0x2000 << packageSex);

        // otherwise, assume it's at a level 0 table
        return (0x1000 << packageSex);
    }

public:
    /// parse the file
    void Parse() {
        BINHeader header;
        int result = header.readHeader(data);
        if (!result) {
            //free(inputData);
            return;//FileInfo();
        }
        metaData = header;
        packageSex = ((~metaData.stfsVD.blockSeparation) & 1);

        if (packageSex == 0) { //female
            blockStep[0] = 0xAB;
            blockStep[1] = 0x718F;
        } else { //male
            blockStep[0] = 0xAC;
            blockStep[1] = 0x723A;
        }

        // address of the first hash table in the package, comes right after the header
        firstHashTableAddress = (metaData.headerSize + 0x0FFF) & 0xFFFFF000;

        // calculateOffset the number of tables per level
        tablesPerLevel[0] = (metaData.stfsVD.allocatedBlockCount / 0xAA) +
                            ((metaData.stfsVD.allocatedBlockCount % 0xAA != 0) ? 1 : 0);
        tablesPerLevel[1] = (tablesPerLevel[0] / 0xAA) +
                            ((tablesPerLevel[0] % 0xAA != 0 &&
                              metaData.stfsVD.allocatedBlockCount > 0xAA) ? 1 : 0);
        tablesPerLevel[2] = (tablesPerLevel[1] / 0xAA) +
                            ((tablesPerLevel[1] % 0xAA != 0 &&
                              metaData.stfsVD.allocatedBlockCount > 0x70E4) ? 1 : 0);

        // calculateOffset the level of the top table
        topLevel = CalculateTopLevel();

        // readBytes in the top hash table
        topTable.trueBlockNumber = ComputeLevelNBackingHashBlockNumber(0, topLevel);
        topTable.level = topLevel;

        uint32_t baseAddress = (topTable.trueBlockNumber << 0xC) + firstHashTableAddress;
        topTable.addressInFile = baseAddress + ((metaData.stfsVD.blockSeparation & 2) << 0xB);
        data.seek(topTable.addressInFile);

        uint32_t dataBlocksPerHashTreeLevel[3] = {1, 0xAA, 0x70E4};

        // load the information
        topTable.entryCount = metaData.stfsVD.allocatedBlockCount / dataBlocksPerHashTreeLevel[topLevel];
        if (metaData.stfsVD.allocatedBlockCount > 0x70E4 &&
            (metaData.stfsVD.allocatedBlockCount % 0x70E4 != 0))
            topTable.entryCount++;
        else if (metaData.stfsVD.allocatedBlockCount > 0xAA
                 && (metaData.stfsVD.allocatedBlockCount % 0xAA != 0))
            topTable.entryCount++;

        for (uint32_t i = 0; i < topTable.entryCount; i++) {
            data.readOntoData(0x14, topTable.entries[i].blockHash);
            topTable.entries[i].status = data.readByte();
            topTable.entries[i].nextBlock = data.readInt24();
        }

        // set default values for the root of the file listing
        StfsFileEntry fe;
        fe.pathIndicator = 0xFFFF;
        fe.name = "Root";
        fe.entryIndex = 0xFFFF;
        fileListing.folder = fe;

        ReadFileListing();
    }
};


struct TextChunk {
    std::string keyword;
    std::string text;
};


static StfsFileEntry* FindSavegameFileEntry(StfsFileListing& listing) {
    for (StfsFileEntry& file: listing.fileEntries) {
        if (file.name == "savegame.dat") { return &file; }
    }
    for (StfsFileListing& folder: listing.folderEntries) {
        if (StfsFileEntry* entry = FindSavegameFileEntry(folder); entry) { return entry; }
    }
    return nullptr;
}


static uint32_t c2n(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}


static int64_t stringToHex(const std::string& str) {
    int64_t result = 0;
    size_t i = 0;
    int stringSize = (int)str.size() - 1;//terminating value doesn't count
    for (; i < stringSize; i++) { result = result * 16 + c2n(str[i]); }
    return result;
}


static int64_t stringToInt64(const std::string& str) {
    int64_t result = 0;
    int sign = 1;
    size_t i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    int stringSize = (int)str.size() - 1;//terminating value doesn't count
    for (; i < stringSize; i++) { result = result * 10 + (str[i] - '0'); }

    return result * sign;
}


static WorldOptions getTagsInImage(DataInManager& image) {
    WorldOptions options;
    uint8_t* PNGHeader = image.readBytes(8);
    if (memcmp(PNGHeader, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) != 0) {
        printf("File in thumbnail block is not PNG header, the first 8 bytes are:\n");
        for (size_t i = 0; i < 8; i++) { std::cout << std::hex << (int) (PNGHeader[i]) << " "; }
        std::cout << std::endl;
    }
    free(PNGHeader);

    std::vector<TextChunk> chunks;

    while (true) {
        // Check if we've reached the end of the file
        if (image.isEndOfData()) break;
        // Read chunk length
        uint32_t length = image.readInt();

        // Read chunk type
        char* type = (char*) image.readBytes(4);
        //check if end
        if (std::string(type, 4) == "IEND") {
            free(type);
            break;
        }
        // Check if the chunk is a text chunk
        if (std::string(type, 4) != "tEXt") {
            free(type);
            image.incrementPointer(length + 4);//the extra 4 is the crc
            continue;
        }
        free(type);
        // Read keyword
        int64_t chunkLength = length;
        while (chunkLength > 0) {
            std::string keyword;
            u8 c = 0;
            //remove all null bytes in between
            while (c == 0 && chunkLength > 0) {
                chunkLength--;
                c = image.readByte();
                if (image.isEndOfData()) { break; }
            }
            chunkLength++;//chunkLength is added because the next value was tested and subtracted even on the non-null byte
            image.incrementPointer(-1);
            while (c != 0 && chunkLength > 0) {
                c = image.readByte();
                keyword += (char)c;
                chunkLength--;
                if (image.isEndOfData()) { break; }
            }
            //remove all null bytes in between
            while (c == 0 && chunkLength > 0) {
                chunkLength--;
                c = image.readByte();
                if (image.isEndOfData()) { break; }
            }
            chunkLength++;//chunkLength is added because the next value was tested and subtracted even on the non-null byte
            image.incrementPointer(-1);
            std::string text;
            while (c != 0 && chunkLength > 0) {
                c = image.readByte();
                text += (char)c;
                chunkLength--;
                if (image.isEndOfData()) { break; }
            }
            chunks.push_back({keyword, text});
            if (image.isEndOfData()) { break; }
        }

        // Read chunk CRC
        image.readInt();
    }


    //get keys and store them
    for (const TextChunk& chunk: chunks) {
        const char* keyword = chunk.keyword.c_str();
        if (keyword == std::string("4J_SEED")) {
            options.displaySeed = stringToInt64(chunk.text);
        } else if (keyword == std::string("4J_#LOADS")) {
            options.numLoads = stringToHex(chunk.text);
        } else if (keyword == std::string("4J_HOSTOPTIONS")) {
            options.hostOptions = stringToHex(chunk.text);
        } else if (keyword == std::string("4J_TEXTUREPACK")) {
            options.texturePack = stringToHex(chunk.text);
        } else if (keyword == std::string("4J_EXTRADATA")) {
            options.extraData = stringToHex(chunk.text);
        } else if (keyword == std::string("4J_EXPLOREDCHUNKS")) {
            options.numExploredChunks = stringToHex(chunk.text);
        } else if (keyword == std::string("4J_BASESAVENAME")) {
            options.baseSaveName = chunk.text;
        }
    }
    return options;
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

    std::time_t t = _mkgmtime(&tm);

    if (t == (std::time_t) -1) { return std::nullopt; }
    return std::chrono::system_clock::from_time_t(t);
#else
    std::chrono::year_month_day ymd = std::chrono::year(year) / std::chrono::month(month) / std::chrono::day(day);
    if (!ymd.ok()) { return std::nullopt; }
    return std::chrono::sys_days(ymd) + std::chrono::hours(hour) + std::chrono::minutes(minute) +
           std::chrono::seconds(second);
#endif
}

static FileInfo extractSaveGameDat(u8* inputData, i64 inputSize) {
    DataInManager binFile(inputData, inputSize);
    StfsPackage stfsInfo(binFile);
    stfsInfo.Parse();
    StfsFileListing listing = stfsInfo.GetFileListing();
    StfsFileEntry* entry = FindSavegameFileEntry(listing);
    if (!entry) { return {}; }

    // TODO IMPORTANT: find upper range of this so it can use a buffer
    DataOutManager out(23456789);

    stfsInfo.Extract(entry, out);
    out.size = out.ptr - out.getStartPtr();

    FileInfo savegame;
    savegame.createdTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
    BINHeader meta = stfsInfo.GetMetaData();
    if (meta.thumbnailImage.size) {
        savegame.thumbnailImage = meta.thumbnailImage;
    }

    int savefileSize = (int) out.getSize();
    if (savefileSize) {
        u8* savefile = (u8*) malloc(savefileSize);
        memcpy(savefile, out.getStartPtr(), savefileSize);
        savegame.saveFileData = DataInManager(savefile, savefileSize);
    }

    savegame.saveName = stfsInfo.GetMetaData().displayName;
    savegame.options = getTagsInImage(savegame.thumbnailImage);
    free(inputData);
    return savegame;
}
