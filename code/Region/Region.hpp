#pragma once

#include "include/lce/processor.hpp"

#include "code/SaveFile/writeSettings.hpp"
#include "code/chunk/chunkHandle.hpp"


namespace editor {
    class LCEFile;

    inline bool inRange(i32 x, i32 z, i32 scale) {
        return x >= 0 && x < scale && z >= 0 && z < scale;
    }

    class Region {
        static constexpr u32 REGION_WIDTH = 32;
        static constexpr u32 CHUNK_COUNT = 1024;
        static constexpr u32 SECTOR_BYTES = 0x1000;
        static constexpr u32 CHUNK_HEADER_SIZE = 12;

    public:

        std::vector<ChunkHandle> m_handles;
        i32 m_chunkCount;

        lce::CONSOLE m_console;
        i32 m_regX;
        i32 m_regZ;
        i32 m_regScale;


        /// CONSTRUCTORS

        Region()
            : m_console(lce::CONSOLE::NONE), m_regX(0), m_regZ(0),
              m_chunkCount(0), m_regScale(32) {
            m_handles.resize(1024);
        }

        Region(i32 regX, i32 regZ, lce::CONSOLE console = lce::CONSOLE::NONE)
            : m_console(console), m_regX(regX), m_regZ(regZ),
              m_chunkCount(0), m_regScale(32) {
            m_handles.resize(1024);
        }

        ~Region() = default;

        ND i32 x() const { return m_regX; }
        ND i32 z() const { return m_regZ; }

        /// FUNCTIONS

        MU ChunkHandle* getChunk(int xIn, int zIn);
        MU ChunkHandle* getNonEmptyChunk();

        bool extractChunk(i32 x, i32 z, ChunkHandle& out);

        /// Insert `in` at (x,z), replacing any existing chunk there.
        /// `in` is left empty (moved-from). Returns false on invalid coords.
        bool insertChunk(i32 x, i32 z, ChunkHandle&& in);

        bool moveChunkTo(Region& dst, i32 x, i32 z, i32 dx = -1, i32 dz = -1);

        /// READ AND WRITE

        int read(const LCEFile* fileIn);
        Buffer write(WriteSettings& settings);

    };

}
