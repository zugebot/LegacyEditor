#pragma once

#include "../../common/data/DataReader.hpp"
#include "../../common/data/buffer.hpp"
#include "common/data/ghc/fs_std.hpp"


struct CacheEntry {
    u16 index;
    u32 crc;
    u32 imageSize;
    std::string folderName;
    std::string worldName;
    Buffer imageData;
};

class CacheBinManager {
public:
    bool load(const fs::path& inCachePath);
    bool save(const fs::path& outCachePath);

    void addEntry(const std::string& folderName,
                  const std::string& worldName,
                  const Buffer& imageData,
                  u32 fakeCRC = 0xCAFEBABE);

    ND const std::vector<CacheEntry>& getEntries() const { return entries; }

private:
    std::vector<CacheEntry> entries;
};

bool CacheBinManager::load(const fs::path& inCachePath) {
    entries.clear();

    auto buf = DataReader::readFile(inCachePath);
    DataReader reader(buf.span(), Endian::Little);

    u16 filesFound = reader.read<u16>();
    entries.reserve(filesFound);

    for (u16 index = 0; index < filesFound; index++) {
        if (!reader.canRead(202)) // size of next 6ish lines
            return false;
        MU u16 var0 = reader.read<u16>();
        MU u32 var1 = reader.read<u32>();
        MU u32 iterImageSize = reader.read<u32>();
        std::string iterFolderName = reader.readString(64);
        std::string iterWorldName = reader.readString(128);

        entries.emplace_back(
                var0,
                var1,
                iterImageSize,
                iterFolderName,
                iterWorldName,
                Buffer()
                );
    }

    for (auto& entry : entries) {
        if (!reader.canRead(entry.imageSize))
            return false;
        entry.imageData = reader.readBuffer(entry.imageSize);
    }

    return true;
}

bool CacheBinManager::save(const fs::path& outCachePath) {
    return false;
}

void CacheBinManager::addEntry(
        const std::string& folderName, const std::string& worldName,
        const Buffer& imageData, uint32_t fakeCRC) {

}
