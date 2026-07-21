#include "Region.hpp"

#include "include/lce/processor.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "common/error_status.hpp"

#include "common/data/DataReader.hpp"
#include "common/data/DataWriter.hpp"
#include "common/rle/rle_nsxps4.hpp"


namespace {



} // namespace


namespace editor {

    Region::Region(Region&& other) noexcept
        : m_handles(std::move(other.m_handles)),
          m_chunkCount(other.m_chunkCount),
          m_console(other.m_console),
          m_regX(other.m_regX),
          m_regZ(other.m_regZ),
          m_regScale(other.m_regScale) {

        other.m_chunkCount = 0;
        other.m_regX = 0;
        other.m_regZ = 0;
    }

    Region& Region::operator=(Region&& other) noexcept {
        if (this == &other) return *this;

        m_handles    = std::move(other.m_handles);
        m_chunkCount = other.m_chunkCount;
        m_console    = other.m_console;
        m_regX       = other.m_regX;
        m_regZ       = other.m_regZ;
        m_regScale   = other.m_regScale;

        other.m_chunkCount = 0;
        other.m_regX = 0;
        other.m_regZ = 0;

        return *this;
    }

    MU ND bool Region::extractChunk(i32 x, i32 z, ChunkHandle& out) {
        if (!inRange(x, z, m_regScale)) return false;

        ChunkHandle* src = getChunk(x, z);
        if (!src || src->state() == editor::ChunkState::EMPTY) return false;

        out = std::move(*src);
        *src = ChunkHandle{};

        if (out.state() != editor::ChunkState::EMPTY) {
            m_chunkCount--;
        }

        return true;
    }

    MU ND bool Region::insertChunk(i32 x, i32 z, ChunkHandle&& in) {
        if (!inRange(x, z, m_regScale)) return false;
        ChunkHandle* dst = getChunk(x, z);
        if (!dst) return false;

        const bool dstEmpty = (dst->state() == editor::ChunkState::EMPTY);
        const bool inEmpty  = (in.state()  == editor::ChunkState::EMPTY);

        if (dstEmpty && !inEmpty) m_chunkCount++;
        if (!dstEmpty && inEmpty) m_chunkCount--;

        *dst = std::move(in);
        return true;
    }

    MU ChunkHandle* Region::getNonEmptyChunk() {
        for (auto& chunk : m_handles) {
            if (chunk.state() != editor::ChunkState::EMPTY) {
                return &chunk;
            }
        }
        return nullptr;
    }

    MU ChunkHandle* Region::ensureChunk(c_int xIn, c_int zIn, eChunkVersion defaultChunkVersion) {
        ChunkHandle* chunk = getChunk(xIn, zIn);
        if (!chunk) { return nullptr; } // attempted to index out of bounds

        if (!(chunk->state() != editor::ChunkState::EMPTY)) {
            chunk->createNewChunk(xIn, zIn, defaultChunkVersion);
        }
        return chunk;
    }


    MU bool Region::moveChunkTo(Region& dst,
                             i32 x, i32 z,
                             i32 dx, i32 dz) {
        if (dx == -1) dx = x;
        if (dz == -1) dz = z;

        if (!inRange(x, z, m_regScale)) return false;
        if (!inRange(dx, dz, dst.m_regScale)) return false;

        ChunkHandle chunk;
        if (!extractChunk(x, z, chunk)) return false;

        return dst.insertChunk(dx, dz, std::move(chunk));
    }


