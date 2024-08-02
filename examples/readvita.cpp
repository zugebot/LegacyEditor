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


    for (size_t index = 0; index < fileListing.region_overworld.size(); index++) {
        editor::updateChunksToAquatic(index, fileListing.region_overworld, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_nether.size(); index++) {
        editor::updateChunksToAquatic(index, fileListing.region_nether, fileListing.myConsole, consoleOut);
    }
    for (size_t index = 0; index < fileListing.region_end.size(); index++) {
        editor::updateChunksToAquatic(index, fileListing.region_end, fileListing.myConsole, consoleOut);
    }


    fileListing.fileInfo.basesavename = L"lolza admireU skibidi toilet";
    editor::ConvSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);

    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());
    return 0;
}