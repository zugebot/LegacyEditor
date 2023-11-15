#include "v12Chunk.hpp"


namespace universal {


    void V12Chunk::writeChunk(DataManager& managerOut, DIM) {
        dataManager = managerOut;

        dataManager.writeInt32(chunkData.chunkX);
        dataManager.writeInt32(chunkData.chunkZ);
        dataManager.writeInt64(chunkData.lastUpdate);
        dataManager.writeInt64(chunkData.inhabitedTime);
        writeBlocks();
        writeLightData();
        dataManager.writeBytes(chunkData.heightMap.data(), 256);
        dataManager.writeInt16(chunkData.terrainPopulated);
        dataManager.writeBytes(chunkData.biomes.data(), 256);
        writeNBTData();
    }


    void V12Chunk::writeBlocks() {

    }

    void V12Chunk::writeLightSection(u8_vec& light, int& readOffset) {
        std::vector<int> sectionOffsets; // To store offsets of sections

        // Write headers
        for (int i = 0; i < 128; i++) {
            auto begin = light.begin() + readOffset;
            auto end = begin + 128;
            if (std::all_of(begin, end, [](u8 v) { return v == 0; })) {
                dataManager.writeByte(128);
            } else if (std::all_of(begin, end, [](u8 v) { return v == 255; })) {
                dataManager.writeByte(129);
            } else {
                sectionOffsets.push_back(readOffset);
                dataManager.writeByte(sectionOffsets.size() - 1); // Use index as header
            }
            readOffset += 128;
        }

        // Write light data sections
        for (int offset : sectionOffsets) {
            dataManager.writeBytes(&light[offset], 128);
        }
    }

    void V12Chunk::writeLight(int index, int& readOffset, u8_vec& light) {
        u8* startPtr = dataManager.ptr;
        dataManager.writeInt32(0); // Placeholder for size

        writeLightSection(light, readOffset);

        // Calculate and write the size
        u8* endPtr = dataManager.ptr;
        i64 size = (endPtr - startPtr - 4) / 128;  // -4 to exclude the size field itself
        dataManager.ptr = startPtr;
        dataManager.writeInt32(static_cast<int>(size));
        dataManager.ptr = endPtr;

        dataManager.writeToFile(dataManager.data, dataManager.getPosition(), dir_path + "light_" + std::to_string(index) + ".bin");
    }

    void V12Chunk::writeLightData() {
        Data _data(123456);
        DataManager data(_data);

        int readOffset = 0;
        writeLight(0, readOffset, chunkData.skyLight);
        writeLight(1, readOffset, chunkData.skyLight);
        readOffset = 0;
        writeLight(2, readOffset, chunkData.blockLight);
        writeLight(3, readOffset, chunkData.blockLight);

        delete[] _data.data;
    }



    void V12Chunk::writeNBTData() {

    }






}