/*
auto *file = fileListing.entity_overworld;
DataManager entityManager(file->data);
u32 i1 = entityManager.readInt32();
for (int index = 0; index < i1; index++) {
    i32 i2 = (i32)entityManager.readInt32();
    i32 i3 = (i32)entityManager.readInt32();
    auto* base = NBT::readTag(entityManager);
}


DataManager in_ext;
in_ext.readFromFile(dir_path + R"(tests\WiiU Save\231008144148.ext)");
in_ext.data = in_ext.data + 0x100;
in_ext.ptr = in_ext.data;
in_ext.size -= 0x100;
WorldOptions options = getTagsInImage(in_ext);




#include <zlib.h>
DataManager dat;
dat.readFromFile(TEST_OUT);
unsigned long crc = crc32(0L, Z_NULL, 0);
crc = crc32(crc, (const unsigned char*)dat.data, dat.size);
DataManager metadata;
metadata.readFromFile(ps3_ + "METADATA");
metadata.writeInt32AtOffset(0x04, dat.size);
metadata.writeInt32AtOffset(0x0C, crc);
metadata.writeToFile(ps3_ + "METADATA");



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
        return path.substr(lastBackslashPos + 1);
    }
    return "";
}
*/



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
    lce::CONSOLE console = parser.console;
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