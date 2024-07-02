#pragma once

#include "lce/enums.hpp"

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
            } anon{};
        public:
            FileData() {
                anon.decSize = 0;
                anon.isCompressed = 1;
                anon.rleFlag = 1;
                anon.unknownFlag = 1;
                anon.timestamp = 0;
            }

            MU void setTimestamp(c_u32 val) { anon.timestamp = val; }
            MU void setDecSize(c_u64 val) { anon.decSize = val; }
            MU void setRLE(c_u64 val) { anon.rleFlag = val; }
            MU void setUnknown(c_u64 val) { anon.unknownFlag = val; }
            MU void setCompressed(c_u64 val) { anon.isCompressed = val; }

            MU ND u32 getTimestamp() const { return anon.timestamp; }
            MU ND u64 getDecSize() const { return anon.decSize; }
            MU ND u64 getRLE() const { return anon.rleFlag; }
            MU ND u64 getUnknown() const { return anon.unknownFlag; }
            MU ND u64 getCompressed() const { return anon.isCompressed; }
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