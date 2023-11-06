#include <cassert>
#include <iostream>

#include "LegacyEditor/utils/dataManager.hpp"

#include "LegacyEditor/LCE/Chunk/include.h"
#include "LegacyEditor/LCE/Region/ChunkManager.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"
#include "LegacyEditor/LCE/SaveFile/ConsoleParser.hpp"
#include "LegacyEditor/LCE/SaveFile/fileListing.hpp"
#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/time.hpp"


int main() {
    /*
    {
        DataManager managerOut(14);
        managerOut.setBigEndian();
        u16 i1 = 0x1234;
        u32 i2 = 0x12345678;
        u64 i3 = 0x1234567812345678;
        managerOut.writeShort(i1);
        managerOut.writeInt(i2);
        managerOut.writeLong(i3);
        managerOut.writeToFile(dir_path + "data_out.bin");
        printf("%hu\n%i\n%llu\n", i1, i2, i3);
        // managerOut.writeWString("Hello World!", 64, true);
        DataManager managerIn;
        managerIn.setBigEndian();
        managerIn.readFromFile(dir_path + "data_out.bin");
        i1 = managerIn.readShort();
        i2 = managerIn.readInt();
        i3 = managerIn.readLong();
        printf("%hu\n%i\n%llu\n", i1, i2, i3);
        // auto stringOut = managerIn.readWAsString(64, true);
        // std::cout << stringOut << ", " << stringOut.size() << std::endl;
    }
     */
    // /*


    // Data data;
    // DataManager manager(data);
    // manager.readFromFile(dir_path + "bytes.bin");

    // Data out = Data(1000000);
    // memset(out.start(), 0, 1000000);

    // u32 size = 1000000;
    // int status = tinf_zlib_uncompress((Bytef*)out.start(),
    //                      &size,
    //                      (Bytef*)manager.start(),
    //                      manager.getSize());
    // std::cout << "status: " << status << std::endl;
    // std::cout << "out size: " << size << std::endl;

    // int y; std::cin >> y;






    std::string fileInPath = dir_path + "230918230206_in.wii";

    auto start = getMilliseconds();

    ConsoleParser parser;
    int status = parser.loadConsoleFile(fileInPath.c_str());

    FileListing fileListing(parser);

    fileListing.saveToFolder(dir_path + "dump\\");

    File* levelFilePtr = fileListing.levelFilePtr;

    // NBT::readTag(levelFilePtr);


    RegionManager region(CONSOLE::WIIU);
    region.read(fileListing.overworldFilePtrs[0]);
    // ChunkManager* chunk = region.getChunk(171);

    ChunkManager& chunk = region.chunks[0];
    for (int i = 1; i < 1024; i++) {
        if (region.chunks[i].size > chunk.size) {
            chunk = region.chunks[i];
            std::cout << "switched chosen chunk to " << i << std::endl;
        }
    }


    chunk.ensure_decompress(CONSOLE::WIIU);
    DataManager chunkOut(chunk);
    u16 version = chunkOut.readInt16();

    auto chunkParser = universal::V12Chunk();
    chunkParser.readChunk(chunkOut, DIM::OVERWORLD);

    // auto* compound = static_cast<NBTTagCompound*>(chunkParser.chunkData.NBTData->data);
    auto* compound = chunkParser.chunkData.NBTData->toType<NBTTagCompound>();
    for (const auto& pair : compound->tagMap) {
        printf("Key: %s, Type: %d\n", pair.first.c_str(), pair.second.type);
    }
    int x; std::cin >> x;

    chunk.ensure_compressed(CONSOLE::WIIU);


    DataManager chunkIn(chunk);
    chunkIn.writeToFile(dir_path + "chunk_in_0.bin");
    std::cout << chunk.size << std::endl;




    /*
    DataManager levelOut(fileListing.levelFilePtr);
    auto* levelData = NBT::readTag(levelOut);
    auto* root = static_cast<NBTTagCompound*>(levelData->data);
    NBTTagCompound* levelDataIn = root->getCompoundTag("Data");
    for (const auto& pair : levelDataIn->tagMap) {
        printf("Key: %s, Type: %d\n", pair.first.c_str(), pair.second.type);
    }
    levelOut.writeToFile(dir_path + "chunk_out.bin");
     */


    chunk.ensure_compressed(CONSOLE::WIIU);

    /*
    for (int i = 0; i < 1024; i++) {
        chunk->ensure_decompress(CONSOLE::WIIU);
        chunk->ensure_compressed(CONSOLE::WIIU);

    }

    DataManager chunkOut(chunk);
    chunkOut.writeToFile(dir_path + "chunk_out_0.bin");
    std::cout << chunk->size << std::endl;

    printf("hi");
    for (auto* regionFilePtr : fileListing.overworldFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
            int x = 0;
            std::cout << x;
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;
        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }
    for (auto* regionFilePtr : fileListing.netherFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;

        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }
    for (auto* regionFilePtr : fileListing.endFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;

        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }
     */
    Data dataOut = fileListing.write();
    // */
    /*
    // compare decompressed FileListing
    // DataManager(parser).writeToFile(dir_path + "in_" + "file_listing");
    // DataManager(dataOut).writeToFile(dir_path + "out_" + "file_listing");

    // test to ensure read and write data are the same
    // for (int i = 0; i < 1058310; i++) {
    //     u8 a = parser.data[i];
    //     u8 b = dataOut.data[i];
    //     if (a != b) {
    //         delete[] &a; // sysfaults
    //     }
    // }

    // int reg = 1;
    // File* filePtr = fileListing.overworldFilePtrs[reg];
    // std::cout << filePtr->name << std::endl;

    // Region region(CONSOLE::WIIU);
    // region.read(filePtr);

    // Chunk* chunk = region.getChunk(4, 15);
    // chunk->ensure_decompress(CONSOLE::WIIU);

    // DataManager chunkIn(chunk->data, chunk->size);
    // u16 version = chunkIn.readShort();
    // i32 xPos = (i32)chunkIn.readInt();
    // i32 zPos = (i32)chunkIn.readInt();
    // std::cout << version << " " << xPos << " " << zPos << std::endl;
    // chunkIn.seekStart();
    // chunk->ensure_compressed(CONSOLE::WIIU);

    // int count = -1;
    // for (Chunk& chunk : region.chunks) {
    //     count++;
    //     if (chunk.sectors == 0) continue;
    //     std::cout << count << std::endl;
    //     printf("[%4u, %1u] [%5u][%5u]\n", chunk.location, chunk.sectors, chunk.size, chunk.dec_size);
    // }

    // Data data = region.write(CONSOLE::WIIU);
    // fileListing.overworldFilePtrs[reg]->data = data.data;
    // fileListing.overworldFilePtrs[reg]->size = data.size;

    // DataManager(filePtr).writeToFile(dir_path + "in_" + filePtr->name);
    // DataManager(data).writeToFile(dir_path + "out_" + filePtr->name);
    */
    // /*

    std::string out_file = dir_path + "230918230206_out.wii";
    ConsoleParser::saveWiiU(out_file, dataOut);

    auto diff = getMilliseconds() - start;
    std::cout << "time: " << diff << "ms" << std::endl;

    return status;
     // */
}