#include <filesystem>
#include <iostream>
#include <thread>

#include "LegacyEditor/LCE/FileInfo/FileInfo.hpp"
#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/threaded.hpp"
#include "LegacyEditor/utils/timer.hpp"


std::string dir_path, out_path, wiiu, ps3_;
typedef std::pair<std::string, std::string> strPair_t;
std::map<std::string, strPair_t> TESTS;
void TEST_PAIR(stringRef_t key, stringRef_t path_in, stringRef_t out) {
    std::string pathIn = dir_path + R"(tests\)" + path_in;
    TESTS.insert(std::make_pair(key, std::make_pair(pathIn, out)));
}


int main() {

    // unit tests
    dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";
    out_path = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
    wiiu = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
    ps3_ = R"(D:\Emulator Folders\rpcs3-v0.0.18-12904-12efd291_win64\dev_hdd0\home\00000001\savedata\NPUB31419--231212220825\)";
    TEST_PAIR("superflat",   R"(superflat)"                                    , wiiu + R"(231105133853)");
    TEST_PAIR("aquatic_tut", R"(aquatic_tutorial)"                             , wiiu + R"(230918230206)");
    TEST_PAIR("vita",        R"(Vita Save\PCSB00560-231005063840\GAMEDATA.bin)", wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("elytra_tut",  R"(elytra_tutorial)"                              , wiiu + R"(230918230206)");
    TEST_PAIR("NS_save1"  ,  R"(NS\190809160532.dat)"                          , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("fortnite",    R"(fortnite_world)"                               , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("rpcs3_flat",  R"(RPCS3_GAMEDATA)"                               , ps3_ + R"(GAMEDATA)");
    TEST_PAIR("X360_TU69",   R"(XBOX360_TU69.bin)"                             , dir_path + R"(tests\XBOX360_TU69.bin)" );
    TEST_PAIR("X360_TU74",   R"(XBOX360_TU74.dat)"                             , dir_path + R"(tests\XBOX360_TU74.dat)" );
    TEST_PAIR("nether",      R"(nether)", wiiu + R"(231114151239)");
    TEST_PAIR("corrupt_save",R"(CODY_UUAS_2017010800565100288444\GAMEDATA)", wiiu + R"(231000000000)");
    const std::string TEST_IN = TESTS["aquatic_tut"].first;   // file to read from
    const std::string TEST_OUT = TESTS["aquatic_tut"].second; // file to write to
    constexpr auto consoleOut = CONSOLE::WIIU;


    /*
    const std::string fileIn  = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\CODY_UUAS_2017010800565100288444\GAMEDATA)";
    const std::string fileOut = dir_path + R"(230918230206_out.ext)";
    editor::FileInfo save_info;
    save_info.readFile(fileIn);
    const DataManager manager(save_info.thumbnail);
    int status = manager.writeToFile(dir_path + "thumbnail.png");
    const int result = save_info.writeFile(fileOut, CONSOLE::PS3);
    if (result) {
        return result;
    }
    */


    // read savedata
    editor::FileListing fileListing;

    if (fileListing.read(TEST_IN) != 0) {
        return printf_err("failed to load file\n");
    }

    fileListing.fileInfo.basesavename = L"FORTNITE BALLS";
    fileListing.fileInfo.seed = 0;

    fileListing.printFileList();
    fileListing.printDetails();

    editor::map::saveMapToPng(fileListing.maps[0], R"(C:\Users\jerrin\CLionProjects\LegacyEditor\)");

    if (fileListing.saveToFolder() != 0) {
        return printf_err("failed to save files to folder\n");
    }

    // edit regions (threaded)
    // add functions to "LegacyEditor/LCE/scripts.hpp"
    const auto timer = Timer();
    // run_parallel<4>(editor::convertElytraToAquaticChunks, std::ref(fileListing));
    for (int x = 0; x < 4; x++) {
        convertElytraToAquaticChunks(0, fileListing);
    }

    fileListing.convertRegions(consoleOut);
    printf("Total Time: %.3f\n", timer.getSeconds());

    // fileListing.oldestVersion = 11;
    fileListing.currentVersion = 11;

    // convert to fileListing
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to " + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());


    return statusOut;
}