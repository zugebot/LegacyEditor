#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "lce/processor.hpp"
#include "lce/blocks/block_ids.hpp"
#include "lce/registry/blockRegistry.hpp"
#include "lce/registry/textureRegistry.hpp"

#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_NAME = "SWITCH1";
    const std::string TEST_IN = TESTS[TEST_NAME].first;
    const std::string TEST_OUT = TESTS[TEST_NAME].second;
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    if (fileListing.read(TEST_IN, true) != 0) {
        return printf_err("failed to load file\n");
    }

    fileListing.printDetails();
    fileListing.pruneRegions();
    fileListing.printFileList();

    /*
    editor::RegionManager region;
    region.read(fileListing.region_overworld[2]);
    editor::ChunkManager *chunk = region.getNonEmptyChunk();

    chunk->ensureDecompress(consoleSwitch);
    chunk->readChunk(consoleSwitch);

    {
        lce::registry::BlockRegistry blockReg;
        blockReg.setup();

        fs::create_directory("render");
        lce::registry::TextureRegistry textures;
        textures.setup();

        const int CHUNK_HEIGHT = 96;

        for (int zIter = 0; zIter < 16; zIter++) {
            Picture chunkRender(16 * 16, CHUNK_HEIGHT * 16, 4);

            for (int xIter = 0; xIter < 16; xIter++) {
                placeBlock(chunk->chunkData, xIter, 64, zIter, lce::blocks::ids::GOLD_BLOCK_ID, 0, false);

                for (int yIter = 0; yIter < CHUNK_HEIGHT; yIter++) {
                    u16 block_id = editor::chunk::getBlock(chunk->chunkData, xIter, yIter, zIter) >> 4;
                    Picture const* block_texture = textures.getBlockFromID(block_id);
                    if (block_texture != nullptr) {
                        c_int xPix = xIter * 16;
                        c_int yPix = (CHUNK_HEIGHT - yIter - 1) * 16;
                        chunkRender.placeSubImage(block_texture, xPix, yPix);
                    }
                }
            }

            chunkRender.saveWithName("chunk_render(Z=" + std::to_string(zIter) + ")[" + std::to_string(chunk->chunkData->chunkX) + ", " + std::to_string(chunk->chunkData->chunkZ) + "].png", "render/");
        }
    }



    placeBlock(chunk->chunkData, 7, 64, 7, lce::blocks::ids::DRIED_KELP_BLOCK_ID, 0, false);


    chunk->chunkData->defaultNBT();
    chunk->writeChunk(consoleOut);
    chunk->ensureCompressed(consoleOut);

    fileListing.replaceRegionOW(2, region, consoleOut);
    */

    for (size_t index = 0; index < fileListing.region_overworld.size(); index++) {
        editor::convertElytraToAquaticChunks(index, fileListing.region_overworld, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_nether.size(); index++) {
        editor::convertElytraToAquaticChunks(index, fileListing.region_nether, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_end.size(); index++) {
        editor::convertElytraToAquaticChunks(index, fileListing.region_end, fileListing.myConsole, consoleOut);
    }

    fileListing.myOldestVersion = 11;
    fileListing.myCurrentVersion = 11;

    fileListing.fileInfo.basesavename = L"Fortnite";
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
                           + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());

    return 0;
}