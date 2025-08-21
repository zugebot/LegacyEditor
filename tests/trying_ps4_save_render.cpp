#include <iostream>

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"
#include "include/lce/registry/blockRegistry.hpp"
#include "include/lce/registry/textureRegistry.hpp"

#include "code/Impl/DisplayMetadata.hpp"
#include "code/include.hpp"
#include "unit_tests.hpp"
#include "utils/timer.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_NAME = "flatTestPS4"; // "PS4_khaloody";
    c_auto [fst, snd] = TESTS[TEST_NAME];
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    /*
    const std::string fileIn  = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\CODY_UUAS_2017010800565100288444\GAMEDATA)";
    const std::string fileOut = dir_path + R"(230918230206_out.ext)";
    editor::DisplayMetadata save_info;
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

    // fileListing.fileInfo.basesavename = L"Changed the name!";
    // fileListing.fileInfo.seed = 69420;
    fileListing.pruneRegions();
    fileListing.printDetails();


    /*
    lce::registry::BlockRegistry blockReg;
    blockReg.setup();

    fs::create_directory("render");
    lce::registry::TextureRegistry textures;
    textures.setup();


    // figure out the bounds of each of the regions
    for (int i = 0; i < fileListing.region_overworld.size(); i++) {
        c_auto& regionFile = fileListing.region_overworld[i];

        auto region = editor::RegionManager();
        region.read(regionFile);

        int chunkIndex = -1;
        auto chunkCoords = std::map<int, std::pair<int, int>>();

        auto* chunky = region.getNonEmptyChunk();

        for (auto& chunk : region.chunks) {
            chunkIndex++;
            if (chunk.size == 0) { continue; }

            chunk.ensureDecompress(fileListing.console);
            chunk.readChunk(fileListing.console);


            c_auto* chunkData = chunk.chunkData;
            if (!chunkData->validChunk) { continue; }

            chunkCoords[chunkIndex] = std::make_pair(chunkData->chunkX, chunkData->chunkZ);


            const int zIter = 0;
            const int CHUNK_HEIGHT = 128;

            Picture chunkRender(16 * 16, CHUNK_HEIGHT * 16, 4);

            for (int xIter = 0; xIter < 16; xIter++) {
                for (int yIter = 0; yIter < CHUNK_HEIGHT; yIter++) {
                    u16 block_id = editor::getBlock(chunk.chunkData, xIter, yIter, zIter) >> 4;
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



            chunk.chunkData->lastVersion -= 1;
            chunk.writeChunk(lce::CONSOLE::WIIU); // fileListing.console);
            chunk.ensureCompressed(lce::CONSOLE::WIIU); // fileListing.console);
        }

            chunk.chunkData->lastVersion = 0x000C;
            chunk.writeChunk(lce::CONSOLE::WIIU);       // fileListing.console);
            chunk.ensureCompressed(lce::CONSOLE::WIIU); // fileListing.console);

        }
    }
    */


    c_auto timer = Timer();
    // run_parallel<32>(editor::convertElytraToAquaticChunks, std::ref(fileListing));
    for (int i = 0; i < 32; i++) {
        ConvertPillagerToAquaticChunks(i, fileListing);
    }
    fileListing.convertRegions(consoleOut);


    printf("Total Time: %.3f\n", timer.getSeconds());

    // fileListing.oldestVersion = 11;
    // fileListing.currentVersion = 11;

    // convert to fileListing
    editor::WriteSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());


    return statusOut;
}

