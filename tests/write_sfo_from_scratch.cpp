
#include "include/lce/processor.hpp"
#include "common/data/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"


std::string EXE_CURRENT_PATH;

int main(int argc, char* argv[]) {
    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();

    // fs::path sfo1Path = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\RPCS3\NPUB31419--240424132851\PARAM.SFO)";
    fs::path sfo1Path = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\savefiles\PS4\jerrins_and_tiaras\PS4-CUSA00744-2CUSA00744-210322225338.0\savedata0\sce_sys\param.sfo)";

    SFOManager sfo1(sfo1Path.string());

    auto attrs1 = sfo1.getAttributes();
    for (auto& attr : attrs1) {
        std::cout << attr.toString() << std::endl;
    }

    std::cout << "\n\n";

    fs::path sfo2Path = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\savefiles\PS3\TU6 Generation\NPUB31419--200322134139\PARAM.SFO)";
    SFOManager sfo2(sfo2Path.string());
    auto attrs2 = sfo2.getAttributes();
    for (auto& attr : attrs2) {
        std::cout << attr.toString() << std::endl;
    }

    /*
    fs::path outPath = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\RPCS3\NPUB31419--240424132851\param.sfo_out)";


    SFOManager newSFO;
    newSFO.addParam(eSFO_FMT::INT, "*GAMEDATA", "0");
    newSFO.addParam(eSFO_FMT::INT, "*ICON0.PNG", "0");
    newSFO.addParam(eSFO_FMT::INT, "*METADATA", "1");
    newSFO.addParam(eSFO_FMT::INT, "*THUMB", "0");
    newSFO.addParam(eSFO_FMT::UTF8_SPECIAL, "*ACCOUNT_ID", "0000000000000000");
    newSFO.addParam(eSFO_FMT::INT, "ATTRIBUTE", "0");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "CATEGORY", "SD");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", " ");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS", "");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS2", "");
    newSFO.addParam(eSFO_FMT::INT, "PARENTAL_LEVEL", "0");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "RPCS3_BLIST", "ICON0.PNG/METADATA/THUMB/GAMEDATA");
    newSFO.addParam(eSFO_FMT::INT, "SAVEDATA_DIRECTORY", "NPUB31419--240424132851"); // {RELEASE}--{TIME}
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_LIST_PARAM", "733");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "SUB_TITLE", "fgh");
    newSFO.addParam(eSFO_FMT::UTF8_NORMAL, "TITLE", "Minecraft: PlayStation®3 Edition");
    newSFO.setMagic(eSFO_MAGIC::PS3_HDD);
    newSFO.saveToFile(outPath.string());
     */



}