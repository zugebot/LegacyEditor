#include "RegionManager.hpp"

#include "LegacyEditor/utils/file.hpp"


MU ChunkManager* RegionManager::getChunk(int x, int z) {
    u32 index = x + z * REGION_WIDTH;
    if (index > CHUNK_COUNT) return nullptr;
    return &chunks[index];
}


MU ChunkManager* RegionManager::getChunk(int index) {
    if (index > CHUNK_COUNT) return nullptr;
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


void RegionManager::read(Data* dataIn) {
    u32 totalSectors = dataIn->size / SECTOR_SIZE + 1;

    size_t chunkIndex;
    u8 sectors[1024];
    u32 locations[1024];

    // step 0: copying data from file
    DataManager managerIn(dataIn, consoleIsBigEndian(console));

    for (chunkIndex = 0; chunkIndex < 1024; chunkIndex++) {
        u32 val = managerIn.readInt32();
        sectors[chunkIndex] = val & 0xFF;
        locations[chunkIndex] = val >> 8;
    }

    // step 2: read timestamps [1024]
    for (ChunkManager& chunk : chunks) {
        chunk.setTimestamp(managerIn.readInt32());
    }

    // step 3: read chunk size, decompressed size
    chunkIndex = 0;
    for (ChunkManager& chunk : chunks) {
        if (sectors[chunkIndex] == 0) {
            chunkIndex++;
            continue;
        }

        if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
            printf("[%u] chunk sector[%u, %u] end goes outside file...\n", totalSectors, locations[chunkIndex], sectors[chunkIndex]);
            throw std::runtime_error("debug here");
        }

        // read chunk info
        managerIn.seek(SECTOR_SIZE * locations[chunkIndex]);
        chunkIndex++;

        // allocates memory for the chunk
        chunk.size = managerIn.readInt32();
        chunk.setRLE(chunk.size >> 31);
        chunk.setUnknown((chunk.size >> 30) & 1);
        chunk.size &= 0x3FFFFFFF;
        chunk.allocate(chunk.size);

        // set chunk's decompressed size attribute
        switch (console) {
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
                chunk.setDecSize(managerIn.readInt32()); // rle dec size (?)
                chunk.setDecSize(managerIn.readInt32()); // final dec size
                break;
            case CONSOLE::XBOX360:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            default:
                chunk.setDecSize(managerIn.readInt32()); // final dec size
                break;
        }

        // each chunk gets its own memory
        memcpy(chunk.start(), managerIn.ptr, chunk.size);

    }


}


Data RegionManager::write(CONSOLE consoleIn) {
    u32 locations[1024] = {0};
    u8 sectors[1024] = {0};

    // step 1: make sure all chunks are compressed correctly
    // step 2: recalculate sectorCount of each chunk
    // step 3: calculate chunk offsets for each chunk
    int total_sectors = 2;
    size_t chunkIndex = 0;
    for (ChunkManager& chunk : this->chunks) {
        if (chunk.size == 0) {
            sectors[chunkIndex] = 0;
            locations[chunkIndex] = 0;
        } else {
            chunk.ensure_compressed(consoleIn);
            sectors[chunkIndex] = chunk.size / SECTOR_SIZE + 1;
            locations[chunkIndex] = total_sectors;
            total_sectors += sectors[chunkIndex];
        }
        chunkIndex++;
    }

    // step 4: allocate memory and create buffer
    u32 data_size = total_sectors * SECTOR_SIZE;
    Data dataOut = Data(data_size);
    DataManager managerOut(dataOut, consoleIsBigEndian(consoleIn));

    // step 5: write each chunk offset
    for (chunkIndex = 0; chunkIndex < 1024; chunkIndex++) {
        u32 val = sectors[chunkIndex] | locations[chunkIndex] << 8;
        managerOut.writeInt32(val);
    }

    // step 6: write each chunk timestamp
    for (const ChunkManager& chunk : chunks) {
        managerOut.writeInt32(chunk.getTimestamp());
    }

    // step 7: seek to each location, write chunk attr's, then chunk data
    // make sure the pointer is a multiple of SECTOR_SIZE
    chunkIndex = 0;
    for (const ChunkManager& chunk : chunks) {
        if (sectors[chunkIndex] == 0) {
            chunkIndex++;
            continue;
        }

        // this looks kinda bad
        managerOut.seek(locations[chunkIndex++] * SECTOR_SIZE);

        u32 size = chunk.size;
        if (chunk.getRLE())     size |= 0x80000000;
        if (chunk.getUnknown()) size |= 0x40000000;
        managerOut.writeInt32(size);

        switch (console) {
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
                managerOut.writeInt32(chunk.getDecSize());
                managerOut.writeInt32(chunk.getDecSize());
                break;
            case CONSOLE::XBOX360:
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