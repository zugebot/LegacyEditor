#include "RegionManager.hpp"

#include <stdexcept>

#include "LegacyEditor/LCE/FileListing/LCEFile.hpp"
#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


namespace editor {


    RegionManager::RegionManager() = default;


    RegionManager::~RegionManager() = default;


    MU ChunkManager* RegionManager::getChunk(const int xIn, const int zIn) {
        const u32 index = xIn + zIn * REGION_WIDTH;
        if (index > SECTOR_INTS) { return nullptr; }
        return &chunks[index];
    }


    MU ChunkManager* RegionManager::getChunk(const int index) {
        if (index > SECTOR_INTS) { return nullptr; }
        return &chunks[index];
    }


    MU ChunkManager* RegionManager::getNonEmptyChunk() {
        for (auto& chunk: chunks) {
            if (chunk.size != 0) {
                return &chunk;
            }
        }
        return nullptr;
    }


    /**
     * step 1: copying data from file
     * step 2: read timestamps [CHUNK_COUNT]
     * step 3: read chunk size, decompressed size
     * step 4: read chunk info
     * step 5: allocates memory for the chunk
     * step 6: set chunk's decompressed size attribute
     * step 7: each chunk gets its own memory
     * @param fileIn
     */
    void RegionManager::read(const LCEFile* fileIn) {
        console = fileIn->console;
        auto* dataIn = &fileIn->data;

        const u32 totalSectors = dataIn->size / SECTOR_BYTES + 1;

        size_t chunkIndex;
        u8 sectors[SECTOR_INTS];
        u32 locations[SECTOR_INTS];

        DataManager managerIn(dataIn, consoleIsBigEndian(console));

        for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
            const u32 val = managerIn.readInt32();
            sectors[chunkIndex] = val & 0xFF;
            locations[chunkIndex] = val >> 8;
        }

        for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
            chunks[chunkIndex].fileData.setTimestamp(managerIn.readInt32());
        }

        for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
            if (sectors[chunkIndex] == 0) { continue; }

            ChunkManager& chunk = chunks[chunkIndex];

            if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
                printf("[%u] chunk sector[%u, %u] end goes outside file...\n",
                       totalSectors, locations[chunkIndex], sectors[chunkIndex]);
                throw std::runtime_error("RegionManager::read error\n");
            }

            managerIn.seek(SECTOR_BYTES * locations[chunkIndex]);

            chunk.setSizeFromReading(managerIn.readInt32());
            chunk.allocate(chunk.size);

            switch (console) {
                case CONSOLE::PS3:
                case CONSOLE::RPCS3: {
                    const u32 num1 = managerIn.readInt32();
                    managerIn.readInt32();
                    chunk.fileData.setDecSize(num1); // rle dec size
                    break;
                }
                default:
                    chunk.fileData.setDecSize(managerIn.readInt32()); // final dec size
                    break;
                }
                memcpy(chunk.start(), managerIn.ptr, chunk.size);
            }
        }


    /**
     * step 1: make sure all chunks are compressed correctly
     * step 2: recalculate sectorCount of each chunk
     * step 3: calculate chunk offsets for each chunk
     * step 4: allocate memory and create buffer
     * step 5: write each chunk offset
     * step 6: write each chunk timestamp
     * step 7: seek to each location, write chunk attr's, then chunk data
     * @param consoleIn
     * @return
     */
        Data RegionManager::write(const CONSOLE consoleIn) {
            u8 sectors[SECTOR_INTS] = {};
            u32 locations[SECTOR_INTS] = {};

            // 1: Sectors Block
            // 2: Locations Block
            int total_sectors = 2;
            for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
                if (ChunkManager& chunk = chunks[chunkIndex]; chunk.size != 0) {
                    chunk.ensureCompressed(consoleIn);
                    sectors[chunkIndex] = (chunk.size + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;
                    locations[chunkIndex] = total_sectors;
                    total_sectors += sectors[chunkIndex];
                }
            }

            const u32 data_size = total_sectors * SECTOR_BYTES;
            const auto dataOut = Data(data_size);
            DataManager managerOut(dataOut, consoleIsBigEndian(consoleIn));

            for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
                managerOut.writeInt32(sectors[chunkIndex] | locations[chunkIndex] << 8);
            }

            for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
                managerOut.writeInt32(chunks[chunkIndex].fileData.getTimestamp());
            }

            for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
                if (sectors[chunkIndex] == 0) { continue; }

                ChunkManager& chunk = chunks[chunkIndex];
                managerOut.seek(locations[chunkIndex] * SECTOR_BYTES);
                managerOut.writeInt32(chunk.getSizeForWriting());
                switch (console) {
                    case CONSOLE::PS3:
                    case CONSOLE::RPCS3:
                        managerOut.writeInt32(chunk.fileData.getDecSize());
                    default:
                        managerOut.writeInt32(chunk.fileData.getDecSize());
                        break;
                }
                managerOut.writeBytes(chunk.start(), chunk.size);
            }
        return dataOut;
    }
}