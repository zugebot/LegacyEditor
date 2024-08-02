#include "lce/processor.hpp"

#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"
#include "lce/registry/blockRegistry.hpp"
#include "lce/registry/textureRegistry.hpp"


int main() {
    PREPARE_UNIT_TESTS();
    const char* READ_ERROR = "failed to load file '%s'\n";
    const char* WRITE_ERROR = "converting to %s failed...\n";


    c_auto [fst1, snd1] = TESTS["WIIU_PIRATES"];
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    if (int status = fileListing.read(fst1); status != 0) {
        return printf_err(status, READ_ERROR, fst1.c_str());
    }

    fileListing.removeFileTypes({
            lce::FILETYPE::GRF,
            lce::FILETYPE::MAP,
            lce::FILETYPE::VILLAGE,
            // lce::FILETYPE::REGION_OVERWORLD,
            lce::FILETYPE::REGION_NETHER,
            lce::FILETYPE::REGION_END
    });

    fileListing.printDetails();





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
     */


    editor::ConvSettings settings(consoleOut, snd1);
    if (int statusOut = fileListing.write(settings); statusOut != 0) {
        return printf_err(statusOut, WRITE_ERROR, consoleToCStr(consoleOut));
    }

    printf("Finished!\nFile Out: %s\n",snd1.c_str());

    /*
    printf("Attempting to re-read out file...\n");
    editor::FileListing fileListing1;
    std::string out = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\real)";
    if (int status = fileListing1.read(out); status != 0) {
        return printf_err(status, READ_ERROR, out.c_str());
    }
    // fileListing.printDetails();
    */

    return 0;
}