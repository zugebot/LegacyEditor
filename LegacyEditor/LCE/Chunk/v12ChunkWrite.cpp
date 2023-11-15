#include "v12Chunk.hpp"



bool is0_128(u8* ptr) {
    u64* ptr64 = reinterpret_cast<u64*>(ptr);
    for (int i = 0; i < 16; ++i) {
        if (ptr64[i] != 0x0000000000000000) {
            return false;
        }
    }
    return true;
}

bool is255_128(u8* ptr) {
    u64* ptr64 = reinterpret_cast<u64*>(ptr);
    for (int i = 0; i < 16; ++i) {
        if (ptr64[i] != 0xFFFFFFFFFFFFFFFF) {
            return false;
        }
    }
    return true;
}


namespace universal {


    void V12Chunk::writeChunk(DataManager& managerOut, DIM) {
        dataManager = managerOut;

        // dataManager.writeInt32(chunkData.chunkX);
        // dataManager.writeInt32(chunkData.chunkZ);
        // dataManager.writeInt64(chunkData.lastUpdate);
        // dataManager.writeInt64(chunkData.inhabitedTime);
        // writeBlocks();
        writeLightData();
        // dataManager.writeBytes(chunkData.heightMap.data(), 256);
        // dataManager.writeInt16(chunkData.terrainPopulated);
        // dataManager.writeBytes(chunkData.biomes.data(), 256);
        // writeNBTData();
    }


    void V12Chunk::writeBlockData() {

    }

    void V12Chunk::writeLightSection(u8_vec& light, int& readOffset) {
        std::vector<int> sectionOffsets; // To store offsets of sections

        // Write headers
        u8* ptr = light.data() + readOffset;

        for (int i = 0; i < 128; i++) {
            if (is0_128(ptr)) {
                // if (std::all_of(ptr, ptr + 128, [](u8 v) { return v == 0; })) {
                dataManager.writeByte(128);
                // } else if (std::all_of(ptr, ptr + 128, [](u8 v) { return v == 255; })) {
            } else if (is255_128(ptr)) {
                dataManager.writeByte(129);
            } else {
                sectionOffsets.push_back(readOffset);
                dataManager.writeByte(sectionOffsets.size() - 1); // Use index as header
            }
            ptr += 128;
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

        printf("%d: size=%d\n", index, dataManager.getPosition());
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