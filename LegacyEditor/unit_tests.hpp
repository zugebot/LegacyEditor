#pragma once

#include "lce/processor.hpp"

#include "LCE/include.hpp"


#ifdef UNIT_TESTS
extern std::string dir_path;
extern std::string out_path;
extern std::string wiiu;
extern std::string ps3_;
#endif

std::string dir_path, out_path, wiiu, ps3_;
std::map<std::string, std::pair<std::string, std::string>> TESTS;
void TEST_PAIR(const std::string &key, const std::string &path_in, const std::string &out) {
    std::string pathIn = dir_path + R"(tests\)" + path_in;
    std::pair<std::string, std::string> pair = std::make_pair(pathIn, out);
    TESTS.insert(std::make_pair(key, pair));
}

static void PREPARE_UNIT_TESTS() {
    dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";
    out_path = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
    const std::string out_build = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\out\)";
    wiiu = R"(D:\wiiu\mlc\usr\save\00050000\101d9d00\user\80000001\)";
    ps3_ = R"(D:\Emulator Folders\rpcs3-v0.0.18-12904-12efd291_win64\dev_hdd0\home\00000001\savedata\NPUB31419--231212220825\)";
    TEST_PAIR("superflat",   R"(superflat)"                                    , wiiu + R"(231105133853)");
    TEST_PAIR("aquatic_tut", R"(aquatic_tutorial)"                             , wiiu + R"(230918230206)");
    TEST_PAIR("vita",        R"(Vita Save\PCSB00560-231005063840\GAMEDATA.bin)", dir_path + R"(BLANK_VITA_SAVE)");
    TEST_PAIR("elytra_tut",  R"(elytra_tutorial)"                              , wiiu + R"(230918230206)");
    TEST_PAIR("NS_save1"  ,  R"(NS\190809160532.dat)"                          , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("fortnite",    R"(fortnite_world)"                               , wiiu + R"(BLANK_SAVE)");
    TEST_PAIR("rpcs3_flat",  R"(RPCS3_GAMEDATA)"                               , ps3_ + R"(GAMEDATA)");
    TEST_PAIR("X360_TU69",   R"(XBOX360_TU69.bin)"                             , dir_path + R"(saves\XBOX360_TU69.bin)" );
    TEST_PAIR("X360_TU74",   R"(XBOX360_TU74.dat)"                             , dir_path + R"(saves\XBOX360_TU74.dat)" );
    TEST_PAIR("nether",      R"(nether)"                                       , wiiu + R"(231114151239)");
    TEST_PAIR("corrupt_save",R"(CODY_UUAS_2017010800565100288444\GAMEDATA)"    , wiiu + R"(231000000000)");
    TEST_PAIR("PS4_khaloody",R"(PS4\00000008\savedata0\GAMEDATA)"              , wiiu + R"(240510172144)");
    TEST_PAIR("flatTestPS4" ,R"(superflatTest\00000002\savedata0\GAMEDATA)"    , wiiu + R"(240510172144)");


    TEST_PAIR("PS4_to_wiiu" ,R"(BLANK_SAVE)"                                   , dir_path + "PS4_to_wiiu_to_wiiu");
    TEST_PAIR("BIG_WIIU"    ,R"(240219135035)"                                 , "");
    TEST_PAIR("XBOX_BIN"    ,R"(XBOX360_TU69.bin)", out_build + R"(xbox_to_wiiu)");
    TEST_PAIR("XBOX_DAT"    ,R"(XBOX360_TU74.dat)", out_build + R"(xbox_to_wiiu)");
}

