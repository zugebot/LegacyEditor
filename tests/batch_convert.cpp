#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/include.hpp"


void waitForEnter() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}



int getNumberFromUser(const std::string& param, int lower, int upper) {
    int number;

    while (true) {
        std::cout << param;
        std::cin >> number;

        // Check if the input is an integer and within the specified range
        if (std::cin.fail() || number < lower || number > upper) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "[X] Invalid input. Please try again." << std::endl;
        } else {
            break; // Valid input received, exit the loop
        }
    }
    return number;
}


int main(int argc, char *argv[]) {



    // DataManager bin;
    // bin.readFromFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\XBOX360\XBOX360_TU74.dat)");
    // bin.writeToFile(bin.data + 12, bin.size - 12, "D:/PycharmProjects/testLZX/XBOX360_TU74_new.dat");






    std::cout << "\n";
    std::cout << "[-] Find the project here! https://github.com/zugebot/LegacyEditor" << std::endl;
    std::cout << "\n";
    std::cout << "[-] Supports reading [ Xbox360, PS3, RPCS3, PSVITA, PS4, WiiU, Switch ]\n"
                 "[-] Supports writing [ -------  ---  RPCS3, PSVITA, ---  WiiU  ------ ]\n";


    // Make sure user provides files
    if (argc < 2) {
        std::cerr << "\nMust supply at least one save file to convert.\n";
        std::cerr << "You can do this by dragging and dropping the GAMEDATA/SAVEGAME/.dat/.bin file[s] onto the executable, or passing them as command line arguments.\n";
        std::cerr << "click ENTER to exit.\n";
        waitForEnter();
        return -1;
    }


    // Ensure the "out" directory exists
    fs::path dirMain(argv[0]);
    fs::path outDir = dirMain.parent_path() / "out";
    if (!fs::exists(outDir)) {
        fs::create_directory(outDir);
    }


    // Gets the console out
    std::cout << "\n[*] Name the console you want your saves converted to (don't include spaces)."
                 "\n[>] Console:";
    std::string consoleIn;
    std::cin >> consoleIn;
    const lce::CONSOLE consoleOut = lce::strToConsole(consoleIn);
    if (consoleOut == lce::CONSOLE::NONE) {
        std::cerr << "Invalid console name, exiting\n";
        std::cerr << "click ENTER to exit.\n";
        waitForEnter();
        return -1;
    }


    // gets ps3 product code, if ps3 / rpcs3 are chosen
    editor::WriteSettings writeSettings(consoleOut, outDir);
    if (consoleOut == lce::CONSOLE::RPCS3 ||
        consoleOut == lce::CONSOLE::PS3) {
        std::cout << "\n[*] Please select a PS3 region (type the index number):" << std::endl;
        std::cout << "[1] NPEB01899 | Europe (HDD)\n"
                  << "[2] NPUB31419 | USA    (HDD)\n"
                  << "[3] NPJB00549 | Japan  (HDD)\n"
                  << "[4] BLES01976 | Europe (Disc)(not tested)\n"
                  << "[5] BLUS31426 | USA    (Disc)(not tested)\n";
        std::string prompt = "[>] Region:";
        int selection = getNumberFromUser(prompt, 1, 5);
        auto pCode = editor::PS3ProductCodeArray[selection];
        writeSettings.myProductCodes.setPS3(pCode);
    }

    if (consoleOut == lce::CONSOLE::VITA) {
        std::cout << "\n[*] Please select a PSVita region (type the index number):" << std::endl;
        std::cout << "[1] PCSE00491 | Europe\n"
                  << "[2] PCSB00560 | USA\n"
                  << "[3] PCSG00302 | Japan \n";
        std::string prompt = "[>] Region:";
        int selection = getNumberFromUser(prompt, 1, 3);
        auto pCode = editor::PSVITAProductCodeArray[selection];
        writeSettings.myProductCodes.setVITA(pCode);
    }


    // iterate over all the files they gave
    for (int saveIndex = 1; saveIndex < argc; ++saveIndex) {
        fs::path filePath(argv[saveIndex]);
        if (!fs::exists(filePath)) {
            std::cerr << "File does not exist: " << filePath.make_preferred() << "\n";
            continue;
        }

        editor::FileListing fileListing;
        if (fileListing.read(filePath.string()) != 0) {
            std::cerr << "Failed to load file: " << filePath.make_preferred() << "\n";
            continue;
        }


        fileListing.printDetails();
        MU int stat = fileListing.dumpToFolder("");

        const int statusOut = fileListing.write(writeSettings);
        if (statusOut != 0) {
            std::cerr << "Converting to " << consoleToStr(consoleOut)
                      << " failed for file: " << filePath << "\n";
            continue;
        }


        std::cout << "[>]: " << filePath.make_preferred() << "\n";
        std::cout << "[<]: " << writeSettings.getOutFilePath().make_preferred() << "\n";
    }

    std::cout << "\n\nclick ENTER to exit.\n";
    waitForEnter();
    return 0;
}