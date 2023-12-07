#include <cassert>
#include <filesystem>
#include <iostream>
#include <thread>

#include "LegacyEditor/utils/dataManager.hpp"

#include "LegacyEditor/LCE/BinFile/BINSupport.hpp"
#include "LegacyEditor/LCE/MC/blocks.hpp"
#include "LegacyEditor/LCE/Map/Map.hpp"
#include "LegacyEditor/LCE/Region/Chunk/modifiers.hpp"
#include "LegacyEditor/LCE/Region/ChunkManager.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"
#include "LegacyEditor/LCE/SaveFile/ConsoleParser.hpp"
#include "LegacyEditor/LCE/SaveFile/fileListing.hpp"
#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/mapcolors.hpp"
#include "LegacyEditor/utils/picture.hpp"
#include "LegacyEditor/utils/time.hpp"


typedef std::pair<std::string, std::string> strPair_t;


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


/// Function to shuffle an array using Fisher-Yates algorithm
void shuffleArray(uint16_t arr[], int size) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = size - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        std::swap(arr[i], arr[j]);
    }
}


int toBlock(int x, int y, int z) {
    return y + 256 * z + 4096 * x;
}


// DataManager in_ext;
// in_ext.readFromFile(dir_path + R"(tests\WiiU Save\231008144148.ext)");
// in_ext.data = in_ext.data + 0x100;
// in_ext.ptr = in_ext.data;
// in_ext.size -= 0x100;
// WorldOptions options = getTagsInImage(in_ext);



void processRegion(CONSOLE parserConsole, int regionIndex, FileListing& fileListing) {
    const CONSOLE console = parserConsole;

    // read a region file
    RegionManager region(fileListing.console);
    region.read(fileListing.region_overworld[regionIndex]);

    int h = -1;
    for (ChunkManager& chunkManager : region.chunks) {
        h++;
        if (chunkManager.size == 0) { continue; }

        chunkManager.ensure_decompress(console);
        chunkManager.readChunk(DIM::OVERWORLD);
        auto* chunkData = chunkManager.chunkData;

        std::cout << "Chunk: " << chunkData->getCoords() << " " << chunkData->lastVersion;
        if (chunkData->hasSubmerged) { std::cout << " (Submerged)"; }
        std::cout << std::endl;

        /*
        // chunkParser.convertOldToNew();
        i32 chunkX = chunkData.chunkX, chunkZ = chunkData.chunkZ;
        if (chunkX == -11 && chunkZ == 2) {
            for (int x = 0; x < 65536; x++) {
                u16 block = chunkData.newBlocks[x];

                // above 150
                if (block >> 4 > 30) {
                    if ((block & 15) != 0) {
                        std::cout << (block >> 4) << ":" << (block & 15) << std::endl;
                    }
                    chunkData.newBlocks[x] = 0;
                }


                _n11_2_blocks.insert(chunkData.newBlocks[x]);
            }
            int ertgije; std::cin >> ertgije;
        } else if (chunkX > -13 && chunkX < -9 && chunkZ < 4 && chunkZ > -3) {
            for (int x = 0; x < 65536; x++) {
                allBlocks.insert(chunkData.newBlocks[x]);
            }
        }
        // std::cout << chunkData.NBTData->toString() << std::endl;
        // chunkDataIn.writeToFile(dir_path + "chunk_read.bin");
        // /
        */


        u16 blocks[65536];
        for (u16 x = 0; x < 16; x++) {
            for (u16 z = 0; z < 16; z++) {
                for (u16 y = 0; y < 256; y++) {
                    u16 block1 = chunk::getBlock(chunkManager.chunkData, x, y, z);
                    u16 data_1 = 0;
                    int offset1 = y + 256 * z + 4096 * x;

                    /*
                    u16 block2 = block1;
                    int offset2 = 255 - y + 256 * z + 4096 * x;
                    u16 compare1 = (block1 & 0x1FF0) >> 4;
                    if (block1 & 0x8000) { // fix stupid blocks
                        if (compare1 == 271) { // sea pickle
                            block1 = (block1 & 0x9FF7) | 0x08;
                        }
                        if (compare1 == 272) { // bubble column
                            block1 = (block1 & 0x7FFF) | 0b1111;
                        }
                    }

                    u16 compare2;
                    if (block2 & 0x8000) {
                        block2 &= 0x1FFF;
                        compare2 = (block2 & 0x1FF0) >> 4;
                        if (compare2 == 271) { // sea pickle
                            // block2 = 0;
                        }
                        if (compare2 == 272) { // bubble column
                            // block2 = 0;
                        }
                    }*/

                    block1 = BlockID::AIR_ID;
                    if (y == 0) { block1 = BlockID::BEDROCK_ID; }

                    u32 jumpBL, jumpDA;

                    if (y == 1) {
                        if (x % 2 != 0 || z % 2 != 0) {
                            if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                                chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                                block1 = BlockID::AIR_ID;
                                goto END;
                            }
                            block1 = BlockID::BEDROCK_ID;
                            goto END;
                        }
                    }

                    if (y == 2) {
                        if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                            chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                            goto END;
                        }

                        if (x % 2 != 0 || z % 2 != 0) {
                            goto END;
                        }

                        jumpBL = 8 * (chunkData->chunkX + 16);
                        jumpDA = 8 *  chunkData->chunkZ;

                        block1 = (jumpBL + x / 2) << 4;
                        data_1 = jumpDA + z / 2;

                        if (block1 > 252 << 4 || block1 == BlockID::BEACON_ID) {
                            block1 = 0;
                            data_1 = 0;
                        }
                    }

                    END:

                    blocks[offset1] = block1 | data_1;
                }
            }
        }

        memcpy(&chunkData->newBlocks[0], &blocks[0], 131072);

        memset(&chunkData->biomes[0], 0x0B, 256);
        memset(&chunkData->blockLight[0], 0xFF, 32768);
        memset(&chunkData->heightMap[0], 0xFF, 256);
        memset(&chunkData->skyLight[0], 0xFF, 32768);
        chunkData->terrainPopulated = 2046;
        chunkData->lastUpdate = 100;
        chunkData->inhabitedTime = 200;

        int x;


        /*
        // chunkParser.placeBlock(4, 158, 4, 8, 0, true);
        // chunkParser.placeBlock(4, 158, 8, 9, 0, true);
        // chunkParser.placeBlock(8, 158, 8, 10, 0, true);
        // chunkParser.placeBlock(8, 158, 4, 11, 0, true);

        // for (u16 y = 0; y < 256; y++) {
        //     blocks[y] = 2 << 4;
        // }

        // memset(&chunkData.blockLight[0], 0xFF, 32768);
        // memset(&chunkData.skyLight[0], 0xFF, 32768);
        */

        if (chunkData->NBTData != nullptr) {
            chunkData->NBTData->toType<NBTTagCompound>()->deleteAll();
            delete chunkData->NBTData;
        }

        chunkData->NBTData = new NBTBase(new NBTTagCompound(), TAG_COMPOUND);
        auto* chunkRootNbtData = static_cast<NBTTagCompound*>(chunkData->NBTData->data);
        auto* entities = new NBTTagList();
        auto* tileEntities = new NBTTagList();
        auto* tileTicks = new NBTTagList();
        chunkRootNbtData->setListTag("Entities", entities);
        chunkRootNbtData->setListTag("TileEntities", tileEntities);
        chunkRootNbtData->setListTag("TileTicks", tileTicks);
        chunkManager.writeChunk(DIM::OVERWORLD);
        chunkManager.ensure_compressed(console);

    }

    fileListing.region_overworld[regionIndex]->data.deallocate();
    fileListing.region_overworld[regionIndex]->data = region.write(console);

}




