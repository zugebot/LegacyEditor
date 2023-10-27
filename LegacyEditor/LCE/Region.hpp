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
    u32 sectorCount = 0;

    CONSOLE console = CONSOLE::NONE;

    explicit Region(CONSOLE consoleIn) {
        console = consoleIn;
    }

    Chunk* getLoc(int x, int z) {
        int index = x + z * 32;
        if (index > CHUNK_COUNT) return nullptr;
        return &chunks[x + z * 32];
    }

    void read(File* fileIn) {
        // step 0: copying data from file
        Data dat;
        dat.allocate(fileIn->size);
        memcpy(dat.start(), fileIn->start(), fileIn->size);
        DataInManager managerIn(dat);

        // step 1: read offsets
        for (Chunk& chunk : chunks) {
            chunk.location = managerIn.readInt();
            chunk.data = managerIn.start() + chunk.getOffset();
        }

        // step 2: read timestamps
        for (Chunk& chunk : chunks) {
            chunk.timestamp = managerIn.readInt();
        }

        // step 3: read chunk size, decompressed size
        for (Chunk& chunk : chunks) {
            if (chunk.getSectorEnd() > sectorCount) {
                printf("chunk sector end goes outside file, could this file be corrupted?\n");
            }

            if (!chunk.isSaved() ) continue;

            // read chunk info
            auto offset = chunk.getSectorOffset();
            managerIn.seek((i64) offset);
            chunk.setDataSize(managerIn.readInt());

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
            memcpy(chunk.start(), managerIn.start(), chunk.size);
        }


    }



    File write(CONSOLE consoleIn) {

        // step 1: make sure all chunks are compressed correctly
        // step 2: recalculate sectorCount of each chunk
        // step 3: calculate chunk offsets for each chunk
        int sectors = 2;
        int count = 0;
        for (Chunk& chunk : chunks) {
            if (chunk.data == nullptr) {
                // reset chunk data
                chunk.location = 0;
                chunk.timestamp = 0;
            } else {
                chunk.compress(consoleIn);
                uint8_t chunk_sectors = chunk.size / SECTOR_SIZE;
                chunk.location = sectors * 256 + chunk_sectors;
                sectors += chunk_sectors;
            }
            count++;
        }

        // step 4: allocate memory and create buffer
        uint32_t data_size = sectors * SECTOR_SIZE;
        Data out = Data(data_size);
        DataOutManager managerOut(out);
        File returnFile(out);
        // auto* data_ptr = new uint8_t[data_size];
        // DataOutManager dataOut(data_ptr, data_size);

        // step 5: write each chunk offset
        for (const Chunk& chunk : chunks) {
            managerOut.writeInt(chunk.location);
        }

        // step 6: write each chunk timestamp
        for (const Chunk& chunk : chunks) {
            managerOut.writeInt(chunk.timestamp);
        }

        // step 7: seek to each location, write chunk attr's, then chunk data
        // make sure the pointer is a multiple of SECTOR_SIZE
        for (const Chunk& chunk : chunks) {
            managerOut.seek(chunk.getSectorOffset());
            managerOut.writeInt(chunk.size);

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

        return returnFile;
    }



















};