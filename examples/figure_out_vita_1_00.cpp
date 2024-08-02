#include "lce/processor.hpp"

#include "LegacyEditor/code/include.hpp"
#include "LegacyEditor/unit_tests.hpp"


int main() {
    PREPARE_UNIT_TESTS();


    auto [fst, snd] = TESTS["VITA_1_00"];
    editor::FileListing theListing;
    int status = theListing.read(fst);
    if (status != 0) return printf_err(status, "failed to read listing\n");

    status = theListing.dumpToFolder(R"(C:\\Users\\jerrin\\Desktop\\OUT\\)");
    if (status != 0) return printf_err(status, "failed to dump listing\n");

    fs::path OUT = R"(C:\Users\jerrin\Desktop\OUT\)";
    editor::ConvSettings settings(lce::CONSOLE::RPCS3, editor::ePS3ProductCode::NPUB31419, OUT);
    status = theListing.write(settings);
    if (status != 0) return printf_err(status, "failed to write listing\n");


}