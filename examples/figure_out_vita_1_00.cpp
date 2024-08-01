#include "lce/processor.hpp"

#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();

    auto [fst, snd] = TESTS["VITA_1_00"];

    editor::FileListing listing;
    int status = listing.read(fst);

    if (status != 0) return status;


}