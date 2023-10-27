#include <iostream>

#include "LegacyEditor/utils/managers/dataOutManager.hpp"
#include "LegacyEditor/utils/managers/dataInManager.hpp"

#include "LegacyEditor/LCE/ConsoleParser.hpp"
#include "LegacyEditor/LCE/fileListing.hpp"

std::string dir_path = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\)";
std::string test_path = R"(C:\Users\Jerrin\CLionProjects\LegacyConverter\build\tests\)";

int main() {
    {
        auto managerOut = DataOutManager(128);
        managerOut.setBigEndian();
        managerOut.writeWString("Hello World!", 64, false);
        managerOut.saveToFile(dir_path + "data_out.bin");

        auto managerIn = DataInManager(1);
        managerIn.setBigEndian();
        managerIn.readFromFile(dir_path + "data_out.bin");

        auto stringOut = managerIn.readWAsString(64, false);
        std::cout << stringOut << ", " << stringOut.size() << std::endl;
    }
    std::string fileInPath = dir_path + "1.13_Test_In.wii";

    ConsoleFileParser parser;
    int status = parser.loadConsoleFile(fileInPath.c_str());

    FileListing fileListing;
    DataInManager worldIn(parser);
    parser.using_memory = false;

    worldIn.setBigEndian();

    fileListing.read(worldIn);

    DataOutManager worldOut = fileListing.write();

    worldIn.seekStart();
    worldOut.seekStart();
    printf("\nin: %u | out: %u\n", worldIn.size, worldOut.size);

    std::string file_path_dec_in = dir_path + "test_dec_in.wii_d";
    std::string file_path_dec_out = dir_path + "test_dec_out.wii_d";
    worldIn.saveToFile(file_path_dec_in);
    worldOut.saveToFile(file_path_dec_out);


    u8 dI, dO;
    for (int i = 0; i < 4996457; i++) {
           dI = worldIn.data[i];
           dO = worldOut.data[i];
           if (dI != dO) {
               printf("index %7d doesn't match up [%u != %u]\n", i, dI, dO);

           }
    }
    int x; std::cin >> x;

    std::cout << "waiting..." << std::endl;

    std::string out_file = dir_path + "1.13_Test_Out.wii";
    parser.saveWiiU(out_file, worldOut);




    return status;
}
