#pragma once

#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {


    /// "Elytra" chunks.
    /// "Aquatic" chunks.
    class V11Chunk {
    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim);

        V11Chunk() = default;

        MU void readBlocks() const;
        template<size_t BitsPerBlock>
        MU bool readGrid(u8 const* buffer, u8* grid) const;
        MU void readData() const;
        MU void readNBTData() const;
    };


}