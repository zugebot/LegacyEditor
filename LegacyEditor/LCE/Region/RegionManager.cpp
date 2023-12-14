#include "RegionManager.hpp"

#include "LegacyEditor/utils/file.hpp"


MU ChunkManager* RegionManager::getChunk(int x, int z) {
    u32 index = x + z * REGION_WIDTH;
    if (index > SECTOR_INTS) return nullptr;
    return &chunks[index];
}


MU ChunkManager* RegionManager::getChunk(int index) {
    if (index > SECTOR_INTS) return nullptr;
    return &chunks[index];
}


MU ChunkManager* RegionManager::getNonEmptyChunk() {
    for (auto & chunk : chunks) {
        if (chunk.size != 0) {
            return &chunk;
        }
    }
    return nullptr;
}


void RegionManager::read(File* fileIn) {
    read(&fileIn->data);
}


/**
 * step 1: copying data from file
 * step 2: read timestamps [CHUNK_COUNT]
 * step 3: read chunk size, decompressed size
 * step 4: read chunk info
 * step 5: allocates memory for the chunk
 * step 6: set chunk's decompressed size attribute
 * step 7: each chunk gets its own memory
 * @param dataIn
 */
void RegionManager::read(Data* dataIn) {
    u32 totalSectors = dataIn->size / SECTOR_BYTES + 1;

    size_t chunkIndex;
    u8 sectors[SECTOR_INTS];
    u32 locations[SECTOR_INTS];

    DataManager managerIn(dataIn, consoleIsBigEndian(console));

    for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        u32 val = managerIn.readInt32();
        sectors[chunkIndex] = val & 0xFF;
        locations[chunkIndex] = val >> 8;
    }

    for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        chunks[chunkIndex].setTimestamp(managerIn.readInt32());
    }

    for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        if (sectors[chunkIndex] == 0) continue;

        ChunkManager& chunk = chunks[chunkIndex];

        if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
            printf("[%u] chunk sector[%u, %u] end goes outside file...\n",
                   totalSectors, locations[chunkIndex], sectors[chunkIndex]);
            throw std::runtime_error("RegionManager::read error\n");
        }

        managerIn.seek(SECTOR_BYTES * locations[chunkIndex]);

        chunk.size = managerIn.readInt32();
        chunk.setRLE(chunk.size >> 31);
        chunk.setUnknown((chunk.size >> 30) & 1);
        chunk.size &= 0x00FFFFFF;
        chunk.allocate(chunk.size);

        switch (console) {
            case CONSOLE::PS3: {
                u32 x1 = managerIn.readInt32();
                u32 x2 = managerIn.readInt32();
                chunk.setDecSize(x1); // rle dec size
                break;
            }
            case CONSOLE::RPCS3: {
                u32 x1 = managerIn.readInt32();
                u32 x2 = managerIn.readInt32();
                chunk.setDecSize(x1); // final dec size
                break;
            }
            case CONSOLE::XBOX360:
            case CONSOLE::SWITCH:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            default:
                chunk.setDecSize(managerIn.readInt32()); // final dec size
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
Data RegionManager::write(CONSOLE consoleIn) {
    u32 locations[SECTOR_INTS] = {0};
    u8 sectors[SECTOR_INTS] = {0};

    int total_sectors = 2;
    for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        ChunkManager& chunk = chunks[chunkIndex];
        if (chunk.size == 0) {
            sectors[chunkIndex] = 0;
            locations[chunkIndex] = 0;
        } else {
            chunk.ensure_compressed(consoleIn);
            sectors[chunkIndex] = (chunk.size + 12) / SECTOR_BYTES + 1;
            locations[chunkIndex] = total_sectors;
            total_sectors += sectors[chunkIndex];
        }
    }

    u32 data_size = total_sectors * SECTOR_BYTES;
    Data dataOut = Data(data_size);
    DataManager managerOut(dataOut, consoleIsBigEndian(consoleIn));

    for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        managerOut.writeInt32(sectors[chunkIndex] | locations[chunkIndex] << 8);
    }

    for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        managerOut.writeInt32(chunks[chunkIndex].getTimestamp());
    }

    for (int chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {
        if (sectors[chunkIndex] == 0) continue;

        ChunkManager& chunk = chunks[chunkIndex];
        managerOut.seek(locations[chunkIndex] * SECTOR_BYTES);

        u32 size = chunk.size;
        if (chunk.getRLE())     size |= 0x80000000;
        if (chunk.getUnknown()) size |= 0x40000000;
        managerOut.writeInt32(size);

        switch (console) {
            case CONSOLE::PS3:
                managerOut.writeInt32(chunk.getDecSize());
                managerOut.writeInt32(chunk.getDecSize());
            case CONSOLE::RPCS3:
                managerOut.writeInt32(chunk.getDecSize());
                managerOut.writeInt32(chunk.getDecSize());
                break;
            case CONSOLE::XBOX360:
            case CONSOLE::SWITCH:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            default:
                managerOut.writeInt32(chunk.getDecSize());
                break;
        }
        managerOut.writeBytes(chunk.start(), chunk.size);
    }
    return dataOut;
}