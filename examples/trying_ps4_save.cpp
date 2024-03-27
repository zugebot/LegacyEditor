#include <iostream>

#include "LegacyEditor/libs/ghc/fs_std.hpp"

#include "LegacyEditor/LCE/FileInfo/FileInfo.hpp"
#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/timer.hpp"

#include "../LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_NAME = "PS4_khaloody"; //"PS4_khaloody";
    const std::string TEST_IN = TESTS[TEST_NAME].first;   // file to read from
    const std::string TEST_OUT = TESTS[TEST_NAME].second; // file to write to
    constexpr auto consoleOut = CONSOLE::WIIU;

    /*
    const std::string fileIn  = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\CODY_UUAS_2017010800565100288444\GAMEDATA)";
    const std::string fileOut = dir_path + R"(230918230206_out.ext)";
    editor::FileInfo save_info;
    save_info.readFile(fileIn);
    const DataManager manager(save_info.thumbnail);
    int status = manager.writeToFile(dir_path + "thumbnail.png");
    const int result = save_info.writeFile(fileOut, CONSOLE::PS3);
    if (result) {
        return result;
    }
    */

    // read savedata
    editor::FileListing fileListing;

    if (fileListing.read(TEST_IN) != 0) {
        return printf_err("failed to load file\n");
    }


    const std::string gamedata_files = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\tests\PS4\00000007\savedata0)";
    if (const int status = fileListing.readExternalRegions(gamedata_files)) {
        return status;
    }

    fileListing.removeFileTypes({
        editor::LCEFileType::PLAYER,
        editor::LCEFileType::REGION_NETHER,
        editor::LCEFileType::REGION_END
    });



    // editor::map::saveMapToPng(fileListing.maps[0], R"(C:\Users\jerrin\CLionProjects\LegacyEditor\)");

    if (fileListing.saveToFolder() != 0) {
        return printf_err("failed to save files to folder\n");
    }

    // fileListing.pruneRegions();
    fileListing.fileInfo.basesavename = L"Khalooody PS4 World";
    fileListing.fileInfo.seed = 0;
    fileListing.pruneRegions();
    fileListing.printFileList();
    fileListing.printDetails();


    int chunksOut = 0;
    // figure out the bounds of each of the regions
    for (int i = 0; i < fileListing.region_overworld.size(); i++) {
        const auto& regionFile = fileListing.region_overworld[i];

        auto region = editor::RegionManager();
        region.read(regionFile);

        /*
        int minX = INT32_MAX;
        int minZ = INT32_MAX;
        int maxX = INT32_MIN;
        int maxZ = INT32_MIN;
        */
        int chunkIndex = -1;
        auto chunkCoords = std::map<int, std::pair<int, int>>();

        for (auto& chunk : region.chunks) {
            chunkIndex++;
            if (chunk.size == 0) { continue; }

            chunk.ensureDecompress(fileListing.console);

            /*
            if (chunksOut < 5) {
                DataManager chunkOut(chunk.data, chunk.size);
                chunkOut.writeToFile(dir_path + "chunk" + std::to_string(chunksOut++));
            }*/

            chunk.readChunk(fileListing.console);
            const auto* chunkData = chunk.chunkData;
            if (!chunkData->validChunk) { continue; }

            chunkCoords[chunkIndex] = std::make_pair(chunkData->chunkX, chunkData->chunkZ);

            /*
            minX = minX > (int)chunkData->chunkX ? (int)chunkData->chunkX : minX;
            minZ = minZ > (int)chunkData->chunkZ ? (int)chunkData->chunkZ : minZ;
            maxX = maxX < (int)chunkData->chunkX ? (int)chunkData->chunkX : maxX;
            maxZ = maxZ < (int)chunkData->chunkZ ? (int)chunkData->chunkZ : maxZ;
            */
            chunk.writeChunk(fileListing.console);
            chunk.ensureCompressed(fileListing.console);
        }

        printf("done!");
        /*
        printf("%s: min: (%d, %d), max(%d, %d)\n",
            region->constructFileName(fileListing.console, true).c_str(),
            minX, minZ, maxX, maxZ);
        */
    }





    // edit regions (threaded)
    // add functions to "LegacyEditor/LCE/scripts.hpp"
    const auto timer = Timer();

    // run_parallel<32>(editor::convertElytraToAquaticChunks, std::ref(fileListing));
    for (int i = 0; i < 32; i++) {
        ConvertPillagerToAquaticChunks(i, fileListing);
    }

    // fileListing.convertRegions(consoleOut);
    printf("Total Time: %.3f\n", timer.getSeconds());

    // fileListing.oldestVersion = 11;
    // fileListing.currentVersion = 11;

    // convert to fileListing
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to " + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());


    return statusOut;
}

