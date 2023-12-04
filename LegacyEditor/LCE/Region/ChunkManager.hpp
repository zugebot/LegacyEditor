#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/data.hpp"

class ChunkManager : public Data {
private:
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
    }

    MU void setTimestamp(u32 val) { managerData.timestamp = val;}
    MU void setDecSize(u64 val) { managerData.decSize = val;}
    MU void setRLE(u64 val) { managerData.rleFlag = val;}
    MU void setUnknown(u64 val) { managerData.unknownFlag = val;}
    MU void setCompressed(u64 val) { managerData.isCompressed = val;}

    MU ND u32 getTimestamp() const {return managerData.timestamp;}
    MU ND u64 getDecSize() const {return managerData.decSize;}
    MU ND u64 getRLE() const {return managerData.rleFlag;}
    MU ND u64 getUnknown() const {return managerData.unknownFlag;}
    MU ND u64 getCompressed() const {return managerData.isCompressed;}

    ~ChunkManager() = default;
    void ensure_decompress(CONSOLE console);
    void ensure_compressed(CONSOLE console);
};
