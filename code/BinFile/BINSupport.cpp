#include "BINSupport.hpp"

#include "include/lce/processor.hpp"


namespace editor {


    void StfsVD::readStfsVD(DataReader& input) {
        size = input.read<u8>();
        input.read<u8>(); // reserved
        blockSeparation = input.read<u8>();
        input.setEndian(Endian::Little);
        fileTableBlockCount = input.read<u16>();
        fileTableBlockNum = input.readInt24();
        input.skip<0x14>(); // skip the hash
        input.setEndian(Endian::Big);
        allocBlockCount = input.read<u32>();
        unallocatedBlockCount = input.read<u32>();
    }


    int BINHeader::readHeader(DataReader& binFile) {
        binFile.seek(0x340U);
        headerSize = binFile.read<u32>();

        //content type, 1 is savegame
        if (binFile.read<u32>() != 1) {
            printf(".bin file is not a savegame, exiting\n");
            return 0;
        }

        //file system
        binFile.seek(0x3A9U);
        if (binFile.read<u32>()) {
            printf(".bin file is not in STFS format, exiting\n");
            return 0;
        }

        binFile.seek(0x0379U);
        stfsVD.readStfsVD(binFile);
        binFile.seek(0x0411U);

        // read the savegame name
        displayName = binFile.readNullTerminatedWString();

        // skip all the irrelevant data to extract the savegame
        binFile.seek(0x1712U);
        // get thumbnail image, if not present, use the title one if present
        if (c_u32 thumbnailImageSize = binFile.read<u32>()) {
            binFile.skip<4>(); // read the other size but it will not be used
            thumbnailImage = binFile.readBuffer(thumbnailImageSize);
        } else {
            if (c_u32 titleThumbImageSize = binFile.read<u32>()) {
                binFile.seek(0x571AU);

                thumbnailImage = binFile.readBuffer(thumbnailImageSize);
            }
        }
        return 1;
    }

    void write(std::vector<u8>& out, u8* buf, c_u32 amount) {
        out.insert(out.end(), buf, buf + amount);
    }


    Buffer StfsPackage::extractFile(StfsFileEntry* entry) {
        if (entry->nameLen == 0) { entry->name = "default"; }
        std::vector<u8> out2;


        // get the file size that we are extracting
        u32 fileSize = entry->fileSize;
        if (fileSize == 0) { return {}; }

        // check if all the blocks are consecutive
        if (entry->flags & 1) {
            // allocate 0xAA blocks of memory, for maximum efficiency, yo
            auto* buffer = new u8[0xAA000];

            // seek to the beginning of the file
            c_u32 startAddress = blockToAddress(entry->startingBlockNum); // blockToAddress(entry->startingBlockNum);
            data.seek(startAddress);

            // calculateOffset the number of blocks to read before we hit a table
            u32 blockCount = (computeLevel0BackingHashBlockNumber(entry->startingBlockNum) + blockStep[0]) -
                             ((startAddress - firstHashTableAddress) >> 0xC);

            // pick up the change at the beginning, until we hit a hash table
            if ((u32) entry->blocksForFile <= blockCount) {
                data.readBytes(entry->fileSize, buffer);
                write(out2, buffer, entry->fileSize);
                // out.writeBytes(buffer, entry->fileSize);

                // free the temp buffer
                delete[] buffer;
                return {};
            } else {
                c_u32 amount = blockCount << 0xC;
                data.readBytes(amount, buffer);
                write(out2, buffer, amount);
                // out.writeBytes(buffer, amount);
            }

            // extract the blocks in between the tables
            u32 tempSize = (entry->fileSize - (blockCount << 0xC));
            while (tempSize >= 0xAA000) {
                // skip past the hash table(s)
                u32 currentPos = data.tell();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // read in the 0xAA blocks between the tables
                data.readBytes(0xAA000, buffer);
                write(out2, buffer, 0xAA000);
                // out.writeBytes(buffer, 0xAA000);

                tempSize -= 0xAA000;
                blockCount += 0xAA;
            }

            // pick up the change at the end
            if (tempSize != 0) {
                // skip past the hash table(s)
                c_u32 currentPos = data.tell();
                data.seek(currentPos + GetHashTableSkipSize(currentPos));

                // read in the extra crap
                data.readBytes(tempSize, buffer);
                write(out2, buffer, tempSize);
                // out.writeBytes(buffer, tempSize);
            }

            // free the temp buffer
            delete[] buffer;
        } else {
            // generate the blockchain which we have to extract
            c_u32 fullReadCounts = fileSize / 0x1000;

            fileSize -= (fullReadCounts * 0x1000);

            u32 block = entry->startingBlockNum;

            // allocate data for the blocks
            u8 buffer[0x1000];

            // read all the full blocks the file allocates
            for (u32 i = 0; i < fullReadCounts; i++) {
                extractBlock(block, buffer);
                write(out2, buffer, 0x1000);
                block = getBlockHashEntry(block).nextBlock;
            }

            // read the remaining data
            if (fileSize != 0) {
                extractBlock(block, buffer, fileSize);
                write(out2, buffer, (int) fileSize);
            }
        }
        Buffer ret;
        ret.allocate(out2.size());
        std::memcpy(ret.data(), out2.data(), out2.size());
        return ret;
    }


