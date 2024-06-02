#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();
    const auto [fst, snd] = TESTS["XBOX_BIN"];
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    if (fileListing.read(fst) != 0) {
        return printf_err("failed to load file\n");
    }

    fileListing.printDetails();
    fileListing.printFileList();

    // fileListing.fileInfo.basesavename = L"Fortnite";
    const int statusOut = fileListing.write(snd, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to "
            + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s",snd.c_str());

    return 0;
}