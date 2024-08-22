#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();


    const std::string TEST_NAME = "vita";
    c_auto [fst, snd] = TESTS[TEST_NAME];
    constexpr auto consoleOut = lce::CONSOLE::VITA;

    editor::FileListing fileListing;
    int status = fileListing.read(fst);
    if (status != 0) {
        return printf_err(status, "failed to load file '%s'\n", fst.c_str());
    }

    fileListing.fileInfo.baseSaveName = L"Fortnite";

    fileListing.removeFileTypes({
        lce::FILETYPE::PLAYER,
        lce::FILETYPE::DATA_MAPPING});

    fileListing.printDetails();

    editor::WriteSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) {
        return printf_err(-1, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }
    printf("Finished!\nFile Out: %s", snd.c_str());

    return 0;
}