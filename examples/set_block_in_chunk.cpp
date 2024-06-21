#include <iostream>

#include "lce/processor.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/code/include.hpp"

#include "LegacyEditor/unit_tests.hpp"

#include "lce/blocks/block_ids.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_IN  = wiiu + R"(230918230206)";
    const std::string TEST_OUT = wiiu + R"(230918230207)";
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing::AUTO_REMOVE_PLAYERS = false;

    editor::FileListing fileListing;
    if (fileListing.read(TEST_IN, true) != 0) {
        return printf_err("failed to load file\n");
    }
    c_auto consoleIn = fileListing.myConsole;


    fileListing.printDetails();
    fileListing.printFileList();

    editor::RegionManager region;
    region.read(fileListing.region_overworld[2]);
    editor::ChunkManager *chunk = region.getChunk(0, 0);

    chunk->ensureDecompress(consoleIn);
    chunk->readChunk(consoleIn);

    placeBlock(chunk->chunkData, 7, 64, 7, lce::blocks::ids::DRIED_KELP_BLOCK_ID, 0, false);

    chunk->chunkData->defaultNBT();
    chunk->writeChunk(consoleOut);
    chunk->ensureCompressed(consoleOut);

    fileListing.replaceRegionOW(2, region, consoleOut);

    fileListing.fileInfo.basesavename = L"Fortnite";

    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
            + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());

    return 0;
}