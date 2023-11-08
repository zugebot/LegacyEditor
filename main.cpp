#include <cassert>
#include <iostream>

#include "LegacyEditor/utils/dataManager.hpp"

#include "LegacyEditor/LCE/Chunk/include.h"
#include "LegacyEditor/LCE/Region/ChunkManager.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"
#include "LegacyEditor/LCE/SaveFile/ConsoleParser.hpp"
#include "LegacyEditor/LCE/SaveFile/fileListing.hpp"
#include "LegacyEditor/utils/NBT/include.hpp"
#include "LegacyEditor/utils/picture.hpp"
#include "LegacyEditor/utils/time.hpp"


void compareNBT(NBTBase* first, NBTBase* second) {
    auto* firstNBT = NBTBase::toType<NBTTagCompound>(first)->getCompoundTag("Data");
    auto* secondNBT = NBTBase::toType<NBTTagCompound>(second)->getCompoundTag("Data");;

    for (const auto& tag : firstNBT->tagMap) {
        if (!secondNBT->hasKey(tag.first)) {
            printf("second does not contain tag '%s'\n", tag.first.c_str());
        }
    }

    for (const auto& tag : secondNBT->tagMap) {
        if (!firstNBT->hasKey(tag.first)) {
            printf("first does not contain tag '%s'\n", tag.first.c_str());
        }
    }



}




int main() {
    auto start = getMilliseconds();

    std::string wiiuFilePath = dir_path + "tests/Pirates.wii";
    std::string vitaFilePath = dir_path + "tests/GAMEDATA_VITA.bin";

    ConsoleParser parserWiiU;
    int status1 = parserWiiU.readConsoleFile(wiiuFilePath);
    FileListing fileListingWiiU(parserWiiU.console, parserWiiU);
    fileListingWiiU.saveToFolder(dir_path + "dump_wiiu");


    auto* filewiiu = fileListingWiiU.levelFilePtr;
    DataManager playerDatawiiu(filewiiu->data);
    auto datawiiu = NBT::readTag(playerDatawiiu);
    std::string playerNBTStringwiiu = datawiiu->toString();

    ConsoleParser parserVita;
    int status2 = parserVita.readConsoleFile(vitaFilePath);
    FileListing fileListingVita(parserVita.console, parserVita);
    fileListingVita.saveToFolder(dir_path + "dump_vita");

    auto* filevita = fileListingVita.levelFilePtr;
    DataManager playerDatavita(filevita->data);
    auto datavita = NBT::readTag(playerDatavita);
    std::string playerNBTStringvita = datavita->toString();


    //std::cout << playerNBTStringwiiu << std::endl;
    // std::cout << "\n\n\n" << std::endl;
    // std::cout << playerNBTStringvita << std::endl;
    compareNBT(datawiiu, datavita);


    auto* map = fileListingVita.mapFilePtrs[0];
    DataManager mapManager(map->data);
    auto data = NBT::readTag(mapManager);
    // std::cout << data->toString() << std::endl;
    auto* mapCompound = NBTBase::toType<NBTTagCompound>(data)->getCompoundTag("data");
    auto* byteArray = mapCompound->getByteArray("colors");
    std::cout << byteArray->size << std::endl;

    Picture picture(128, 128);
    int count = 0;
    for (int i = 0; i < 16384; i++) {
        picture.data[count++] = byteArray->array[i];
        picture.data[count++] = byteArray->array[i];
        picture.data[count++] = byteArray->array[i];
    }


    picture.saveWithName("map_0.png", dir_path);




    // fileListingWiiU.deleteAllChunks();


    // Data dataOutVita = fileListingWiiU.write(CONSOLE::WIIU);
    // parser.saveWiiU(dir_path + "tests/230918230206", dataOutVita);

    int z; std::cin >> z;


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
    /*


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
    // std::string fileInPath = dir_path + "230918230206.wii";
    */

    /*

    auto* player = fileListingWiiU.structureFilePtrs[0]; // playerFilePtrs[0];
    DataManager playerData(player->data);
    auto data = NBT::readTag(playerData);
    std::string playerNBTString = data->toString();
    std::cout << playerNBTString << std::endl;
    */

    /*
    File* levelFilePtr = fileListingWiiU.levelFilePtr;

    // NBT::readTag(levelFilePtr);


    RegionManager region(CONSOLE::WIIU);
    region.read(fileListingWiiU.overworldFilePtrs[0]);
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
    */
    /*
    DataManager levelOut(fileListingWiiU.levelFilePtr);
    auto* levelData = NBT::readTag(levelOut);
    auto* root = static_cast<NBTTagCompound*>(levelData->data);
    NBTTagCompound* levelDataIn = root->getCompoundTag("Data");
    for (const auto& pair : levelDataIn->tagMap) {
        printf("Key: %s, Type: %d\n", pair.first.c_str(), pair.second.type);
    }
    levelOut.writeToFile(dir_path + "chunk_out.bin");
     */
    // chunk.ensure_compressed(CONSOLE::WIIU);
    /*
    for (int i = 0; i < 1024; i++) {
        chunk->ensure_decompress(CONSOLE::WIIU);
        chunk->ensure_compressed(CONSOLE::WIIU);

    }

    DataManager chunkOut(chunk);
    chunkOut.writeToFile(dir_path + "chunk_out_0.bin");
    std::cout << chunk->size << std::endl;

    printf("hi");
    for (auto* regionFilePtr : fileListingWiiU.overworldFilePtrs) {
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
    for (auto* regionFilePtr : fileListingWiiU.netherFilePtrs) {
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
    for (auto* regionFilePtr : fileListingWiiU.endFilePtrs) {
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
    // Data dataOut = fileListingWiiU.write(CONSOLE::WIIU);
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
    // File* filePtr = fileListingWiiU.overworldFilePtrs[reg];
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
    // fileListingWiiU.overworldFilePtrs[reg]->data = data.data;
    // fileListingWiiU.overworldFilePtrs[reg]->size = data.size;

    // DataManager(filePtr).writeToFile(dir_path + "in_" + filePtr->name);
    // DataManager(data).writeToFile(dir_path + "out_" + filePtr->name);
    */
    /*

    std::string out_file = dir_path + "230918230206_out.wii";
    ConsoleParser::saveWiiU(out_file, dataOut);

    auto diff = getMilliseconds() - start;
    std::cout << "time: " << diff << "ms" << std::endl;

     */
    return 0;
}