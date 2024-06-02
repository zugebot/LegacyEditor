#include "../LegacyEditor/unit_tests.hpp"
#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"

int main() {
    PREPARE_UNIT_TESTS();



    const std::string TEST_NAME = "vita";
    const std::string TEST_IN = TESTS[TEST_NAME].first;   // file to read from
    const std::string TEST_OUT = TESTS[TEST_NAME].second; // file to write to
    constexpr auto consoleOut = lce::CONSOLE::VITA;

    editor::FileListing fileListing;
    if (fileListing.read(TEST_IN) != 0) {
        return printf_err("failed to load file\n");
    }

    fileListing.fileInfo.basesavename = L"Fortnite";

    fileListing.removeFileTypes({
        editor::LCEFileType::PLAYER,
        editor::LCEFileType::DATA_MAPPING});

    fileListing.printDetails();
    fileListing.printFileList();

    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
            + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());

    return 0;
}