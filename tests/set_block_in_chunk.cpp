#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"
#include "include/lce/blocks/blockID.hpp"

#include "code/include.hpp"
#include "unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_IN  = wiiu + R"(230918230206)";
    const std::string TEST_OUT = wiiu + R"(230918230207)";
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing::AUTO_REMOVE_PLAYERS = false;

    editor::FileListing fileListing;
    int status = fileListing.read(TEST_IN);
    if (status != 0) {
        return printf_err(status, "failed to load file '%s'\n", TEST_IN.c_str());
    }
    c_auto consoleIn = fileListing.myReadSettings.getConsole();
    fileListing.fileInfo.baseSaveName = L"Fortnite";


    fileListing.printDetails();

    editor::RegionManager region;
    region.read(fileListing.ptrs.region_overworld[2]);
    editor::ChunkManager *chunk = region.getChunk(0, 0);

    chunk->ensureDecompress(consoleIn);
    chunk->readChunk(consoleIn);

    chunk->chunkData->placeBlock(7, 64, 7, lce::blocks::DRIED_KELP_BLOCK_ID, 0, false);

    chunk->chunkData->defaultNBT();
    chunk->writeChunk(consoleOut);
    chunk->ensureCompressed(consoleOut);

    fileListing.replaceRegionOW(2, region, consoleOut);

    editor::WriteSettings settings(consoleOut, TEST_OUT);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());

    return 0;
}