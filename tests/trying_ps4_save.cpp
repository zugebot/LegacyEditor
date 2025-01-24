#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/FileInfo/FileInfo.hpp"
#include "code/include.hpp"
#include "utils/timer.hpp"
#include "/LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_NAME = "PS4_khaloody"; //"PS4_khaloody";
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

    if (fileListing.dumpToFolder("") != 0) {
        return printf_err(-1, "failed to save files to folder\n");
    }

    // fileListing.pruneRegions();
    fileListing.fileInfo.baseSaveName = L"Khalooody PS4 World";
    fileListing.fileInfo.seed = 0;
    fileListing.pruneRegions();
    fileListing.printDetails();


    // figure out the bounds of each of the regions
    for (size_t i = 0; i < fileListing.ptrs.region_overworld.size(); i++) {
        c_auto& regionFile = fileListing.ptrs.region_overworld[i];

        auto region = editor::RegionManager();
        region.read(regionFile);

        int chunkIndex = -1;
        auto chunkCoords = std::map<int, std::pair<int, int>>();

        for (auto& chunk : region.chunks) {
            chunkIndex++;
            if (chunk.size == 0) { continue; }

            chunk.ensureDecompress(fileListing.myReadSettings.getConsole());

            /*
            if (chunksOut < 5) {
                DataManager chunkOut(chunk.data, chunk.size);
                chunkOut.writeToFile(dir_path + "chunk" + std::to_string(chunksOut++));
            }*/

            chunk.readChunk(fileListing.myReadSettings.getConsole());
            c_auto* chunkData = chunk.chunkData;
            if (!chunkData->validChunk) { continue; }

            chunkCoords[chunkIndex] = std::make_pair(chunkData->chunkX, chunkData->chunkZ);


            chunk.writeChunk(fileListing.myReadSettings.getConsole());
            chunk.ensureCompressed(fileListing.myReadSettings.getConsole());
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
    editor::WriteSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());


    return statusOut;
}

