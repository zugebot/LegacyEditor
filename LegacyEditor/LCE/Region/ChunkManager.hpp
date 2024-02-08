#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/data.hpp"


namespace editor {
    namespace chunk {
        class ChunkData;
    }

    class ChunkManager : public Data {
        static constexpr u32 CHUNK_BUFFER_SIZE = 0xFFFFFF; // 4,194,303

    public:
        struct FileData {
        private:
            struct {
                u32 timestamp : 32;
                u64 decSize : 29;
                u64 isCompressed : 1;
                u64 rleFlag : 1;
                u64 unknownFlag : 1;
            };
        public:
            FileData() {
                decSize = 0;
                isCompressed = 1;
                rleFlag = 1;
                unknownFlag = 1;
                timestamp = 0;
            }

            MU void setTimestamp(const u32 val) { timestamp = val; }
            MU void setDecSize(const u64 val) { decSize = val; }
            MU void setRLE(const u64 val) { rleFlag = val; }
            MU void setUnknown(const u64 val) { unknownFlag = val; }
            MU void setCompressed(const u64 val) { isCompressed = val; }

            MU ND u32 getTimestamp() const { return timestamp; }
            MU ND u64 getDecSize() const { return decSize; }
            MU ND u64 getRLE() const { return rleFlag; }
            MU ND u64 getUnknown() const { return unknownFlag; }
            MU ND u64 getCompressed() const { return isCompressed; }
        };

        FileData fileData{};
        chunk::ChunkData* chunkData = nullptr;

        /// CONSTRUCTORS

        ChunkManager();
        ~ChunkManager();

        /// FUNCTIONS

        void ensureDecompress(CONSOLE console);
        void ensureCompressed(CONSOLE console);

        MU void readChunk(CONSOLE console) const;
        MU void writeChunk(CONSOLE console)
    };

}