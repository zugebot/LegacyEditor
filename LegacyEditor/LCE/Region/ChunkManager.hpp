#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/data.hpp"

class ChunkManager : public Data {
private:
    union {
        struct {
            u32 decSize : 29;
            u32 isCompressed : 1;
            u32 rleFlag : 1;
            u32 unknownFlag : 1;
        } var{};
    };

public:
    u32 timestamp = 0;

    ChunkManager() {
        var.decSize = 0;
        var.isCompressed = 1;
        var.rleFlag = 1;
        var.unknownFlag = 1;
    }



    MU void setDecSize(u32 val) {var.decSize = val;}
    MU void setRLE(u32 val) {var.rleFlag = val;}
    MU void setUnknown(u32 val) {var.unknownFlag = val;}
    MU void setCompressed(u32 val) {var.isCompressed = val;}
    MU ND u32 getDecSize() const {return var.decSize;}
    MU ND u32 getRLE() const {return var.rleFlag;}
    MU ND u32 getUnknown() const {return var.unknownFlag;}
    MU ND u32 getCompressed() const {return var.isCompressed;}


    ~ChunkManager() = default;
    void ensure_decompress(CONSOLE console);
    void ensure_compressed(CONSOLE console);
};
