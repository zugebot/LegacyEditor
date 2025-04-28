#include "RegionManager.hpp"

#include <stdexcept>
#include <cstring>

#include "code/LCEFile/LCEFile.hpp"
#include "common/dataManager.hpp"
#include "common/error_status.hpp"


namespace editor {


    MU ChunkManager* RegionManager::getChunk(c_int xIn, c_int zIn) {
        c_u32 index = xIn + zIn * REGION_WIDTH;
        if (index > SECTOR_INTS) { return nullptr; }
        return &chunks[index];
    }


    MU ChunkManager* RegionManager::getChunk(c_u32 index) {
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
    int RegionManager::read(const LCEFile* fileIn) {
        if (fileIn->data.size == 0) {
            return SUCCESS;
        }

        myConsole = fileIn->console;
        auto* dataIn = &fileIn->data;

        c_u32 totalSectors = dataIn->size / SECTOR_BYTES + 1;

        size_t chunkIndex;
        u8 sectors[SECTOR_INTS];
        u32 locations[SECTOR_INTS];

        DataManager managerIn(dataIn, getConsoleEndian(myConsole));


        managerIn.skip<0x2000>();
        for (chunkIndex = 0; chunkIndex < SECTOR_INTS; chunkIndex++) {

            // first
            {
                c_u32 val = managerIn.readAtOffset<u32>(0x0 + chunkIndex * 4);
                sectors[chunkIndex] = val & 0xFF;
                locations[chunkIndex] = val >> 8;
            }

            // second
            {
                c_u32 timestamp = managerIn.readAtOffset<u32>(0x1000 + chunkIndex * 4);
                chunks[chunkIndex].fileData.setTimestamp(timestamp);
            }

            if (sectors[chunkIndex] == 0) {
                continue;
            }

            ChunkManager& chunk = chunks[chunkIndex];

            if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
                printf("[%u] chunk sector[%u, %u] end goes outside file...\n",
                       totalSectors, locations[chunkIndex], sectors[chunkIndex]);
                throw std::runtime_error("RegionManager::read error\n");
            }

            managerIn.seek(SECTOR_BYTES * locations[chunkIndex]);
            chunk.setSizeFromReading(managerIn.read<u32>());


            bool status = chunk.allocate(chunk.size);
            if (!status) {
                printf("Failed to allocate %d bytes for chunk", chunk.size);
                return STATUS::MALLOC_FAILED;
            }
            std::memset(chunk.data, 0, chunk.size);

            switch (myConsole) {
                case lce::CONSOLE::PS3:
                case lce::CONSOLE::RPCS3: {
                    chunk.fileData.setDecSize(managerIn.read<u32>());
                    chunk.fileData.setRLESize(managerIn.read<u32>());
                    break;
                }
                default:
                    c_u32 dec_and_rle_size = managerIn.read<u32>();
                    chunk.fileData.setDecSize(dec_and_rle_size);
                    chunk.fileData.setRLESize(dec_and_rle_size);
                    break;
            }
            std::memcpy(chunk.start(), managerIn.ptr(), chunk.size);

        }
        return SUCCESS;
    }


    void RegionManager::convertChunks(lce::CONSOLE consoleIn) {
        MU int index = 0;
        for (auto& chunk: chunks) {
            if (chunk.size == 0) continue;

            MU c_bool shouldSkipRLE = chunk.fileData.getCompressedFlag();
            chunk.ensureDecompress(myConsole, shouldSkipRLE);
            chunk.ensureCompressed(consoleIn, shouldSkipRLE);

            index++;
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
    Data RegionManager::write(const lce::CONSOLE consoleIn) {
        u8 sectors[SECTOR_INTS] = {};
        u32 locations[SECTOR_INTS] = {};

        // 1: Sectors Block
        // 2: Locations Block
        int total_sectors = 2;
        for (u32 x = 0; x < 32; x++) {
            for (u32 z = 0; z < 32; z++) {
                u32 chunkIndex = z * 32 + x;
                if (ChunkManager& chunk = chunks[chunkIndex]; chunk.size != 0) {
                    chunk.ensureCompressed(consoleIn);
                    sectors[chunkIndex] = (chunk.size + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;
                    locations[chunkIndex] = total_sectors;
                    total_sectors += sectors[chunkIndex];
                }
            }
        }

        c_u32 data_size = total_sectors * SECTOR_BYTES;
        Data dataOut;
        dataOut.allocate(data_size);
        DataManager managerOut(dataOut, getConsoleEndian(consoleIn));
#ifndef DONT_MEMSET0
        std::memset(dataOut.data, 0, dataOut.size);
#endif
        u32 largestOffset = 0;
        managerOut.skip<0x2000>();
        for (u32 x = 0; x < 32; x++) {
            for (u32 z = 0; z < 32; z++) {
                u32 chunkIndex = z * 32 + x;

                u32 chunk_header = sectors[chunkIndex] | locations[chunkIndex] << 8;
                managerOut.writeAtOffset<u32>(0x0 + chunkIndex * 4, chunk_header);

                u32 chunk_timestamp = chunks[chunkIndex].fileData.getTimestamp();
                managerOut.writeAtOffset<u32>(0x1000 + chunkIndex * 4, chunk_timestamp);


                if (sectors[chunkIndex] == 0) {
                    managerOut.seek(locations[chunkIndex] * SECTOR_BYTES);
                    managerOut.skip<8>();
                    if (consoleIn == lce::CONSOLE::PS3 || consoleIn == lce::CONSOLE::RPCS3) {
                        managerOut.skip<4>();
                    }

                } else {
                    ChunkManager& chunk = chunks[chunkIndex];
                    managerOut.seek(locations[chunkIndex] * SECTOR_BYTES);
                    managerOut.write<u32>(chunk.getSizeForWriting());
                    switch (consoleIn) {
                        case lce::CONSOLE::PS3:
                        case lce::CONSOLE::RPCS3:
                            managerOut.write<u32>(chunk.fileData.getDecSize());
                            managerOut.write<u32>(chunk.fileData.getRLESize());
                            break;
                        default:
                            managerOut.write<u32>(chunk.fileData.getDecSize());
                            break;
                    }
                    managerOut.writeBytes(chunk.start(), chunk.size);
                    if (managerOut.tell() > largestOffset) {
                        largestOffset = managerOut.tell();
                    }
                }

            }
        }

        dataOut.size = largestOffset;
        return dataOut;
    }
}