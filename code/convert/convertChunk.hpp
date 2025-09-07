#pragma once


namespace editor {

    class ChunkHandle;
    class ChunkData;
    class WriteSettings;

    using ChunkConverterFn = void(*)(ChunkHandle&, WriteSettings&);


    void downdateBlocks(ChunkHandle& handle, WriteSettings& settings);

    void convertReadChunkToAquatic(ChunkHandle& handle, WriteSettings& settings);

    void convertReadChunkToElytra(ChunkHandle& handle, WriteSettings& settings);

    void convertReadChunkToPotions(ChunkHandle& handle, WriteSettings& settings);
}