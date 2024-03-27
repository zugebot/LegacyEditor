#include <iostream>


#include "LegacyEditor/libs/ghc/fs_std.hpp"
#include "LegacyEditor/LCE/MC/blocks.hpp"
#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "../LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    const std::string TEST_IN  = wiiu + R"(230918230206)";
    const std::string TEST_OUT = wiiu + R"(230918230207)";
    constexpr auto consoleOut = CONSOLE::WIIU;

    editor::FileListing fileListing;
    if (fileListing.read(TEST_IN) != 0) {
        return printf_err("failed to load file\n");
    }
    const auto consoleIn = fileListing.console;

    fileListing.removeFileTypes({
        // editor::FileType::PLAYER,
        editor::LCEFileType::DATA_MAPPING});

    fileListing.printDetails();
    fileListing.printFileList();

    editor::RegionManager region;
    region.read(fileListing.region_overworld[2]);
    editor::ChunkManager *chunk = region.getChunk(0, 0);

    chunk->ensureDecompress(consoleIn);
    chunk->readChunk(consoleIn);

    placeBlock(chunk->chunkData, 7, 64, 7, DRIED_KELP_BLOCK_ID, 0, false);

    chunk->chunkData->defaultNBT();
    chunk->writeChunk(consoleOut);
    chunk->ensureCompressed(consoleOut);

    fileListing.region_overworld[2]->data.deallocate();
    fileListing.region_overworld[2]->data = region.write(consoleOut);

    // fileListing.fileInfo.basesavename = L"Fortnite";
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
            + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());

    return 0;
}