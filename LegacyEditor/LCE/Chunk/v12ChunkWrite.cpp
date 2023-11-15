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
        std::vector<int> sectionOffsets;  // To store offsets of sections

        // Write headers
        u8* ptr = light.data() + readOffset;
        for (int i = 0; i < 128; i++) {
            if (std::all_of(ptr, ptr + 128, [](u8 v) { return v == 0; })) {
                dataManager.writeByte(128);
            } else if (std::all_of(ptr, ptr + 128, [](u8 v) { return v == 255; })) {
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

    void V12Chunk::writeLight(int& readOffset, u8_vec& light) {
        u8* startPtr = dataManager.ptr;
        dataManager.incrementPointer4(); // Placeholder for size

        writeLightSection(light, readOffset);

        // Calculate and write the size
        u8* endPtr = dataManager.ptr;
        i64 size = (endPtr - startPtr - 4 - 128) / 128;  // -4 to exclude the size field itself
        dataManager.ptr = startPtr;
        dataManager.writeInt32(static_cast<int>(size));
        dataManager.ptr = endPtr;
    }

    void V12Chunk::writeLightData() {
        int readOffset = 0;
        writeLight(readOffset, chunkData.skyLight);
        writeLight(readOffset, chunkData.skyLight);
        readOffset = 0;
        writeLight(readOffset, chunkData.blockLight);
        writeLight(readOffset, chunkData.blockLight);
    }


    void V12Chunk::writeNBTData() {
        if (chunkData.NBTData != nullptr) {
            NBT::writeTag(chunkData.NBTData, dataManager);
        }
    }


}