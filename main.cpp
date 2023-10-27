#include <iostream>

#include "LegacyEditor/utils/managers/dataOutManager.hpp"
#include "LegacyEditor/utils/managers/dataInManager.hpp"

#include "LegacyEditor/LCE/ConsoleParser.hpp"
#include "LegacyEditor/LCE/fileListing.hpp"

std::string dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";
std::string test_path = R"(C:\Users\Jerrin\CLionProjects\LegacyConverter\build\tests\)";

int main() {
    /*
    {
        auto managerOut = DataOutManager(128);
        managerOut.setBigEndian();
        managerOut.writeWString("Hello World!", 64, true);
        managerOut.saveToFile(dir_path + "data_out.bin");

        auto managerIn = DataInManager(1);
        managerIn.setBigEndian();
        managerIn.readFromFile(dir_path + "data_out.bin");

        auto stringOut = managerIn.readWAsString(64, true);
        std::cout << stringOut << ", " << stringOut.size() << std::endl;
    }
     */
    std::string fileInPath = dir_path + "230918230206_In.wii";

    ConsoleFileParser parser;
    int status = parser.loadConsoleFile(fileInPath.c_str());

    FileListing fileListing;
    DataInManager worldIn(parser);
    parser.using_memory = false;

    worldIn.setBigEndian();

    fileListing.read(worldIn);

    File* filePtr = fileListing.overworldFilePtrs[0];







    // DataOutManager worldOut = fileListing.write();
    // std::string out_file = dir_path + "230918230206_out.wii";
    // parser.saveWiiU(out_file, worldOut);
    return status;
}


// worldIn.seekStart();
// worldOut.seekStart();
// printf("\nin: %u | out: %u\n", worldIn.size, worldOut.size);
// std::string file_path_dec_in = dir_path + "test_dec_in.wii_d";
// std::string file_path_dec_out = dir_path + "test_dec_out.wii_d";
// worldIn.saveToFile(file_path_dec_in);
// worldOut.saveToFile(file_path_dec_out);
// std::cout << "waiting..." << std::endl;