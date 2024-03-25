#pragma once

#include "ChunkManager.hpp"
#include "LegacyEditor/LCE/MC/containsConsole.hpp"
#include "LegacyEditor/utils/processor.hpp"


namespace editor {
    class File;

    class RegionManager : public Data, public ContainsConsole {
        static constexpr u32 REGION_WIDTH = 32;
        static constexpr u32 SECTOR_BYTES = 4096;
        static constexpr u32 SECTOR_INTS = 1024; // SECTOR_BYTES / 4
        static constexpr u32 CHUNK_HEADER_SIZE = 12;

    public:
        ChunkManager* chunks = nullptr;
        bool isRead = false;

        /// CONSTRUCTORS

        explicit RegionManager(const File* fileIn);
        ~RegionManager();

        /// FUNCTIONS

        MU ND ChunkManager* getChunk(int xIn, int zIn) const;
        MU ND ChunkManager* getChunk(int index) const;
        MU ND ChunkManager* getNonEmptyChunk() const;

        /// READ / WRITE

        void ensureRead();
        Data ensureWrite(CONSOLE consoleIn);
    };

}
