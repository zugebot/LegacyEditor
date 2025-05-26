#include "RegionManager.hpp"

#include "include/lce/processor.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "common/error_status.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace {



} // namespace


namespace editor {


    MU ND bool RegionManager::extractChunk(i32 x, i32 z, ChunkManager& out) {
        if (!inRange(x, z, m_regScale)) return false;

        ChunkManager* src = getChunk(x, z);
        if (!src || src->buffer.empty()) return false;

        out = std::move(*src);
        *src = ChunkManager{};
        return true;
    }

    MU ND bool RegionManager::insertChunk(i32 x, i32 z, ChunkManager&& in) {
        if (!inRange(x, z, m_regScale)) return false;
        ChunkManager* dst = getChunk(x, z);
        if (!dst) return false;

        *dst = std::move(in); // move into the region
        return true;
    }


    bool RegionManager::moveChunkTo(RegionManager& dst,
                                    i32 x, i32 z,
                                    i32 dx, i32 dz) {
        if (dx == -1) dx = x;
        if (dz == -1) dz = z;

        if (!inRange(x, z, m_regScale)) return false;
        if (!inRange(dx, dz, dst.m_regScale)) return false;

        ChunkManager chunk;
        if (!extractChunk(x, z, chunk)) return false;

        return dst.insertChunk(dx, dz, std::move(chunk));
    }


    void RegionManager::convertChunks(lce::CONSOLE consoleIn) {
        MU int index = 0;
        for (auto& chunk: m_chunks) {
            if (chunk.buffer.empty()) continue;

            chunk.ensureDecompress(m_console);
            chunk.ensureCompressed(consoleIn);

            index++;
        }
    }


    MU ChunkManager* RegionManager::getChunk(c_int xIn, c_int zIn) {
        c_u32 index = xIn + zIn * m_regScale;
        if (index > CHUNK_COUNT) { return nullptr; }
        return &m_chunks[index];
    }


    MU ChunkManager* RegionManager::getNonEmptyChunk() {
        for (auto& chunk: m_chunks) {
            if (!chunk.buffer.empty()) {
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
        if (fileIn->m_data.empty()) {
            return SUCCESS;
        }

        // DataWriter::writeFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\build\)" + fileIn->getFileName(),
        //                       fileIn->m_data.span());

        m_regX = fileIn->getRegionX();
        m_regZ = fileIn->getRegionZ();

        m_console = fileIn->m_console;
        auto* dataIn = &fileIn->m_data;

        c_u32 totalSectors = dataIn->size() / SECTOR_BYTES + 1;

        size_t chunkIndex;
        std::vector<u8> sectors;
        std::vector<u32> locations;
        sectors.resize(CHUNK_COUNT);
        locations.resize(CHUNK_COUNT);

        DataReader reader(dataIn->data(), dataIn->size(), getConsoleEndian(m_console));


        reader.skip<0x2000>();
        for (chunkIndex = 0; chunkIndex < CHUNK_COUNT; chunkIndex++) {

            // sector + location + timestamp
            c_u32 val = reader.peek_at<u32>(0x0 + chunkIndex * 4);
            sectors[chunkIndex] = val & 0xFF;
            locations[chunkIndex] = val >> 8;
            if (sectors[chunkIndex] == 0) { continue; }
            c_u32 timestamp = reader.peek_at<u32>(0x1000 + chunkIndex * 4);
            m_chunks[chunkIndex].chunkHeader.setTimestamp(timestamp);

            // bound check
            if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
                printf("[%u] chunk sector[%u, %u] end goes outside file...\n",
                       totalSectors, locations[chunkIndex], sectors[chunkIndex]);
                throw std::runtime_error("RegionManager::read error\n");
            }

            // read chunk
            ChunkManager& chunk = m_chunks[chunkIndex];
            reader.seek(SECTOR_BYTES * locations[chunkIndex]);
            chunk.read(reader, m_console);
        }
        return SUCCESS;
    }


    /**
     * step 1: make sure all chunks are compressed correctly
     * step 2: recalculate sectorCount of each chunk
     * step 3: calculate chunk offsets for each chunk
     * step 4: allocate memory and create buffer
     * step 5: write each chunk offset
     * step 6: write each chunk m_timestamp
     * step 7: seek to each location, write chunk attr's, then chunk data
     * @param consoleIn
     * @return
     */
    Buffer RegionManager::write(const lce::CONSOLE consoleIn) {
        std::vector<u8> sectors;
        std::vector<u32> locations;
        sectors.resize(CHUNK_COUNT);
        locations.resize(CHUNK_COUNT);


        // 1: Sectors Block
        // 2: Locations Block
        int total_sectors = 2;
        for (u32 x = 0; x < 32; x++) {
            for (u32 z = 0; z < 32; z++) {
                u32 chunkIndex = z * 32 + x;
                if (ChunkManager& chunk = m_chunks[chunkIndex]; !chunk.buffer.empty()) {
                    chunk.writeChunk(consoleIn);
                    chunk.ensureCompressed(consoleIn);
                    sectors[chunkIndex] = (chunk.buffer.size() + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;
                    locations[chunkIndex] = total_sectors;
                    total_sectors += sectors[chunkIndex];
                }
            }
        }

        c_u32 data_size = total_sectors * SECTOR_BYTES;
        DataWriter writer(data_size, getConsoleEndian(consoleIn));

#ifndef DONT_MEMSET0
        std::memset((void*) writer.data(), 0, writer.size());
#endif

        u32 largestOffset = 0;
        writer.skip<0x2000>();
        for (u32 x = 0; x < 32 - 2 + 2; x++) {
            for (u32 z = 0; z < 32; z++) {
                u32 chunkIndex = z * 32 + x;

                u32 chunk_header = sectors[chunkIndex] | locations[chunkIndex] << 8;
                writer.writeAtOffset<u32>(0x0 + chunkIndex * 4, chunk_header);

                u32 chunk_timestamp = m_chunks[chunkIndex].chunkHeader.getTimestamp();
                writer.writeAtOffset<u32>(0x1000 + chunkIndex * 4, chunk_timestamp);

                if (sectors[chunkIndex] != 0) {
                    ChunkManager& chunk = m_chunks[chunkIndex];
                    writer.seek(locations[chunkIndex] * SECTOR_BYTES);
                    chunk.write(writer, consoleIn);

                    if (writer.tell() > largestOffset) {
                        largestOffset = writer.tell();
                    }
                }
            }
        }
        writer.seek(largestOffset);
        Buffer dataOut = writer.take();

        return dataOut;
    }
} // namespace editor