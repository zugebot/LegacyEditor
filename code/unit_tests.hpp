#pragma once

#include "lce/processor.hpp"

#include "common/data/ghc/fs_std.hpp"


#ifdef UNIT_TESTS
extern std::string dir_path;
extern std::string wiiu;
#endif

fs::path dir_path, wiiu;
std::map<std::string, std::pair<std::string, std::string>> TESTS;
void TEST_PAIR(const std::string &key, const std::string &path_in, fs::path& out) {
    fs::path pathIn = dir_path / "savefiles" / path_in;
    std::pair<std::string, std::string> pair = std::make_pair(pathIn.string(), out.string());
    TESTS.insert(std::make_pair(key, pair));
}

static void PREPARE_UNIT_TESTS() {
    const char* userProfile = std::getenv("USERPROFILE");
    if (!userProfile) {
        std::cerr << "USERPROFILE environment variable not set!\n";
        return;
    }
    fs::path dirPath = fs::path(userProfile) / "CLionProjects" / "LegacyEditor";


    wiiu = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";

    // PS3
    TEST_PAIR("PS3_1"       , R"(PS3/NPUB31419--240715040616/GAMEDATA)"         , wiiu);
    // VITA
    TEST_PAIR("vita"        , R"(VITA/save/PCSB00560-231005063840/GAMEDATA.bin)", wiiu);
    TEST_PAIR("vita_mcs"    , R"(VITA/CavernLarge_MG01.mcs)"                    , wiiu);
    TEST_PAIR("VITA_1_00"    , R"(VITA/PCSE00491/PCSE00491-240725153321/GAMEDATA.bin)", wiiu);
    // RPCS3
    TEST_PAIR("RPCS3_1"     , R"(RPCS3/NPUB31419--240424132851)"       , wiiu);
    TEST_PAIR("RPCS3_1.00"  , R"(RPCS3/BLES01976--240802043920)"       , wiiu);
    // XBOX360
    fs::path temp1 = wiiu / R"(saves\XBOX360_TU69.bin)";
    fs::path temp2 = wiiu / R"(saves\XBOX360_TU74.bin)";
    TEST_PAIR("X360_TU69", R"(XBOX360/XBOX360_TU69.bin)", temp1);
    TEST_PAIR("X360_TU74", R"(XBOX360/XBOX360_TU74.dat)", temp2);
    // PS4
    TEST_PAIR("PS4_khaloody", R"(PS4/folder/00000008/savedata0/GAMEDATA)"       , wiiu);
    TEST_PAIR("flatTestPS4" , R"(PS4/superflatTest/00000002/savedata0/GAMEDATA)", wiiu);
    TEST_PAIR("corrupt_save", R"(PS4/CODY_UUAS_2017010800565100288444/GAMEDATA)", wiiu);
    // SWITCH
    TEST_PAIR("SWITCH_1"    , R"(SWITCH/190809160532.dat)"                      , wiiu);
    TEST_PAIR("SWITCH_2"    , R"(SWITCH/231011215216.dat)"                      , wiiu);
    // WIIU
    TEST_PAIR("WIIU_PIRATES", R"(WIIU/Pirates.wii)"                             , wiiu);
    // IDK
    TEST_PAIR("aquatic_tut",  R"(TUTORIAL/aquatic_tutorial)"                    , wiiu);
    TEST_PAIR("elytra_tut",   R"(TUTORIAL/elytra_tutorial)"                     , wiiu);
}

