#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"
#include <string>


int main() {
    PREPARE_UNIT_TESTS();
    auto [fst, snd] = TESTS["vita"];
    lce::CONSOLE consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    int status = fileListing.read(fst);
    if (status != 0) {
        return printf_err(status, "failed to load file '%s'\n", fst.c_str());
    }


    for (size_t index = 0; index < fileListing.ptrs.region_overworld.size(); index++) {
        editor::convertChunksToAquatic(index, fileListing.ptrs.region_overworld, fileListing.myReadSettings.getConsole(), consoleOut);
    }
    for (size_t index = 0; index < fileListing.ptrs.region_nether.size(); index++) {
        editor::convertChunksToAquatic(index, fileListing.ptrs.region_nether, fileListing.myReadSettings.getConsole(), consoleOut);
    }
    for (size_t index = 0; index < fileListing.ptrs.region_end.size(); index++) {
        editor::convertChunksToAquatic(index, fileListing.ptrs.region_end, fileListing.myReadSettings.getConsole(), consoleOut);
    }


    fileListing.fileInfo.baseSaveName = L"lolza admireU skibidi toilet";
    editor::WriteSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);

    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());
    return 0;
}