    /// convert a block into an address in the file
    ND u32 StfsPackage::blockToAddress(c_u32 blockNum) const {
        if (blockNum >= 0xFFFFFF) throw std::runtime_error("STFS: Block number must be less than 0xFFFFFF.\n");
        return (computeBackingDataBlockNumber(blockNum) << 0x0C) + firstHashTableAddress;
    }


    /// get the address of a hash for a data block
    ND u32 StfsPackage::getHashAddressOfBlock(u32 blockNum) {
        if (blockNum >= metaData.stfsVD.allocBlockCount)
            throw std::runtime_error("STFS: Reference to illegal block number.\n");

        u32 hashAddr = (computeLevel0BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress;
        hashAddr += (blockNum % 0xAA) * 0x18;

        switch (topLevel) {
            case 0:
                hashAddr += ((metaData.stfsVD.blockSeparation & 2) << 0xB);
                break;
            case 1:
                hashAddr += ((topTable.entries[blockNum / 0xAA].status & 0x40) << 6);
                break;
            case 2:
                c_u32 level1Off = ((topTable.entries[blockNum / 0x70E4].status & 0x40) << 6);
                c_u32 pos =
                        ((computeLevel1BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress + level1Off) +
                        ((blockNum % 0xAA) * 0x18);
                data.seek(pos + 0x14);
                hashAddr += (data.read<u8>() & 0x40) << 6;
                break;
        }
        return hashAddr;
    }


    /// readDataBlocks the file listing from the file
    void StfsPackage::readFileListing() {
        fileListing.fileEntries.clear();
        fileListing.folderEntries.clear();

        // set up the entry for the blockchain
        StfsFileEntry entry;
        entry.startingBlockNum = metaData.stfsVD.fileTableBlockNum;
        entry.fileSize = (metaData.stfsVD.fileTableBlockCount * 0x1000);

        // generate a blockchain for the full file listing
        u32 block = entry.startingBlockNum;

        StfsFileListing fl;
        for (u32 x = 0; x < metaData.stfsVD.fileTableBlockCount; x++) {
            c_u32 currentAddr = blockToAddress(block);
            data.seek(currentAddr);

            for (u32 i = 0; i < 0x40; i++) {
                StfsFileEntry fe;
                fe.fileEntryAddress = currentAddr + (i * 0x40); // set the current position
                fe.entryIndex = (x * 0x40) + i;  // calculateOffset the entry index (in the file listing)
                fe.name = data.readString(0x28); // read the name, if the length is 0 then break
                fe.nameLen = data.read<u8>();    // read the name length

                if ((fe.nameLen & 0x3F) == 0) {
                    data.seek(currentAddr + ((i + 1) * 0x40));
                    continue;
                } else if (fe.name.empty()) {
                    break;
                }

                // check for a mismatch in the total allocated blocks for the file
                data.setEndian(Endian::Little);
                fe.blocksForFile = data.readInt24();
                data.skip<3>();

                // read more information
                fe.startingBlockNum = data.readInt24();
                data.setEndian(Endian::Big);
                fe.pathIndicator = data.read<u16>();
                fe.fileSize = data.read<u32>();
                fe.createdTimeStamp = data.read<u32>();
                fe.accessTimeStamp = data.read<u32>();

                // get the flags
                fe.flags = fe.nameLen >> 6;

                // bits 6 and 7 are flags, clear them
                fe.nameLen &= 0x3F;

                fl.fileEntries.push_back(fe);
            }

            block = getBlockHashEntry(block).nextBlock;
        }

        // sort the file listing
        addToListing(&fl, &fileListing);
        //writtenToFile = fileListing;
    }


    /// extractFile a block's data
    void StfsPackage::extractBlock(c_u32 blockNum, u8* inputData, c_u32 length) const {
        if (blockNum >= metaData.stfsVD.allocBlockCount)
            throw std::runtime_error("STFS: Reference to illegal block number.\n");

        // check for an invalid block length
        if (length > 0x1000) throw std::runtime_error("STFS: length cannot be greater 0x1000.\n");

        // go to the block's position
        data.seek(blockToAddress(blockNum));

        // read the data, and return
        data.readBytes(length, inputData);
    }


    /// convert a block number into a true block number, where the first block is the first hash table
    ND u32 StfsPackage::computeBackingDataBlockNumber(u32 blockNum) const {
        u32 toReturn = (((blockNum + 0xAA) / 0xAA) << packageSex) + blockNum;
        if (blockNum < 0xAA) return toReturn;
        else if (blockNum < 0x70E4)
            return toReturn + (((blockNum + 0x70E4) / 0x70E4) << packageSex);
        else
            return (1 << packageSex) + (toReturn + (((blockNum + 0x70E4) / 0x70E4) << packageSex));
    }


    /// get a block's hash entry
    HashEntry StfsPackage::getBlockHashEntry(c_u32 blockNum) {
        if (blockNum >= metaData.stfsVD.allocBlockCount) {
            throw std::runtime_error("STFS: Reference to illegal block number.\n");
        }

        // go to the position of the hash address
        data.seek(getHashAddressOfBlock(blockNum));

        // read the hash entry
        HashEntry he{};
        data.readBytes(0x14, he.blockHash);
        he.status = data.read<u8>();
        he.nextBlock = data.readInt24();

        return he;
    }


    /// get the true block number for the hash table that hashes the block at the level passed in
    ND u32 StfsPackage::computeLevelNBackingHashBlockNumber(u32 blockNum, u8 level) {
        switch (level) {
            case 0:
                return computeLevel0BackingHashBlockNumber(blockNum);
            case 1:
                return computeLevel1BackingHashBlockNumber(blockNum);
            case 2:
                return computeLevel2BackingHashBlockNumber();
            default:
                throw std::runtime_error("STFS: Invalid level.\n");
        }
    }


    /// get the true block number for the hash table that hashes the block at level 0
    ND u32 StfsPackage::computeLevel0BackingHashBlockNumber(u32 blockNum) {
        if (blockNum < 0xAA) return 0;
        u32 num = (blockNum / 0xAA) * blockStep[0];
        num += ((blockNum / 0x70E4) + 1) << ((u8) packageSex);
        if (blockNum / 0x70E4 == 0) return num;
        return num + (1 << (u8) packageSex);
    }


    /// get the true block number for the hash table that hashes the block at level 1 (female)
    ND u32 StfsPackage::computeLevel1BackingHashBlockNumber(u32 blockNum) {
        if (blockNum < 0x70E4) return blockStep[0];
        return (1 << (u8) packageSex) + (blockNum / 0x70E4) * blockStep[1];
    }

    /// get the true block number for the hash table that hashes the block at level 2
    ND u32 StfsPackage::computeLevel2BackingHashBlockNumber() {
        return blockStep[1];
    }


    /// add the file entry to the file listing
    void StfsPackage::addToListing(StfsFileListing* fullListing, StfsFileListing* out) {
        for (auto& fileEntry: fullListing->fileEntries) {
            // check if the file is a directory
            bool isDirectory = (fileEntry.flags & 2);

            // make sure the file belongs to the current folder
            if (fileEntry.pathIndicator == out->folder.entryIndex) {
                // add it if it's a file
                if (!isDirectory) out->fileEntries.push_back(fileEntry);
                // if it's a directory and not the current directory, then add it
                else if (fileEntry.entryIndex != out->folder.entryIndex) {
                    StfsFileListing fl;
                    fl.folder = fileEntry;
                    out->folderEntries.push_back(fl);
                }
            }
        }

        // for every folder added, add the files to them
        for (auto& folderEntry: out->folderEntries) {
            addToListing(fullListing, &folderEntry);
        }
    }


    /// calculateOffset the level of the topmost hash table
    ND int StfsPackage::calculateTopLevel() const {
        if (metaData.stfsVD.allocBlockCount <= 0xAA) return 0;
        else if (metaData.stfsVD.allocBlockCount <= 0x70E4)
            return 1;
        else if (metaData.stfsVD.allocBlockCount <= 0x4AF768)
            return 2;
        else
            throw std::runtime_error("STFS: Invalid number of allocated blocks.\n");
    }


    /// get the number of bytes to skip over the hash table
    ND u32 StfsPackage::GetHashTableSkipSize(u32 tableAddress) {
        // convert the address to a true block number
        u32 trueBlockNumber = (tableAddress - firstHashTableAddress) >> 0xC;

        // check if it's the first hash table
        if (trueBlockNumber == 0) return (0x1000 << packageSex);

        // check if it's the level 2 table, or above
        if (trueBlockNumber == blockStep[1]) return (0x3000 << packageSex);
        else if (trueBlockNumber > blockStep[1])
            trueBlockNumber -= (blockStep[1] + (1 << packageSex));

        // check if it's at a level 1 table
        if (trueBlockNumber == blockStep[0] || trueBlockNumber % blockStep[1] == 0) return (0x2000 << packageSex);

        // otherwise, assume it's at a level 0 table
        return (0x1000 << packageSex);
    }


    void StfsPackage::parse() {
        BINHeader header;
        if (c_int result = metaData.readHeader(data); !result) {
            // free(inputData);
            return; // SaveFileInfo();
        }
        metaData = std::move(header);
        packageSex = (~metaData.stfsVD.blockSeparation) & 1;

        if (packageSex == 0) { // female
            blockStep[0] = 0xAB;
            blockStep[1] = 0x718F;
        } else { // male
            blockStep[0] = 0xAC;
            blockStep[1] = 0x723A;
        }

        // address of the first hash table in the package, comes right after the header
        firstHashTableAddress = (metaData.headerSize + 0x0FFF) & 0xFFFFF000;

        // calculateOffset the number of tables per level
        tablesPerLvl[0] = (metaData.stfsVD.allocBlockCount / 0xAA) + ((metaData.stfsVD.allocBlockCount % 0xAA != 0) ? 1 : 0);
        tablesPerLvl[1] = (tablesPerLvl[0] / 0xAA) + ((tablesPerLvl[0] % 0xAA != 0 && metaData.stfsVD.allocBlockCount > 0xAA) ? 1 : 0);
        tablesPerLvl[2] = (tablesPerLvl[1] / 0xAA) + ((tablesPerLvl[1] % 0xAA != 0 && metaData.stfsVD.allocBlockCount > 0x70E4) ? 1 : 0);

        // calculateOffset the level of the top table
        topLevel = static_cast<uint8_t>(calculateTopLevel());

        // read in the top hash table
        topTable.trueBlockNumber = computeLevelNBackingHashBlockNumber(0, topLevel);
        topTable.level = topLevel;

        c_u32 baseAddress = (topTable.trueBlockNumber << 0xC) + firstHashTableAddress;
        topTable.addressInFile = baseAddress + ((metaData.stfsVD.blockSeparation & 2) << 0xB);
        data.seek(topTable.addressInFile);

        c_u32 dataBlocksPerHashTreeLevel[3] = {1, 0xAA, 0x70E4};

        // load the information
        topTable.entryCount = metaData.stfsVD.allocBlockCount / dataBlocksPerHashTreeLevel[topLevel];
        if (metaData.stfsVD.allocBlockCount > 0x70E4 && (metaData.stfsVD.allocBlockCount % 0x70E4 != 0))
            topTable.entryCount++;
        else if (metaData.stfsVD.allocBlockCount > 0xAA && (metaData.stfsVD.allocBlockCount % 0xAA != 0))
            topTable.entryCount++;

        for (u32 i = 0; i < topTable.entryCount; i++) {
            data.readBytes(0x14, topTable.entries[i].blockHash);
            topTable.entries[i].status = data.read<u8>();
            topTable.entries[i].nextBlock = data.readInt24();
        }

        // set default values for the root of the file listing
        StfsFileEntry fe;
        fe.pathIndicator = 0xFFFF;
        fe.name = "Root";
        fe.entryIndex = 0xFFFF;
        fileListing.folder = fe;

        readFileListing();
    }


    StfsFileEntry* findSavegameFileEntry(StfsFileListing& listing) {
        for (StfsFileEntry& file: listing.fileEntries) {
            if (file.name == "savegame.dat") { return &file; }
        }
        for (StfsFileListing& folder: listing.folderEntries) {
            if (StfsFileEntry* entry = findSavegameFileEntry(folder); entry) {
                return entry;
            }
        }
        return nullptr;
    }

}
