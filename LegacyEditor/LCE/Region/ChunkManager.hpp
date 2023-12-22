#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/data.hpp"

#include "LegacyEditor/LCE/Chunk/chunkData.hpp"

namespace editor {

    class ChunkManager : public Data {
        static constexpr u32 CHUNK_BUFFER_SIZE = 0xFFFFFF; // 4,194,303

        enum CHUNK_HEADER : i16 {
            HEADER_NBT = 0x0a00,
            V_8 = 0x0008,
            V_9 = 0x0009,
            V_10 = 0x000A,
            V_11 = 0x000B,
            V_12 = 0x000C,
        };

        struct {
            u32 timestamp;
            u64 decSize : 29;
            u64 isCompressed : 1;
            u64 rleFlag : 1;
            u64 unknownFlag : 1;
        } managerData{};

    public:
        ChunkManager() {
            managerData.decSize = 0;
            managerData.isCompressed = 1;
            managerData.rleFlag = 1;
            managerData.unknownFlag = 1;
            managerData.timestamp = 0;
            chunkData = new chunk::ChunkData();
        }

        chunk::ChunkData* chunkData = nullptr;

        ~ChunkManager() {
            delete chunkData;
        }

        // setters

        MU void setTimestamp(const u32 val) { managerData.timestamp = val; }
        MU void setDecSize(const u64 val) { managerData.decSize = val; }
        MU void setRLE(const u64 val) { managerData.rleFlag = val; }
        MU void setUnknown(const u64 val) { managerData.unknownFlag = val; }
        MU void setCompressed(const u64 val) { managerData.isCompressed = val; }

        // getters

        MU ND u32 getTimestamp() const { return managerData.timestamp; }
        MU ND u64 getDecSize() const { return managerData.decSize; }
        MU ND u64 getRLE() const { return managerData.rleFlag; }
        MU ND u64 getUnknown() const { return managerData.unknownFlag; }
        MU ND u64 getCompressed() const { return managerData.isCompressed; }

        // funcs

        void ensureDecompress(CONSOLE console);
        void ensureCompressed(CONSOLE console);

        MU void readChunk(CONSOLE console) const;
        MU void writeChunk(CONSOLE console);
    };

}