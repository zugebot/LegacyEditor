#include <cassert>
#include <filesystem>
#include <iostream>
#include <thread>

#include <zlib.h>

#include "LegacyEditor/LCE/BinFile/BINSupport.hpp"
#include "LegacyEditor/LCE/Map/map.hpp"
#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/picture.hpp"

#include "LegacyEditor/LCE/FileListing/fileListing.hpp"
#include "LegacyEditor/LCE/MC/blocks.hpp"
#include "LegacyEditor/LCE/Region/Chunk/modifiers.hpp"
#include "LegacyEditor/LCE/Region/ChunkManager.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"
#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/RLESWITCH/rleswitch.hpp"
#include "LegacyEditor/utils/RLEVITA/rlevita.hpp"
#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/utils/mapcolors.hpp"
#include "LegacyEditor/utils/time.hpp"

#include "LegacyEditor/scripts/scripts.hpp"


 /*
DataManager dat;
    dat.readFromFile(TEST_OUT);
    unsigned long crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const unsigned char*)dat.data, dat.size);
    DataManager metadata;
    metadata.readFromFile(ps3_ + "METADATA");
    metadata.writeInt32AtOffset(0x04, dat.size);
    metadata.writeInt32AtOffset(0x0C, crc);
    metadata.writeToFile(ps3_ + "METADATA");
  */


std::string extractFileName(const std::string& path) {
    size_t dotDatPos = path.find('.');
    if (dotDatPos != std::string::npos) {
        return path.substr(0, dotDatPos);
    }
    return "";
}


std::string extractPart(const std::string& path) {
    size_t lastBackslashPos = path.find_last_of('\\');

    if (lastBackslashPos != std::string::npos) {
        // Return the substring from the character after the last backslash to the end
        return path.substr(lastBackslashPos + 1);
    }

    return ""; // Return empty string if the backslash is not found
}


namespace fs = std::filesystem;

typedef std::pair<std::string, std::string> strPair_t;
std::map<std::string, strPair_t> TESTS;

std::string wiiu = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
std::string ps3_ = R"(D:\Emulator Folders\rpcs3-v0.0.18-12904-12efd291_win64\dev_hdd0\home\00000001\savedata\NPUB31419--231212220825\)";

void TEST_PAIR(stringRef_t key, stringRef_t in, stringRef_t out) {
    std::string pathIn = dir_path + R"(tests\)" + in;
    TESTS.insert(std::make_pair(key, std::make_pair(pathIn, out)));
}

