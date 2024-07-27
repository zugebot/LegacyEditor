#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "lce/processor.hpp"
#include "lce/registry/blockRegistry.hpp"
#include "lce/registry/textureRegistry.hpp"

#include "LegacyEditor/code/FileInfo/FileInfo.hpp"
#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/utils/timer.hpp"
#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_NAME = "flatTestPS4";
    c_auto [fst, snd] = TESTS[TEST_NAME];
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    /*
    const std::string fileIn  = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\CODY_UUAS_2017010800565100288444\GAMEDATA)";
    const std::string fileOut = dir_path + R"(230918230206_out.ext)";
    editor::FileInfo save_info;
    save_info.readFile(fileIn);
    const DataManager manager(save_info.thumbnail);
    int status = manager.writeToFile(dir_path + "thumbnail.png");
    const int result = save_info.writeFile(fileOut, lce::CONSOLE::PS3);
    if (result) {
        return result;
    }
    */

    // read savedata
    editor::FileListing fileListing;
    int status = fileListing.read(fst);
    if (status != 0) {
        return printf_err(status, "failed to load file '%s'\n", fst.c_str());
    }

    fileListing.removeFileTypes({
            lce::FILETYPE::PLAYER,
            lce::FILETYPE::REGION_NETHER,
            lce::FILETYPE::REGION_END
    });

    // editor::map::saveMapToPng(fileListing.maps[0], R"(C:\Users\jerrin\CLionProjects\LegacyEditor\)");

    if (fileListing.dumpToFolder("C:/Users/jerrin/CLionProjects/LegacyEditor/") != 0) {
        return printf_err(-1, "failed to save files to folder\n");
    }

    // fileListing.pruneRegions();
    fileListing.fileInfo.basesavename = L"superflatTestGG";
    fileListing.fileInfo.seed = 0;
    fileListing.pruneRegions();
    fileListing.printDetails();


    lce::registry::BlockRegistry blockReg;
    blockReg.setup();

    fs::create_directory("render");
    lce::registry::TextureRegistry textures;
    textures.setup();


    // figure out the bounds of each of the regions
    for (size_t i = 0; i < fileListing.region_overworld.size(); i++) {
        c_auto& regionFile = fileListing.region_overworld[i];

        auto region = editor::RegionManager();
        region.read(regionFile);

        int chunkIndex = -1;
        auto chunkCoords = std::map<int, std::pair<int, int>>();


        for (auto& chunk : region.chunks) {
            chunkIndex++;
            if (chunk.size == 0) { continue; }

            chunk.ensureDecompress(fileListing.myConsole);
            /*
            if (chunksOut < 5) {
                DataManager chunkOut(chunk.data, chunk.size);
                chunkOut.writeToFile(dir_path + "chunk" + std::to_string(chunksOut++));
            }*/
            chunk.readChunk(fileListing.myConsole);
            c_auto* chunkData = chunk.chunkData;
            if (!chunkData->validChunk) { continue; }

            chunkCoords[chunkIndex] = std::make_pair(chunkData->chunkX, chunkData->chunkZ);


            const int zIter = 0;
            const int CHUNK_HEIGHT = 128;

            Picture chunkRender(16 * 16, CHUNK_HEIGHT * 16, 4);

            for (int xIter = 0; xIter < 16; xIter++) {
                for (int yIter = 0; yIter < CHUNK_HEIGHT; yIter++) {
                    u16 block_id = editor::chunk::getBlock(chunk.chunkData, xIter, yIter, zIter) >> 4;
                    Picture const* block_texture = textures.getBlockFromID(block_id);
                    if (block_texture != nullptr) {
                        const int xPix = xIter * 16;
                        const int yPix = (CHUNK_HEIGHT - yIter - 1) * 16;
                        chunkRender.placeSubImage(block_texture, xPix, yPix);
                    }
                }
            }

            chunkRender.saveWithName("chunk_render[" + std::to_string(chunkData->chunkX) + ", "
                                             + std::to_string(chunkData->chunkZ) + "].png", "render/");





            chunk.writeChunk(fileListing.myConsole);
            chunk.ensureCompressed(fileListing.myConsole);
        }

        printf("done!");

    }





    // edit regions (threaded)
    // add functions to "LegacyEditor/code/scripts.hpp"
    c_auto timer = Timer();

    // run_parallel<32>(editor::convertElytraToAquaticChunks, std::ref(fileListing));
    for (int i = 0; i < 32; i++) {
        ConvertPillagerToAquaticChunks(i, fileListing);
    }

    // fileListing.convertRegions(consoleOut);
    printf("Total Time: %.3f\n", timer.getSeconds());

    // fileListing.oldestVersion = 11;
    // fileListing.currentVersion = 11;

    // convert to fileListing
    const int statusOut = fileListing.write(snd, consoleOut);
    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());


    return statusOut;
}

