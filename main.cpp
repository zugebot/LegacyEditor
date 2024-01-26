#include <filesystem>
#include <iostream>
#include <thread>

#include "LegacyEditor/LCE/FileInfo/FileInfo.hpp"
#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/threaded.hpp"
#include "LegacyEditor/utils/timer.hpp"
#include "LegacyEditor/utils/RLESWITCH/rleswitch.hpp"


std::string dir_path, out_path, wiiu, ps3_;
typedef std::pair<std::string, std::string> strPair_t;
std::map<std::string, strPair_t> TESTS;
void TEST_PAIR(stringRef_t key, stringRef_t path_in, stringRef_t out) {
    std::string pathIn = dir_path + R"(tests\)" + path_in;
    TESTS.insert(std::make_pair(key, std::make_pair(pathIn, out)));
}



int readRegionFileCache(DataManager& managerIn) {
    static constexpr uint16_t MAGIC = 0x789C;

    uint16_t header1 = managerIn.readInt16AtOffset(11520);
    uint16_t header2 = managerIn.readInt16AtOffset(7424);

    uint32_t SECTOR_BYTES = 0;
    uint32_t chunkCount = 0;

    if (header1 == MAGIC) {
        SECTOR_BYTES = 3840;
        chunkCount = 960;
    } else if (header2 == MAGIC) {
        SECTOR_BYTES = 3710;
        chunkCount = 927;
    } else {
        return STATUS::INVALID_SAVE;
    }

    auto *chunks = new editor::ChunkManager[chunkCount];

    for ()




    managerIn.seek(SECTOR_BYTES * locations[chunkIndex]);

    chunk.size = managerIn.readInt32();
    chunk.setRLE(chunk.size >> 31);
    chunk.setUnknown(chunk.size >> 30 & 1);
    chunk.size &= 0x00FFFFFF;
    chunk.allocate(chunk.size);

    switch (console) {
        case CONSOLE::PS3: {
            const u32 num1 = managerIn.readInt32();
            const u32 num2 = managerIn.readInt32();
            chunk.setDecSize(num1); // rle dec size
            break;
        }
        case CONSOLE::RPCS3: {
            const u32 num1 = managerIn.readInt32();
            const u32 num2 = managerIn.readInt32();
            chunk.setDecSize(num1); // final dec size
            break;
        }
        case CONSOLE::XBOX360:
        case CONSOLE::SWITCH:
        case CONSOLE::WIIU:
        case CONSOLE::VITA:
        default:
            chunk.setDecSize(managerIn.readInt32()); // final dec size
            break;
    }
    memcpy(chunk.start(), managerIn.ptr, chunk.size);
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
    TEST_PAIR("PS4_khaloody",R"(PS4\00000008\savedata0\GAMEDATA)", wiiu + R"(BLANK_SAVE)");



    const std::string TEST_IN = TESTS["PS4_khaloody"].first;   // file to read from
    const std::string TEST_OUT = TESTS["PS4_khaloody"].second; // file to write to
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

    fileListing.fileInfo.basesavename = L"TEST NAME";
    fileListing.fileInfo.seed = 0;

    fileListing.printFileList();
    fileListing.printDetails();

    editor::map::saveMapToPng(fileListing.maps[0], R"(C:\Users\jerrin\CLionProjects\LegacyEditor\)");

    if (fileListing.saveToFolder() != 0) {
        return printf_err("failed to save files to folder\n");
    }



    const std::string gamedata_files = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\tests\PS4\00000007\savedata0)";
    namespace fs = std::filesystem;
    fs::remove_all(gamedata_files + "\\dec\\");
    fs::create_directory(gamedata_files + "\\dec\\");


    int fileIndex = -1;
    for (const auto& file : fs::directory_iterator(gamedata_files)) {
        if (is_directory(file)) { continue; }
        fileIndex++;

        uint32_t firstZLibs;
        std::vector<uint8_t> chunkSizes;
        std::vector<uint8_t> chunkLocks;
        std::vector<uint32_t> chunkDatas;


        // initiate filename and filepath
        std::string filepath_in = file.path().string();
        std::string filename = file.path().filename().string();
        std::string filepath_out = gamedata_files;
        filepath_out.append("\\dec\\");
        filepath_out.append(filename);

        // open the file
        DataManager manager_in;
        manager_in.setLittleEndian();
        manager_in.readFromFile(filepath_in);
        uint32_t fileSize = manager_in.readInt32();
        Data dat_out(fileSize);
        DataManager manager_out(dat_out);
        RLESWITCH_DECOMPRESS(manager_in.ptr, manager_in.size - 4,
                             manager_out.ptr, manager_out.size);


        // START OF TESTING



        int toFind = 4;
        manager_out.seek(4);
        int index = 0;
        while (toFind > 0) {
            uint8_t val1 = manager_out.readInt8AtOffset(index);
            uint8_t val2 = manager_out.readInt8AtOffset(index + 1);
            if (val1 != 0 && val2 != 0) {
                chunkSizes.push_back(val1);
                chunkLocks.push_back(val2);
                toFind--;
            }
            index += 2;
            if (index > 0x1D00) {
                break;
            }
        }

        bool added = false;
        while (true) {
            uint16_t value = manager_out.readInt16AtOffset(index);
            if ((value != 0x9C78 && value != 0x789C)
                || manager_out.readInt8AtOffset(index - 1) != 0) {
                index++;
                continue;
            }
            if (!added) {
                firstZLibs = index - 4;
                added = true;
            }
            chunkDatas.push_back(index - 4);
            if (chunkDatas.size() == chunkLocks.size()) {
                break;
            }
            index++;
        }


        manager_out.seekStart();
        manager_out.writeToFile(filepath_out);


        // print out details
        std::cout
                << filename
                << ": "
                << std::setw(7) << fileSize
                << " | "
                << firstZLibs - 4
                << " | ";
        for (int x = 0; x < chunkSizes.size(); x++) {
            std::cout << "("
                      << std::setw(3) << (int)chunkSizes[x]
                      << ", "
                      << std::setw(3) << (int)chunkLocks[x]
                      << ", "
                      << std::setw(5) << (int)chunkDatas[x] - 4;
            if (x != chunkSizes.size() - 1) {
                std::cout << "), ";
            } else {
                std::cout << ")";
            }
        }
        std::cout << std::endl;


    }














    // edit regions (threaded)
    // add functions to "LegacyEditor/LCE/scripts.hpp"
    const auto timer = Timer();
    // run_parallel<4>(editor::convertElytraToAquaticChunks, std::ref(fileListing));
    for (int x = 0; x < 4; x++) {
        // convertElytraToAquaticChunks(0, fileListing);
    }

    // fileListing.convertRegions(consoleOut);
    printf("Total Time: %.3f\n", timer.getSeconds());

    // fileListing.oldestVersion = 11;
    // fileListing.currentVersion = 11;

    // convert to fileListing
    const int statusOut = fileListing.write(TEST_OUT, consoleOut);
    if (statusOut != 0) {
        return printf_err({"converting to " + consoleToStr(consoleOut) + " failed...\n"});
    }
    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());


    return statusOut;
}