    MU ChunkHandle* Region::getChunk(c_int xIn, c_int zIn) {
        if (!inRange(xIn, zIn, m_regScale)) return nullptr;
        c_u32 index = xIn + zIn * m_regScale;
        if (index >= m_handles.size()) return nullptr;
        return &m_handles[index];
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
    int Region::read(const LCEFile* fileIn) {
        m_console = fileIn->m_console;
        m_chunkCount = 0;
        Buffer buffer = fileIn->getBuffer();

        // new gen stuff
        // TODO: the new gen uncompress + decompress are decided based on different inputs...
        if (fileIn->isTinyRegionType() && fileIn->m_console != lce::CONSOLE::NEWGENMCS && fileIn->m_console != lce::CONSOLE::NEWGENMCS_BIG) {
            buffer = std::move(codec::RLE_NSXPS4_DECOMPRESS(buffer));
        }

        if (buffer.empty()) {
            return SUCCESS;
        }

        // DataWriter::writeFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\build\)" + fileIn->getFileName(),
        //                       fileIn->m_data.span());

        m_regX = fileIn->getRegionX();
        m_regZ = fileIn->getRegionZ();

        m_console = fileIn->m_console;

        c_u32 totalSectors = buffer.size() / SECTOR_BYTES + 1;

        size_t chunkIndex;
        std::vector<u8> sectors;
        std::vector<u32> locations;
        sectors.resize(CHUNK_COUNT);
        locations.resize(CHUNK_COUNT);

        DataReader reader(buffer.data(), buffer.size(), getConsoleEndian(m_console));


        reader.skip<0x2000>();
        for (chunkIndex = 0; chunkIndex < CHUNK_COUNT; chunkIndex++) {

            // sector + location + timestamp
            c_u32 val = reader.peek_at<u32>(0x0 + chunkIndex * 4);
            sectors[chunkIndex] = val & 0xFF;
            locations[chunkIndex] = val >> 8;
            if (sectors[chunkIndex] == 0) { continue; }
            c_u32 timestamp = reader.peek_at<u32>(0x1000 + chunkIndex * 4);
            m_handles[chunkIndex].header.setTimestamp(timestamp);

            // bound check
            if (locations[chunkIndex] + sectors[chunkIndex] > totalSectors) {
                printf("[%u] chunk sector[%u, %u] end goes outside file...\n",
                       totalSectors, locations[chunkIndex], sectors[chunkIndex]);
                continue;
            }

            // read chunk
            ChunkHandle& chunk = m_handles[chunkIndex];
            reader.seek(SECTOR_BYTES * locations[chunkIndex]);
            chunk.read(reader, m_console);
            m_chunkCount++;
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
    Buffer Region::write(WriteSettings& settings) {
        lce::CONSOLE consoleIn = settings.m_schematic.save_console;

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
                ChunkHandle& chunk = m_handles[chunkIndex];

                if (chunk.state() == editor::ChunkState::EMPTY) {
                    continue;
                }
                // If written, buffer already has compressed chunk bytes.
                else if (chunk.state() == editor::ChunkState::DECODED) {
                    chunk.encodeChunk(settings);
                }
                // otherwise, it is editor::ChunkState::COMPRESSED

                // Now we must have a buffer to write.
                if (chunk.buffer.empty()) {
                    // If this happens, your state machine is broken.
                    // Either assert or skip.
                    continue;
                }

                sectors[chunkIndex] = (chunk.buffer.size() + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;
                locations[chunkIndex] = total_sectors;
                total_sectors += sectors[chunkIndex];
            }
        }

        c_u32 data_size = total_sectors * SECTOR_BYTES;
        DataWriter writer(data_size, getConsoleEndian(consoleIn));

        u32 largestOffset = 0;
        writer.skip<0x2000>();
        int chunksWritten = 0;
        for (u32 x = 0; x < 32 - 2 + 2; x++) {
            for (u32 z = 0; z < 32; z++) {
                u32 chunkIndex = z * 32 + x;

                u32 chunk_header = locations[chunkIndex] << 8 | sectors[chunkIndex];
                writer.writeAtOffset<u32>(0x0 + chunkIndex * 4, chunk_header);

                u32 chunk_timestamp = m_handles[chunkIndex].header.getTimestamp();
                writer.writeAtOffset<u32>(0x1000 + chunkIndex * 4, chunk_timestamp);

                if (sectors[chunkIndex] != 0) {
                    ChunkHandle& chunk = m_handles[chunkIndex];
                    writer.seek(locations[chunkIndex] * SECTOR_BYTES);
                    chunk.write(writer, consoleIn);
                    chunksWritten++;

                    if (writer.tell() > largestOffset) {
                        largestOffset = writer.tell();
                    }
                }
            }
        }
        writer.seek(largestOffset);
        Buffer buffer = writer.take();


        // TODO: make this not do this when writing MCS files
        if (lce::isConsoleNewGen(consoleIn) && settings.m_schematic.save_console != lce::CONSOLE::NEWGENMCS && settings.m_schematic.save_console != lce::CONSOLE::NEWGENMCS_BIG) {
            Buffer res = codec::RLE_NSXPS4_COMPRESS(buffer);
            buffer = std::move(res);
        }


        return buffer;
    }
} // namespace editor