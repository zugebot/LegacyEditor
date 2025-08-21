#pragma once


namespace editor {

    class ChunkHandle;
    class WriteSettings;

    using ChunkConverterFn = void(*)(ChunkHandle&, WriteSettings&);

    void convertReadChunkToAquatic(ChunkHandle& handle, WriteSettings& settings);

    void convertReadChunkToElytra(ChunkHandle& handle, WriteSettings& settings);

    void convertReadChunkToPotions(ChunkHandle& handle, WriteSettings& settings);
}