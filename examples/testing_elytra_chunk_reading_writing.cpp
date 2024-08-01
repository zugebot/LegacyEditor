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

    const std::string FILE_IN = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\tests\elytra_tutorial)";
    const std::string FILE_OUT = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\ely_to_aqua)";
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    int status = fileListing.read(FILE_IN);
    if (status != 0)
        return printf_err(status, "failed to load file '%s'\n", FILE_IN.c_str());

    fileListing.fileInfo.basesavename = L"Elytra -> Aquatic";
    fileListing.printDetails();

    fileListing.removeFileTypes({lce::FILETYPE::REGION_NETHER, lce::FILETYPE::REGION_END});

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
        editor::updateChunksToAquatic(index, fileListing.region_overworld, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_nether.size(); index++) {
        editor::updateChunksToAquatic(index, fileListing.region_nether, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_end.size(); index++) {
        editor::updateChunksToAquatic(index, fileListing.region_end, fileListing.myConsole, consoleOut);
    }

    fileListing.myOldestVersion = 11;
    fileListing.myCurrentVersion = 11;

    const int statusOut = fileListing.write({consoleOut, FILE_OUT});
    if (statusOut != 0)
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));

    printf("Finished!\nFile Out: %s", FILE_OUT.c_str());

    return 0;
}