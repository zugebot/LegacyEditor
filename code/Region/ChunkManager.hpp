#pragma once

#include "include/lce/enums.hpp"

#include "common/error_status.hpp"

#include "code/Chunk/chunkData.hpp"
#include "common/buffer.hpp"


namespace editor {


    class ChunkManager {
    public:
        struct ChunkHeader {
        private:
        public:
            u32 rleSize{0};
            u32 timestamp : 32 {0};
            u32 decSize : 29 {0};
            u32 isCompressedZip : 1 {1};
            u32 isCompressedRLE : 1 {1};
            u32 isNewSave : 1 {1};


            MU ND u32 getTimestamp() const { return timestamp; }
            MU void setTimestamp(c_u32 val) { timestamp = val; }

            MU ND u64 getDecSize() const { return decSize; }
            MU void setDecSize(c_u64 val) { decSize = val; }

            MU ND u32 getRLESize() const { return rleSize; }
            MU void setRLESize(c_u64 val) { rleSize = val; }

            MU ND u64 isRLECompressed() const { return isCompressedRLE; }
            MU void setRLECompressed(c_u64 val) { isCompressedRLE = val; }

            MU ND u64 getNewSaveFlag() const { return isNewSave; }
            MU void setNewSaveFlag(c_u64 val) { isNewSave = val; }

            MU ND u64 isZipCompressed() const { return isCompressedZip; }
            MU void setZipCompressed(c_u64 val) { isCompressedZip = val; }
        };

        Buffer buffer;
        ChunkHeader chunkHeader;
        chunk::ChunkData* chunkData = nullptr;

        MU ND std::string getDataAsString() const {
            std::string result;
            result += "__timestamp_" + std::to_string(chunkHeader.timestamp) + "___";
            result += "decSize_" + std::to_string(chunkHeader.decSize) + "___";
            result += "isComp_" + std::to_string(chunkHeader.isCompressedZip) + "___";
            result += "rleFlag_" + std::to_string(chunkHeader.isCompressedRLE) + "___";
            result += "unknown_" + std::to_string(chunkHeader.isNewSave);
            return result;
        }


        /// CONSTRUCTORS

        ChunkManager();
        ~ChunkManager();

        ChunkManager(ChunkManager&& other) noexcept
            : buffer(std::move(other.buffer)),
              chunkHeader(other.chunkHeader),
              chunkData(other.chunkData) {
            other.chunkHeader = ChunkHeader();
            other.chunkData = nullptr;
        }

        ChunkManager& operator=(ChunkManager&& other) noexcept {
            if (this != &other) {
                buffer = std::move(other.buffer);
                chunkHeader = other.chunkHeader;
                chunkData = other.chunkData;
                other.chunkData = nullptr;
            }
            return *this;
        }

        ChunkManager(const ChunkManager&) = delete;
        ChunkManager& operator=(const ChunkManager&) = delete;

        /// FUNCTIONS

        MU ND int checkVersion() const;

        int ensureDecompress(lce::CONSOLE console);
        int ensureCompressed(lce::CONSOLE console);

        MU int read(DataReader& reader, lce::CONSOLE console);
        MU int write(DataWriter& writer, lce::CONSOLE console);

        MU void readChunk(lce::CONSOLE inConsole);
        MU void writeChunk(lce::CONSOLE outConsole);

        void setVariableFlags(u32 sizeIn);
        ND u32 getSizeForWriting() const;
    };

} // namespace editor