#include <iostream>

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/include.hpp"
#include "unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();
    c_auto [fst, snd] = TESTS["XBOX_BIN"];
    constexpr auto consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    int status = fileListing.read(fst);
    if (status != 0) {
        return printf_err(status, "failed to load file '%s'\n", fst.c_str());
    }

    fileListing.printDetails();

    editor::WriteSettings settings(consoleOut, snd);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) {
        return printf_err(statusOut, "converting to %s failed...\n", consoleToCStr(consoleOut));
    }

    printf("Finished!\nFile Out: %s",snd.c_str());
    return 0;
}