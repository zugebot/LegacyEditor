#include "v12Chunk.hpp"


namespace universal {


    void V12Chunk::writeChunk(DataManager& managerOut, DIM) {
        dataManager = managerOut;

        dataManager.writeInt32(chunkData.chunkX);
        dataManager.writeInt32(chunkData.chunkZ);
        dataManager.writeInt64(chunkData.lastUpdate);
        dataManager.writeInt64(chunkData.inhabitedTime);
        writeBlocks();
        writeLights();
        dataManager.write(chunkData.heightMap.data(), 256);
        dataManager.writeInt16(chunkData.terrainPopulated);
        dataManager.write(chunkData.biomes.data(), 256);
        writeNBTData();
    }


    void V12Chunk::writeBlocks() {

    }


    void V12Chunk::writeLights() {

    }


    void V12Chunk::writeNBTData() {

    }






}