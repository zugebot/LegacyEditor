#include <iostream>

#include "lce/processor.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/code/include.hpp"

#include "LegacyEditor/unit_tests.hpp"

#include "lce/blocks/block_ids.hpp"
#include "lce/include/picture.hpp"
#include "lce/registry/blockRegistry.hpp"
#include "lce/registry/textureRegistry.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_IN = wiiu + R"(230918230206)";
    const std::string TEST_OUT = wiiu + R"(230918230207)";
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing::AUTO_REMOVE_PLAYERS = false;

    editor::FileListing fileListing;
    if (fileListing.read(TEST_IN) != 0) {
        return printf_err("failed to load file\n");
    }
    c_auto consoleIn = fileListing.console;


    fileListing.printDetails();
    fileListing.printFileList();

    editor::RegionManager region;
    region.read(fileListing.region_overworld[2]);
    editor::ChunkManager *chunk = region.getChunk(10, 10);

    chunk->ensureDecompress(consoleIn);
    chunk->readChunk(consoleIn);

    // this code has no purpose being here other than for show and tell
    lce::registry::BlockRegistry blockReg;
    blockReg.setup();
    std::cout << blockReg.getBlockFromID(1)->getName() << std::endl;

    // set up texture object and output folder
    fs::create_directory("render");
    lce::registry::TextureRegistry textures;
    textures.setup();

    const int CHUNK_HEIGHT = 96;
    for (int zIter = 0; zIter < 16; zIter++) {
        Picture chunkRender(16 * 16, CHUNK_HEIGHT * 16, 4);

        for (int xIter = 0; xIter < 16; xIter++) {
            for (int yIter = 0; yIter < CHUNK_HEIGHT; yIter++) {
                u16 block_id = editor::chunk::getBlock(chunk->chunkData, xIter, yIter, zIter) >> 4;
                Picture const* block_texture = textures.getBlockFromID(block_id);
                if (block_texture != nullptr) {
                    const int xPix = xIter * 16;
                    const int yPix = (CHUNK_HEIGHT - yIter - 1) * 16;
                    chunkRender.placeSubImage(block_texture, xPix, yPix);
                }
            }
        }

        chunkRender.saveWithName("chunk_render_xy_" + std::to_string(zIter) + ".png", "render/");
    }


    chunk->chunkData->defaultNBT();
    chunk->writeChunk(consoleOut);
    chunk->ensureCompressed(consoleOut);
    fileListing.replaceRegionOW(2, region, consoleOut);


    // fileListing.fileInfo.basesavename = L"Fortnite";
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
                           + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s\n", TEST_OUT.c_str());

    return 0;
}