
#include "common/data/DataReader.hpp"
#include "common/data/DataWriter.hpp"
#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"
#include "include/sfo/sfo.hpp"
#include "sfo/sfoHelper.hpp"

#include <windows.h>

void enableAnsiColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

std::string EXE_CURRENT_PATH;

int main(int argc, char* argv[]) {
    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();
    enableAnsiColors();



    fs::path sfoOriginalPath1 = "C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\sfos\\PS4_SFO_User1.sfo";
    SFOManager sfoOriginal1(sfoOriginalPath1.string());


    { // print args of sfoOriginal
        std::cout << "File Path:" << sfoOriginalPath1 << "\n";
        auto attrs1 = sfoOriginal1.getAttributes();
        for (auto& attr: attrs1) {
            // if (attr.myKey == "PARAMS") continue;
            std::cout << attr.toString() << std::endl;
        }
    }

    std::cout << "\n\n";

    fs::path sfoOriginalPath2 = "C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\out\\CUSA00744-250928014616.0\\sce_sys\\param.sfo";
    // fs::path sfoOriginalPath3 = "C:\\Users\\jerrin\\Desktop\\hysick\\9_6_2025\\PS4_CUSA00265CUSA00265-250906185921.1\\savedata0\\sce_sys\\param.sfo";
    SFOManager sfoOriginal2(sfoOriginalPath2.string());


    { // print args of sfoOriginal
        std::cout << "File Path:" << sfoOriginalPath2 << "\n";
        auto attrs1 = sfoOriginal2.getAttributes();
        for (auto& attr: attrs1) {
            // if (attr.myKey == "PARAMS") continue;
            std::cout << attr.toString() << std::endl;
        }
    }

    std::cout << "\n\n";

    printAttributeHexSideBySideColored(sfoOriginal1, sfoOriginal2);

    int x = 0;
    std::cin >> x;


    return 0;




    fs::path sfoOriginalPath = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\savefiles\PS4\jerrins_and_tiaras\PS4-CUSA00744-2CUSA00744-210322225338.0\savedata0\sce_sys\param.sfo)";
    SFOManager sfoOriginal(sfoOriginalPath.string());


    { // print args of sfoOriginal
        auto attrs1 = sfoOriginal.getAttributes();
        for (auto& attr: attrs1) {
            std::cout << attr.toString() << std::endl;
        }
    }





    std::string pc = "CUSA00744";
    std::string folderN = "CUSA00744-210322225338.0";
    std::string worldName = "Jerrin's and Tiara's?";

    SFOManager sfoNew;


    { // attempt to create a duplicate PARAM.SFO
        auto attrParams = sfoOriginal.getAttribute("PARAMS");
        if (!attrParams.has_value()) throw std::runtime_error("input param.sfo does not have attribute \"PARAMS\"");

        auto array = std::get<std::vector<uint8_t>>(attrParams.value().myValue);
        auto reader = DataReader(array.data(), array.size(), Endian::Little);

        std::vector<uint8_t> params(1024);
        DataWriter writer(params.data(), params.size(), Endian::Little);

        auto now = std::chrono::system_clock::now();
        u32 lastModifiedTime = std::chrono::duration_cast<std::chrono::seconds>(
                                       now.time_since_epoch()).count();

        u32 flagUnknown1 = 0x0A; // observed range 0x2–0xA

        writer.write<u32>(0); reader.skip(4);
        writer.writeBytes(reader.ptr(), 4); reader.skip(4);
        writer.writeBytes(reader.ptr(), 32); reader.skip(32);

        writer.write<u32>(1); reader.skip(4);
        writer.writeBytes((c_u8*)pc.data(), pc.size()); writer.skip(16 - pc.size()); reader.skip(16);
        writer.writeBytes((c_u8*)pc.data(), pc.size()); writer.skip(16 - pc.size()); reader.skip(16);

        writer.write<u32>(reader.read<u32>() + 1);  // modificationCount
        writer.skip(12); reader.skip(12);

        writer.write<u32>(flagUnknown1); reader.skip(4);
        writer.write<u32>(reader.read<u32>()); // creationTime

        writer.write<u32>(0); reader.skip(4);
        writer.write<u32>(lastModifiedTime); reader.skip(4);

        std::string params_str((const char*)(params.data()), params.size());
        sfoNew.addParam(eSFO_FMT::UTF8_SPECIAL, "PARAMS", params_str);

        std::string savedata_blocks("\x00\x02\x00\x00\x00\x00\x00\x00", 8);
        sfoNew.addParam(eSFO_FMT::UTF8_SPECIAL, "SAVEDATA_BLOCKS",     savedata_blocks);

        sfoNew.addParam(eSFO_FMT::UTF8_NORMAL,  "SAVEDATA_DIRECTORY",  folderN);
        sfoNew.addParam(eSFO_FMT::INT,          "SAVEDATA_LIST_PARAM", "0");
        sfoNew.addParam(eSFO_FMT::UTF8_NORMAL,  "SUBTITLE",            worldName);
        sfoNew.addParam(eSFO_FMT::UTF8_NORMAL,  "TITLE_ID", pc);

        { // print args of sfoNew
            auto attrs1 = sfoNew.getAttributes();
            for (auto& attr: attrs1) {
                std::cout << attr.toString() << std::endl;
            }
        }

    }


    printAttributeHexSideBySideColored(sfoOriginal, sfoNew);

    std::cout << std::endl;
    // int x;
    // std::cin >> x;



    // std::cout << "\n\n";

    // fs::path sfo2Path = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\savefiles\PS3\TU6 Generation\NPUB31419--200322134139\PARAM.SFO)";
    // SFOManager sfo2(sfo2Path.string());
    // auto attrs2 = sfo2.getAttributes();
    // for (auto& attr : attrs2) {
    //     std::cout << attr.toString() << std::endl;
    // }

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