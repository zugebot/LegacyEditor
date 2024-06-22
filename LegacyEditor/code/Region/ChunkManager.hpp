#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/error_status.hpp"


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

            MU void setTimestamp(c_u32 val) { timestamp = val; }
            MU void setDecSize(c_u64 val) { decSize = val; }
            MU void setRLE(c_u64 val) { rleFlag = val; }
            MU void setUnknown(c_u64 val) { unknownFlag = val; }
            MU void setCompressed(c_u64 val) { isCompressed = val; }

            MU ND u32 getTimestamp() const { return timestamp; }
            MU ND u64 getDecSize() const { return decSize; }
            MU ND u64 getRLE() const { return rleFlag; }
            MU ND u64 getUnknown() const { return unknownFlag; }
            MU ND u64 getCompressed() const { return isCompressed; }
        };

        FileData fileData;
        chunk::ChunkData* chunkData = nullptr;

        /// CONSTRUCTORS

        ChunkManager();
        ~ChunkManager();

        /// FUNCTIONS

        int ensureDecompress(lce::CONSOLE console);
        void ensureCompressed(lce::CONSOLE console);

        MU void readChunk(lce::CONSOLE inConsole) const;
        MU void writeChunk(lce::CONSOLE console);

        void setSizeFromReading(u32 sizeIn);
        ND u32 getSizeForWriting() const;

    };

}