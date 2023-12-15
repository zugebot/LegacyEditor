#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"

class NBTBase;

namespace editor::chunk {


    class ChunkData {
    public:
        // old version
        u8_vec oldBlocks;
        u8_vec blockData;

        // new version
        u16_vec newBlocks;
        u16_vec submerged;
        bool hasSubmerged = false;

        // all versions
        u8_vec blockLight;          //
        u8_vec skyLight;            //
        u8_vec heightMap;           //
        u8_vec biomes;              //
        NBTBase* NBTData = nullptr; //
        i16 terrainPopulated = 0;   //
        i64 lastUpdate = 0;         //
        i64 inhabitedTime = 0;      //

        /// Used to skip the lights in the chunk
        size_t DataGroupCount = 0;

        i32 chunkX = 0;
        i32 chunkZ = 0;

        i32 lastVersion = 0;
        bool validChunk = false;

        ~ChunkData();

        MU ND std::string getCoords() const;

        void defaultNBT();
    };
}