int main() {

    TEST_PAIR("superflat",   R"(superflat)"                                    , wiiu + R"(231105133853)");
    TEST_PAIR("aquatic_tut", R"(aquatic_tutorial)"                             , wiiu + R"(230918230206)");
    TEST_PAIR("vita",        R"(Vita Save\PCSB00560-231005063840\GAMEDATA.bin)", wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("elytra_tut",  R"()"                                             , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("NS_save1"  ,  R"(NS\190809160532.dat)"                          , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("fortnite",    R"(fortnite_world)"                               , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("rpcs3_flat",  R"(RPCS3_GAMEDATA)"                               , ps3_ + R"(GAMEDATA)");
    TEST_PAIR("X360_TU69",   R"(XBOX360_TU69.bin)"                             , dir_path + R"(tests\XBOX360_TU69.bin)" );
    TEST_PAIR("X360_TU74",   R"(XBOX360_TU74.dat)"                             , dir_path + R"(tests\XBOX360_TU74.dat)" );
    TEST_PAIR("nether"   ,   R"(nether)"                                       , wiiu + R"(231114151239)");

    std::string TEST_IN = TESTS["nether"].first;
    std::string TEST_OUT = TESTS["nether"].second;
    const CONSOLE consoleOut = CONSOLE::WIIU;


    // read savedata
    FileListing fileListing;
    int statusIn = fileListing.readFile(TEST_IN);
    if (statusIn) return printf_err("failed to load file\n");

    // edit fileListing
    /*
    fileListing.removeFileTypes({FileType::PLAYER,
                                 FileType::DATA_MAPPING,
                                 FileType::STRUCTURE});
                                 */
    fileListing.saveToFolder(); // debugging



    /*
    if (fileListing.console == CONSOLE::SWITCH) {
        std::string path = extractFileName(TEST.first);
        path += ".sub\\";


        std::map<std::string, int> compStart;
        std::map<std::string, int> index1Start;
        std::map<std::string, int> index2Start;

        try {
            // Check if the provided path is a directory
            if (fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    // Check if the entry is a file
                    if (fs::is_regular_file(entry)) {
                        std::cout << "\nFile: " << entry.path() << std::endl;


                        Data comp;
                        DataManager region(comp, false);
                        std::string name = entry.path().string();

                        region.readFromFile(name);

                        u32 mem_size = region.readInt32();


                        Data out(mem_size);
                        u32 sizeOut = RLESWITCH_DECOMPRESS(region.ptr, region.size -  4,
                                                           out.start(), out.size);

                        comp.deallocate();
                        Data GAMEDATA;
                        GAMEDATA.allocate(sizeOut);
                        memcpy(GAMEDATA.start(), out.start(), sizeOut);
                        out.deallocate();

                        DataManager GAMEDATA_MANAGER(GAMEDATA);
                        GAMEDATA_MANAGER.writeToFile(path + "DEC\\" + extractPart(name));

                        index1Start.insert(std::make_pair(name, swapEndian32(GAMEDATA_MANAGER.readInt32AtOffset(4))));
                        index2Start.insert(std::make_pair(name, swapEndian32(GAMEDATA_MANAGER.readInt32AtOffset(8))));

                        for (int x = 0; x < 4096; x++) {
                            if (GAMEDATA_MANAGER.data[x] == 0x78 && GAMEDATA_MANAGER.data[x + 1] == 0x9C) {
                                compStart.insert(std::make_pair(name, x));
                                break;
                            }
                        }
                    }
                }
            } else {
                std::cout << "The provided path is not a directory." << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }


        for (auto pair : compStart) {
            std::cout << pair.first << " " << pair.second << " " << index1Start[pair.first] << " " <<
                    index2Start[pair.first] << std::endl;
        }


        Data data;
        DataManager managerIn(data);
        managerIn.readFromFile(path + "GAMEDATA_00020000");
        managerIn.setLittleEndian();


        u32 pos;
        for (int x = 0; x < 4096; x++) {
            if (managerIn.data[x] == 0x78 && managerIn.data[x + 1] == 0x9C) {
                pos = x - 4;
            }
        }

        managerIn.seek(pos);

        ChunkManager chunk;
        chunk.size = managerIn.readInt32();
        chunk.setRLE(chunk.size & 1);
        chunk.setUnknown((chunk.size >> 1) & 1);
        chunk.size = (chunk.size & 0xFFFFFFFC) >> 8;
        chunk.allocate(chunk.size);
        chunk.setDecSize(10000);
        // chunk.setDecSize(managerIn.readInt32());
        memcpy(chunk.start(), managerIn.ptr, chunk.size);

        chunk.ensure_decompress(CONSOLE::SWITCH);
        chunk.readChunk(CONSOLE::SWITCH, DIM::OVERWORLD);




        return 0;
    }
    */


    // edit regions (threaded)
    auto start = getNanoSeconds();
    run_parallel<4>(removeNetherrack, std::ref(fileListing));
    // for (int ri = 0; ri < 4; ri++) processRegion(ri, fileListing);
    auto end = getNanoSeconds();
    printf("Total Time: %.3f\n", float(end - start) / float(1000000000));


    // fileListing.convertRegions(consoleOut);
    // convert to fileListing
    int statusOut = fileListing.writeFile(consoleOut, TEST_OUT);
    if (statusOut) {
        std::string str = "converting to " + consoleToStr(consoleOut) + " failed...\n";
        return printf_err(str.c_str());
    } else {
        printf("Finished!\nFile Out: %s", TEST_OUT.c_str());
    }


    return statusOut;
}







/*
std::set<u16> diff;
std::set_difference(_n11_2_blocks.begin(), _n11_2_blocks.end(),
    allBlocks.begin(), allBlocks.end(),
    std::inserter(diff, diff.begin()));

std::cout << "\n";
for (int elem : diff) {
    std::cout << elem << " - " << (elem >> 4) << ":" << (elem & 15) << std::endl;
}
bool foundChunk = false;
ChunkManager chunkToCopy;

for (int regionIndex = 0; regionIndex < 4; regionIndex++) {
    CONSOLE console = parser.console;
    // read a region file
    RegionManager region(fileListing.console);
    region.read(fileListing.region_overworld[regionIndex]);
    fileListing.region_overworld[regionIndex]->data.deallocate();


    for (ChunkManager& chunkManager : region.chunks) {

        if (chunkManager.sectors == 0) { continue; }
        if (chunkManager.data == nullptr) { continue; }

        if (!foundChunk) {
            foundChunk = true;
            chunkToCopy = chunkManager;
            chunkToCopy.data = chunkManager.data;
            chunkToCopy.size = chunkManager.size;
        } else {
            chunkManager.deallocate();
            chunkManager = chunkToCopy;
        }
    }

    fileListing.region_overworld[regionIndex]->data = region.write(console);
}
*/
/*
RegionManager region(fileListing.console);
region.read(fileListing.region_overworld[2]);
ChunkManager* chunkManager = region.getChunk(0, 0);
chunkManager->ensure_decompress(CONSOLE::WIIU);

universal::V12Chunk chunkParser;
auto real = DataManager(chunkManager);
real.writeToFile(dir_path + "chunk_read.bin");
u16 chunkVersion = real.readInt16();

chunkParser.readChunk(real, DIM::OVERWORLD);

u16 block2;
for (u16 x = 0; x < 16; x++) {
for (u16 z = 0; z < 16; z++) {
    for (u16 y = 66; y < 255; y++) {
        block2 = chunkParser.getBlock(x, y, z);

        // chunkParser.placeBlock(x, y, z, 0, 0);
    }
    //chunkParser.placeBlock(x, 239, z, 56, 0);
}
}

chunkParser.placeBlock(3, 253,  3, 56, 0);
chunkParser.placeBlock(4, 253,  2, 57, 0);
chunkParser.placeBlock(0, 253, 15, 58, 0);
chunkParser.placeBlock(7, 250,  7, 59, 0);

chunkParser.rotateUpsideDown();



Data out_lol(123456);

auto out = DataManager(out_lol);

chunkParser.writeChunk(out, DIM::OVERWORLD);


auto* compound = chunkParser.chunkData.NBTData->toType<NBTTagCompound>();
for (const auto& key : compound->tagMap) {
std::cout << key.first << std::endl;
}
std::cout << "Chunk: (" << chunkParser.chunkData.chunkX << ", " << chunkParser.chunkData.chunkZ << ")" << std::endl;

Data out_ptr(chunkParser.dataManager.getPosition());
memcpy(out_ptr.data, out_lol.data, out_ptr.size);

// DataManager(out_ptr).writeToFile(dir_path + "chunk_write.bin");

delete[] out_lol.data;
out_lol.data = nullptr;

delete[] chunkManager->data;
chunkManager->data = nullptr;

chunkManager->data = out_ptr.data;
chunkManager->size = out_ptr.size;
chunkManager->dec_size = chunkManager->size;


universal::V12Chunk chunkParser2;
auto real2 = DataManager(chunkManager);

u16 chunkVersion2 = real2.readInt16();

chunkParser2.readChunk(real2, DIM::OVERWORLD);


chunkManager->ensure_compressed(CONSOLE::WIIU);
Data new_region = region.write(CONSOLE::WIIU);
fileListing.region_overworld[2]->data = new_region;
*/
/*
// ConsoleParser parser2;
// int g = parser2.readConsoleFile(outFilePath);
// FileListing listing(parser2);
// listing.saveToFolder(dir_path + "wiiu_out_recreate_names");

*
for (int i = 0; i < fileListing.overworld.size(); i++) {
std::cout << fileListing.overworld[2]->name << std::endl;
}

DataManager manager;
manager.readFromFile(dir_path + "180809114549.dat");
manager.setLittleEndian();
manager.readInt32();
u32 decomp_size = manager.readInt32();

u8* data = new u8[decomp_size];
tinf_zlib_uncompress((Bytef*) data, &decomp_size, (Bytef*) manager.ptr, manager.size);
DataManager out(data, decomp_size);



out.writeToFile(dir_path + "180809114549_dec.dat");
*/
/*

// int status = ConsoleParser().convertAndReplaceRegions(inFilePath1, inFilePath2, outFilePath, CONSOLE::WIIU);
int status = ConsoleParser().convertTo(inFilePath2, outFilePath, CONSOLE::WIIU);
if (status == 0) {
std::cout << "Converted!" << std::endl;
} else {
std::cout << "Failed to convert..." << std::endl;
}
*/
/*
// ConsoleParser parser;
// int status = parser.readConsoleFile(inFilePath2);
// if (status != 0) return status;
// FileListing fileListing(parser); // read  file listing
// fileListing.saveToFolder(dir_path + "dump_" + consoleToStr(CONSOLE::RPCS3));
// RegionManager region(CONSOLE::RPCS3);
// DataManager regionIn;
// regionIn.readFromFile(dir_path + R"(dump_rpcs3\r.0.0.mcr)");
// auto LOL = Data(regionIn.data, regionIn.size);
// region.read(&LOL);
// ChunkManager* chunk = region.getNonEmptyChunk();
// chunk->ensure_decompress(CONSOLE::RPCS3);
// DataManager chunkOut(chunk);
// chunkOut.writeToFile(dir_path + "rpcs3_chunk_dec.bin");
*/
/*
ConsoleParser out;
out.readConsoleFile(outFilePath);
FileListing files(out);
files.saveToFolder(dir_path + "dump_vita_wiiu");


RegionManager region(CONSOLE::WIIU);
DataManager regionIn;
regionIn.readFromFile(dir_path + R"(dump_vita_wiiu\r.0.0.mcr)");
auto LOL = Data(regionIn.data, regionIn.size);
region.read(&LOL);
ChunkManager* chunk = region.getChunk(13, 16);
chunk->ensure_decompress(CONSOLE::WIIU);

universal::V12Chunk chunkParser;
auto real = DataManager(chunk);
u16 chunkVersion = real.readInt16();
chunkParser.readChunk(real, DIM::OVERWORLD);

int x = chunkParser.chunkData.blocks.size();
for (int i = 0; i < chunkParser.chunkData.blocks.size(); i++) {
u16 block = chunkParser.chunkData.blocks[i];
if (block != 0) {
    std::cout << "Found a block at " << i << std::endl;
    std::cin >> x;
}
}
std::cout << "finished reading" << std::endl;
*/
/*
// ConsoleParser parserWiiU;
// int status1 = parserWiiU.readConsoleFile(wiiuFilePath);
// FileListing fileListingWiiU(parserWiiU.console, parserWiiU);
// fileListingWiiU.saveToFolder(dir_path + "dump_wiiu");
// auto* filewiiu = fileListingWiiU.levelFilePtr;
// DataManager playerDatawiiu(filewiiu->data);
// auto datawiiu = NBT::readTag(playerDatawiiu);
// std::string playerNBTStringwiiu = datawiiu->toString();

// ConsoleParser parserVita;
// int status2 = parserVita.readConsoleFile(vitaFilePath);
// FileListing fileListingVita(parserVita.console, parserVita);
// fileListingVita.removeAllPlayers();
// // fileListingVita.deleteAllChunks();
// fileListingVita.removeAllStructures();
// fileListingVita.saveToFolder(dir_path + "dump_vita");
// RegionManager region(CONSOLE::VITA);
// region.read(fileListingVita.overworldFilePtrs[0]);
// auto chunk = region.chunks[602];
// chunk.ensure_decompress(CONSOLE::WIIU);
// DataManager chunkOut(chunk);
// u16 version = chunkOut.readInt16();
//universal::V12Chunk readChunk;
//readChunk.readChunk(chunkOut, DIM::OVERWORLD);
//chunkOut.writeToFile(dir_path + "chunk_out_dec.bin");


// auto* filevita = fileListingVita.levelFilePtr;
// DataManager playerDatavita(filevita->data);
// auto datavita = NBT::readTag(playerDatavita);
// std::string playerNBTStringvita = datavita->toString();
// std::cout << playerNBTStringwiiu << std::endl;
// std::cout << "\n\n\n" << std::endl;
// std::cout << playerNBTStringvita << std::endl;
// compareNBT(datawiiu, datavita);
*/
/*
saveMapToPng(fileListingVita.mapFilePtrs[0], dir_path);

for (File* file : fileListingVita.dimensionFilePtrs) {
RegionManager regionIter(CONSOLE::VITA);
regionIter.read(file);
Data data = regionIter.write(CONSOLE::WIIU);
delete[] file->data.data;
file->data = data;
}
*/
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
/*
// chunk.ensure_compressed(CONSOLE::WIIU);
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
/*
// Data dataOut = fileListingWiiU.write(CONSOLE::WIIU);
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