int main() {
    dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";

    std::map<std::string, strPair_t> TESTS = {
        {"superflat", std::make_pair(dir_path + R"(tests\superflat)", R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\231105133853)")},
        {"aquatic_tut", std::make_pair(dir_path + R"(tests\aquatic_tutorial)", R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\231105133853)")},
        {"vita", std::make_pair(dir_path + R"(tests\Vita Save\PCSB00560-231005063840\GAMEDATA.bin)", R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\BLANK_SAVE)")},
        {"elytra_tut", std::make_pair(dir_path + R"()", R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\BLANK_SAVE)")},
    };

    strPair_t TEST = TESTS["vita"];


    // read savedata
    ConsoleParser parser;
    int statusIn = parser.readConsoleFile(TEST.first);
    if (statusIn) {
        printf("failed to load file\n");
        return -1;
    }


    // edit fileListing
    FileListing fileListing(parser);
    fileListing.removeFileTypes({FileType::PLAYER, FileType::DATA_MAPPING, FileType::STRUCTURE});
    fileListing.saveToFolder(dir_path + "dump_wiiu");


    // edit regions
    auto start = getNanoSeconds();
    std::vector<std::thread> threads;
    for (int regionIndex = 0; regionIndex < 4; regionIndex++)
        threads.emplace_back(processRegion, parser.console, regionIndex, std::ref(fileListing));
    for (auto& thread : threads) { if (thread.joinable()) { thread.join(); } }
    auto end = getNanoSeconds();
    printf("Total Time: %.3f\n", float(end - start) / float(1000000000));


    // convert to fileListing
    const CONSOLE consoleOut = CONSOLE::WIIU;
    fileListing.convertRegions(consoleOut);
    Data dataOut = fileListing.write(consoleOut);
    int statusOut = ConsoleParser::saveWiiU(TEST.second, dataOut);
    if (statusOut) {
        printf("converting to wiiu failed...\n");
    } else {
        printf("finished!\n");
    }

    return 0;
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