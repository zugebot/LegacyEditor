#include "lce/processor.hpp"

#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"
#include "include/png/crc.hpp"
#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"

int main() {
    PREPARE_UNIT_TESTS();

    {
        fs::path IN_RPCS3 = fs::path(TESTS["RPCS3_1"].first).parent_path();
        IN_RPCS3 /= "PARAM.SFO";
        IN_RPCS3 = IN_RPCS3.make_preferred();

        SFOManager sfo_rpcs3(IN_RPCS3.string());

        std::cout << "** RPCS3 PARAM.SFO **" << std::endl;
        auto attrs_rpcs3 = sfo_rpcs3.getAttributes();
        for (auto& attr: attrs_rpcs3) {
            std::cout << attr.toString() << std::endl;
        }


        std::cout << "\n\n\n";

        fs::path IN_PS3 = fs::path(TESTS["PS3_1"].first).parent_path();
        IN_PS3 /= "PARAM.SFO";
        IN_PS3 = IN_PS3.make_preferred();

        SFOManager sfo_ps3(IN_PS3.string());

        std::cout << "** PS3 PARAM.SFO **" << std::endl;
        auto attrs_ps3 = sfo_ps3.getAttributes();
        for (auto& attr: attrs_ps3) {
            std::cout << attr.toString() << std::endl;
        }
    }

    //fs::path IN = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\240617125342)";

    std::string IN = TESTS["X360_TU74"].first;
    editor::FileListing theListing;
    int status = theListing.read(IN);
    if (status != 0) return printf_err(status, "failed to read listing\n");

    status = theListing.dumpToFolder(R"(C:\\Users\\jerrin\\Desktop\\OUT\\)");
    if (status != 0) return printf_err(status, "failed to dump listing\n");

    fs::path OUT = R"(C:\Users\jerrin\Desktop\OUT\GAMEDATA)";
    status = theListing.write(OUT, lce::CONSOLE::RPCS3);
    if (status != 0) return printf_err(status, "failed to write listing\n");









    /*
    MU auto IN_WII = R"(C:\Users\jerrin\Desktop\New folder\WIIU\240617125342)";

    MU auto OUT_WIIU = R"(C:\Users\jerrin\Desktop\New folder\OUT\WIIU)";
    MU auto OUT_RPCS3 = R"(C:\Users\jerrin\Desktop\New folder\OUT\GAMEDATA)";
    MU auto OUT_RPCS3_TO_WIIU = R"(C:\Users\jerrin\Desktop\New folder\OUT\WIIU_TO_RPCS3_TO_WIIU)";


    editor::FileListing readPS3;
    int status = readPS3.read(TESTS["PS3_1"].first);
    if (status != 0) return printf_err(status, READ_ERROR, OUT_WIIU);


    readPS3.removeFileTypes({
            lce::FILETYPE::DATA_MAPPING,
            lce::FILETYPE::PLAYER,
    });

    readPS3.printDetails();

    status = readPS3.dumpToFolder(R"(C:\Users\jerrin\Desktop\New folder\PS3)");
    if (status != 0) return status;

    NBTBase* nbt;
    DataManager manager(readPS3.level->data);
    nbt = NBT::readTag(manager);
    std::string str = nbt->toString();
    std::cout << str << std::endl;

    status = readPS3.write(IN_WII, lce::CONSOLE::WIIU);
    if (status != 0) return printf_err(status, READ_ERROR, IN_WII);

    editor::FileListing readWiiU;
    status = readWiiU.read(IN_WII);
    if (status != 0) return printf_err(status, READ_ERROR, IN_WII);

    status = readWiiU.dumpToFolder(R"(C:\Users\jerrin\Desktop\New folder\WIIU)");
    if (status != 0) return status;
    */

    /*
    auto IN_PS3 = R"(C:\Users\jerrin\Desktop\New folder\PS3\GAMEDATA)";
    // std::string OUT_STUFF = R"(C:\Users\jerrin\Desktop\New folder\CHUNKS\)";
    // auto consoleOut = lce::CONSOLE::WIIU;
    editor::FileListing ps3_Save;
    editor::FileListing convSave;
    if (int status = ps3_Save.read(IN_PS3); status != 0)
        return printf_err(status, READ_ERROR, IN_PS3);

    // load temp WiiU

    editor::FileListing wiiUSaveTemp;
    if (int status = wiiUSaveTemp.read(IN_WII); status != 0)
        return printf_err(status, READ_ERROR, IN_WII);

    wiiUSaveTemp.printDetails();


    //// write temp WiiU
    if (int stat = readPS3.write(OUT_WIIU, lce::CONSOLE::WIIU); stat != 0)
        return printf_err(stat, WRITE_ERROR, consoleToCStr(lce::CONSOLE::RPCS3));
    */
    /*
    // load WiiU
    editor::FileListing wiiUSave;
    if (int status = wiiUSave.read(OUT_WIIU); status != 0)
        return printf_err(status, READ_ERROR, OUT_WIIU);

    wiiUSave.printDetails();


    // write as WiiU -> RPCS3
    if (int stat = wiiUSave.write(OUT_RPCS3, lce::CONSOLE::RPCS3); stat != 0)
        return printf_err(stat, WRITE_ERROR, consoleToCStr(lce::CONSOLE::RPCS3));
    */
    // load RPCS3
    /*
    editor::FileListing rpcs3SaveTemp;
    if (int status = rpcs3SaveTemp.read(OUT_RPCS3); status != 0)
        return printf_err(status, READ_ERROR, OUT_WIIU);

    rpcs3SaveTemp.printDetails();


    // write as RPCS3 -> WiiU
    if (int stat = rpcs3SaveTemp.write(OUT_RPCS3_TO_WIIU, lce::CONSOLE::WIIU); stat != 0)
        return printf_err(stat, WRITE_ERROR, consoleToCStr(lce::CONSOLE::WIIU));
    */
    // // read final converted
    // editor::FileListing final;
    // if (int status = final.read(OUT_RPCS3_TO_WIIU); status != 0)
    //     return printf_err(status, READ_ERROR, OUT_RPCS3_TO_WIIU);
    /*
    // auto types = {
    //         lce::FILETYPE::GRF,
    //         lce::FILETYPE::MAP,
    //         lce::FILETYPE::VILLAGE,
    //         lce::FILETYPE::REGION_NETHER,
    //         lce::FILETYPE::REGION_END
    // };
    // ps3_Save.removeFileTypes(types);
    // wiiUSave.removeFileTypes(types);


    // editor::FileListing listing;
    // int status1 = listing.convertAndReplaceRegions(IN_PS3, IN_WII, OUT_CONV, consoleOut);
    // if (status1 != 0)
    //     return printf_err(status1, WRITE_ERROR, consoleToCStr(consoleOut));
    // return 0;
     */
    /*
    // we now have 3 saves loaded, ps3, wiiu, and ps3->wiiu
    // now find the difference

    editor::RegionManager ps3_Region;
    editor::RegionManager wiiURegion;
    editor::RegionManager convRegion;
    ps3_Region.read(ps3_Save.region_overworld[2]);
    wiiURegion.read(wiiUSave.region_overworld[2]);
    convRegion.read(convSave.region_overworld[2]);

    editor::ChunkManager* ps3_Chunk = ps3_Region.getNonEmptyChunk();
    editor::ChunkManager* wiiUChunk = wiiURegion.getNonEmptyChunk();
    editor::ChunkManager* convChunk = convRegion.getNonEmptyChunk();

    DataManager(ps3_Chunk->data, ps3_Chunk->size).writeToFile(OUT_STUFF + "1_BASE_ps3_Chunk" + ps3_Chunk->getDataAsString());
    DataManager(wiiUChunk->data, wiiUChunk->size).writeToFile(OUT_STUFF + "1_BASE_wiiUChunk" + wiiUChunk->getDataAsString());
    DataManager(convChunk->data, convChunk->size).writeToFile(OUT_STUFF + "1_BASE_convChunk" + convChunk->getDataAsString());

    ps3_Chunk->ensureDecompress(ps3_Save.myConsole);

    wiiUChunk->ensureDecompress(wiiUSave.myConsole);

    convChunk->ensureDecompress(lce::CONSOLE::NONE);

    DataManager(ps3_Chunk->data, ps3_Chunk->size).writeToFile(OUT_STUFF + "2_DECOMP_ps3_Chunk" + ps3_Chunk->getDataAsString());
    DataManager(wiiUChunk->data, wiiUChunk->size).writeToFile(OUT_STUFF + "2_DECOMP_wiiUChunk" + wiiUChunk->getDataAsString());
    DataManager(convChunk->data, convChunk->size).writeToFile(OUT_STUFF + "2_DECOMP_convChunk" + convChunk->getDataAsString());

    ps3_Chunk->ensureCompressed(ps3_Save.myConsole);
    wiiUChunk->ensureCompressed(wiiUSave.myConsole);
    convChunk->ensureCompressed(convSave.myConsole);

    DataManager(ps3_Chunk->data, ps3_Chunk->size).writeToFile(OUT_STUFF + "3_COMPRESSED_ps3_Chunk" + ps3_Chunk->getDataAsString());
    DataManager(wiiUChunk->data, wiiUChunk->size).writeToFile(OUT_STUFF + "3_COMPRESSED_wiiUChunk" + wiiUChunk->getDataAsString());
    DataManager(convChunk->data, convChunk->size).writeToFile(OUT_STUFF + "3_COMPRESSED_convChunk" + convChunk->getDataAsString());

    // ps3_Chunk->ensureDecompress(ps3_Save.myConsole);
    // DataManager(ps3_Chunk->data, ps3_Chunk->size).writeToFile(OUT_STUFF + "4_DECOMP_ps3_Chunk" + ps3_Chunk->getDataAsString());

    // ps3_Chunk->ensureCompressed(ps3_Save.myConsole);
    // DataManager(ps3_Chunk->data, ps3_Chunk->size).writeToFile(OUT_STUFF + "5_COMPRESSED_ps3_Chunk" + ps3_Chunk->getDataAsString());
    */
    /*
    lce::registry::BlockRegistry blockReg;
    blockReg.setup();
    fs::create_directory("render");
    lce::registry::TextureRegistry textures;
    textures.setup();

    for (size_t i = 0; i < fileListing.region_overworld.size(); i++) {
        c_auto& regionFile = fileListing.region_overworld[i];

        auto region = editor::RegionManager();
        region.read(regionFile);

        MU int chunkIndex = 0;

        MU bool foundChunk = false;
        for (auto& chunk : region.chunks) {
            chunkIndex++;
            if (chunk.size == 0) { continue; }

            // if (!foundChunk) {
                // foundChunk = true;

            chunk.ensureDecompress(fileListing.myConsole);
            chunk.readChunk(fileListing.myConsole);
            auto* chunkData = chunk.chunkData;
            if (!chunkData->validChunk) {
                printf("found invalid chunk...\n");
                continue;
            }
            for (int x = 0; x < 16; x++) {
                for (int z = 0; z < 16; z++) {
                    for (int y = 0; y < 4; y++) {
                        placeBlock(chunkData, x, y, z,
                                   lce::blocks::ids::GOLD_BLOCK_ID, 0, false);
                    }
                }
            }
            chunk.chunkData->defaultNBT();
            chunk.writeChunk(fileListing.myConsole);
            chunk.ensureCompressed(fileListing.myConsole);
                // continue;
            //}

            // chunk.deallocate();
            // chunk.ensureDecompress(fileListing.myConsole);
            // chunk.readChunk(fileListing.myConsole);
            // c_auto* chunkData = chunk.chunkData;
            // if (!chunkData->validChunk) { continue; }
            // chunk.chunkData->defaultNBT();
            // chunk.writeChunk(fileListing.myConsole);
            // chunk.ensureCompressed(fileListing.myConsole);
        }

        fileListing.replaceRegionOW(i, region, fileListing.myConsole);

        printf("done!\n");

    }
    // convSave.printDetails();
    printf("Finished!\nFile Out: %s\n", OUT_RPCS3);
     */

    return 0;
}


