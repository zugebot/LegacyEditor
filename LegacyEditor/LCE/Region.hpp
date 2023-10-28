#pragma once

#include "Chunk.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


class Region {
private:
    static constexpr u32 CHUNK_COUNT = 1024;
    static constexpr u32 SECTOR_SIZE = 4096;

public:
    Chunk chunks[CHUNK_COUNT] = {};
    u32 totalSectors = 0;

    CONSOLE console = CONSOLE::NONE;

    explicit Region(CONSOLE consoleIn) {
        console = consoleIn;
    }

    Chunk* getChunk(int x, int z) {
        int index = x + z * 32;
        if (index > CHUNK_COUNT) return nullptr;
        return &chunks[index];
    }

    Chunk* getRelativeChunk(int x, int z) {
        int index = (x + 16) + (z + 16) * 32;
        if (index > CHUNK_COUNT) return nullptr;
        return &chunks[index];
    }


    Chunk* getChunk(int index) {
        if (index > CHUNK_COUNT) return nullptr;
        return &chunks[index];
    }


    void read(File* fileIn) {
        totalSectors = fileIn->size / SECTOR_SIZE;

        // step 0: copying data from file
        DataManager managerIn(fileIn);

        // step 1: read offsets
        for (Chunk& chunk : chunks) {
            chunk.location = managerIn.readInt24();
            chunk.sectors = managerIn.readByte();
        }

        // step 2: read timestamps
        for (Chunk& chunk : chunks) {
            chunk.timestamp = managerIn.readInt();
        }

        // step 3: read chunk size, decompressed size
        for (Chunk& chunk : chunks) {
            if (chunk.location + chunk.sectors > totalSectors) {
                printf("[%u] chunk sector[%u, %u] end goes outside file...\n", totalSectors, chunk.location, chunk.sectors);
            }

            if (chunk.sectors == 0) continue;

            // read chunk info
            managerIn.seek(4096 * chunk.location);

            // allocates memory for the chunk
            chunk.size = managerIn.readInt();
            chunk.rleFlag = chunk.size >> 31;
            chunk.size &= 0x3FFFFFFF;
            chunk.allocate(chunk.size);

            // set chunk's decompressed size attribute
            switch (console) {
                case CONSOLE::PS3:
                    chunk.dec_size = managerIn.readInt();
                    chunk.dec_size = managerIn.readInt();
                    break;
                case CONSOLE::XBOX360:
                case CONSOLE::WIIU:
                default:
                    chunk.dec_size = managerIn.readInt();
                    break;
            }

            // each chunk gets its own memory
            memcpy(chunk.start(), managerIn.getPtr(), chunk.size);

        }


    }



    Data write(CONSOLE consoleIn) {

        // step 1: make sure all chunks are compressed correctly
        // step 2: recalculate sectorCount of each chunk
        // step 3: calculate chunk offsets for each chunk
        int total_sectors = 2;
        int count = 0;
        for (Chunk& chunk : chunks) {
            if (chunk.sectors == 0) {
                chunk.location = 0;
                continue;
            }

            chunk.ensure_compressed(consoleIn);
            u8 chunk_sectors = (chunk.size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            chunk.location = total_sectors;
            total_sectors += chunk_sectors;

            count++;
        }

        // step 4: allocate memory and create buffer
        u32 data_size = total_sectors * SECTOR_SIZE;
        Data dataOut = Data(data_size);
        DataManager managerOut(dataOut);
        managerOut.setBigEndian();
        // auto* data_ptr = new uint8_t[data_size];
        // DataOutManager dataOut(data_ptr, data_size);

        // step 5: write each chunk offset
        managerOut.seekStart();
        for (const Chunk& chunk : chunks) {
            managerOut.writeInt24(chunk.location);
            managerOut.writeByte(chunk.sectors);
        }

        // return dataOut;

        // step 6: write each chunk timestamp
        for (const Chunk& chunk : chunks) {
            managerOut.writeInt(chunk.timestamp);
        }


        // step 7: seek to each location, write chunk attr's, then chunk data
        // make sure the pointer is a multiple of SECTOR_SIZE
        for (const Chunk& chunk : chunks) {
            if (chunk.sectors == 0) continue;
            managerOut.seek(chunk.location * 4096);

            u32 size = chunk.size;
            if (chunk.rleFlag) {
                u32 mask = 0x00FFFFFF;
                size &= mask;
                size |= (0xC0 << 24);
            }
            managerOut.writeInt(size);

            switch (console) {
                case CONSOLE::PS3:
                    managerOut.writeInt(chunk.dec_size);
                    managerOut.writeInt(chunk.dec_size);
                    break;
                case CONSOLE::XBOX360:
                case CONSOLE::WIIU:
                default:
                    managerOut.writeInt(chunk.dec_size);
                    break;
            }
            managerOut.write(chunk.start(), chunk.size);
        }

        return dataOut;
    }



















};