#pragma once

#include "include/lce/enums.hpp"

#include "common/data.hpp"
#include "common/error_status.hpp"

#include "code/Chunk/chunkData.hpp"


namespace editor {
    // namespace chunk {
    //     class ChunkData;
    // }

    class ChunkManager : public Data {
        static constexpr u32 CHUNK_BUFFER_SIZE = 0xFFFFFF; // 4,194,303

    public:
        struct FileData {
        private:

        public:
            u32 rleSize;
            struct {
                u32 timestamp : 32;
                u64 decSize : 29;
                u64 isCompressed : 1;
                u64 rleFlag : 1;
                u64 unknownFlag : 1;
            } anon{};
            FileData() {
                anon.decSize = 0;
                rleSize = 0;
                anon.isCompressed = 1;
                anon.rleFlag = 1;
                anon.unknownFlag = 1;
                anon.timestamp = 0;
            }

            MU void setTimestamp(c_u32 val) { anon.timestamp = val; }
            MU void setDecSize(c_u64 val) { anon.decSize = val; }
            MU void setRLESize(c_u64 val) { rleSize = val; }

            MU void setRLEFlag(c_u64 val) { anon.rleFlag = val; }
            MU void setUnknownFlag(c_u64 val) { anon.unknownFlag = val; }
            MU void setCompressedFlag(c_u64 val) { anon.isCompressed = val; }

            MU ND u32 getTimestamp() const { return anon.timestamp; }
            MU ND u64 getDecSize() const { return anon.decSize; }
            MU ND u32 getRLESize() const { return rleSize; }
            MU ND u64 getRLEFlag() const { return anon.rleFlag; }
            MU ND u64 getUnknownFlag() const { return anon.unknownFlag; }
            MU ND u64 getCompressedFlag() const { return anon.isCompressed; }
        };

        FileData fileData;
        chunk::ChunkData* chunkData = nullptr;

        MU ND std::string getDataAsString() const {
            std::string result;
            result += "__timestamp_" + std::to_string(fileData.anon.timestamp) + "___";
            result += "decSize_" + std::to_string(fileData.anon.decSize) + "___";
            result += "isComp_" + std::to_string(fileData.anon.isCompressed) + "___";
            result += "rleFlag_" + std::to_string(fileData.anon.rleFlag) + "___";
            result += "unknown_" + std::to_string(fileData.anon.unknownFlag);
            return result;
        }



        /// CONSTRUCTORS

        ChunkManager();
        ~ChunkManager();

        /// FUNCTIONS

        MU ND int checkVersion() const;

        int ensureDecompress(lce::CONSOLE consoleIn, bool skipRLE = false);
        int ensureCompressed(lce::CONSOLE console, bool skipRLE = false);

        MU void readChunk(lce::CONSOLE inConsole);
        MU void writeChunk(lce::CONSOLE outConsole);

        void setSizeFromReading(u32 sizeIn);
        ND u32 getSizeForWriting() const;

    };

}