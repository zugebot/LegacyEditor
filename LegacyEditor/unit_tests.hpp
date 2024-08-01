#pragma once

#include <map>
#include <string>


#ifdef UNIT_TESTS
extern std::string dir_path;
extern std::string out_path;
extern std::string wiiu;
#endif

std::string dir_path, out_path, wiiu;
std::map<std::string, std::pair<std::string, std::string>> TESTS;
void TEST_PAIR(const std::string &key, const std::string &path_in, const std::string &out) {
    std::string pathIn = dir_path + R"(tests/)" + path_in;
    std::pair<std::string, std::string> pair = std::make_pair(pathIn, out);
    TESTS.insert(std::make_pair(key, pair));
}

static void PREPARE_UNIT_TESTS() {
    dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";
    out_path = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
    wiiu = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";

    // PS3
    TEST_PAIR("PS3_1"       , R"(PS3/NPUB31419--240715040616/GAMEDATA)"         , wiiu + R"(PS3_SAVE)");
    // VITA
    TEST_PAIR("vita"        , R"(VITA/save/PCSB00560-231005063840/GAMEDATA.bin)", wiiu + R"(VITA_SAVE)");
    TEST_PAIR("vita_mcs"    , R"(VITA/CavernLarge_MG01.mcs)"                    , wiiu + R"(VITA_MCS_SAVE)");
    TEST_PAIR("VITA_1_00"    , R"(VITA/v1.0.0/PCSE00491/PCSE00491-240725153321/GAMEDATA.bin)", wiiu + R"(VITA_1_00)");
    // RPCS3
    TEST_PAIR("RPCS3_1"     , R"(RPCS3/NPUB31419--240424132851/GAMEDATA)"       , wiiu + R"(RPCS3_SAVE)");
    // XBOX360
    TEST_PAIR("X360_TU69"   , R"(XBOX360/XBOX360_TU69.bin)"                     , wiiu + R"(saves\XBOX360_TU69.bin)" );
    TEST_PAIR("X360_TU74"   , R"(XBOX360/XBOX360_TU74.dat)"                     , wiiu + R"(saves\XBOX360_TU74.dat)" );
    // PS4
    TEST_PAIR("PS4_khaloody", R"(PS4/folder/00000008/savedata0/GAMEDATA)"       , wiiu + R"(240510172144)");
    TEST_PAIR("flatTestPS4" , R"(PS4/superflatTest/00000002/savedata0/GAMEDATA)", wiiu + R"(240510172144)");
    TEST_PAIR("corrupt_save", R"(PS4/CODY_UUAS_2017010800565100288444/GAMEDATA)", wiiu + R"(231000000000)");
    // SWITCH
    TEST_PAIR("SWITCH_1"    , R"(SWITCH/190809160532.dat)"                      , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("SWITCH_2"    , R"(SWITCH/231011215216.dat)"                      , wiiu + R"(switch_to_wiiu)");
    // WIIU
    TEST_PAIR("WIIU_PIRATES", R"(WIIU/Pirates.wii)"                             , wiiu + R"(pirates)");
    // IDK
    TEST_PAIR("aquatic_tut",  R"(TUTORIAL/aquatic_tutorial)"                    , wiiu + R"(230918230206)");
    TEST_PAIR("elytra_tut",   R"(TUTORIAL/elytra_tutorial)"                     , wiiu + R"(230918230206)");
}

