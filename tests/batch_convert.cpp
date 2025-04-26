#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/include.hpp"
#include "common/timer.hpp"


void waitForEnter() {
    std::cout << "[>] Press ENTER to exit...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
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

    /*
    std::string path = R"(C:\Users\jerrin\CLionProjects\BetterNBT\level.dat)";
    DataManager manager;
    manager.readFromFile(path);

    c_auto* nbt = NBT::readTag(manager);

    if (nbt == nullptr) {
        return -1;
    }

    std::cout << nbt->toString() << "\n";

    return 0;
    */



    // DataManager bin;
    // bin.readFromFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\tests\XBOX360\XBOX360_TU74.dat)");
    // bin.writeToFile(bin.data + 12, bin.size - 12, "D:/PycharmProjects/testLZX/XBOX360_TU74_new.dat");


    bool takeInput = true;
    std::string autoConsole = "wiiu";


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
                 "\n[>] Console: ";
    std::string consoleIn;
    if (takeInput) {
        std::cin >> consoleIn;
    } else {
        consoleIn = autoConsole;
    }
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

    // gets psvita product code, if psvita is chosen
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



    // remove player data
    // if (writeSettings.getConsole() != consoleOut) {
    //     writeSettings.shouldRemovePlayers = true;
    //     std::cout << "[!] Source and target consoles differ. Player data will be removed.\n";
    // } else
    {
        std::cout << "\n[*] Do you want to remove all player data from the save files?";
        std::cout << "\n[>] Option (y/n):";
        char wipeChoice;
        if (takeInput) {
            std::cin >> wipeChoice;
        } else {
            wipeChoice = 'y';
        }
        if (wipeChoice == 'y' || wipeChoice == 'Y') {
            writeSettings.shouldRemovePlayers = true;
            std::cout << "[!] Player data will be removed.\n";
        } else {
            writeSettings.shouldRemovePlayers = false;
            std::cout << "[!] Player data will NOT be removed.\n";
        }
    }


    // remove map data
    // if (writeSettings.getConsole() != consoleOut) {
    //     writeSettings.shouldRemoveMaps = true;
    //     std::cout << "[!] Source and target consoles differ. Map data will be removed.\n";
    // } else
    {
        std::cout << "\n[*] Do you want to remove all map data from the save files?";
        std::cout << "\n[>] Option (y/n):";
        char wipeChoice;
        if (takeInput) {
            std::cin >> wipeChoice;
        } else {
            wipeChoice = 'y';
        }
        if (wipeChoice == 'y' || wipeChoice == 'Y') {
            writeSettings.shouldRemoveMaps = true;
            std::cout << "[!] Map data will be removed.\n";
        } else {
            writeSettings.shouldRemoveMaps = false;
            std::cout << "[!] Map data will NOT be removed.\n";
        }
    }


    // remove structure data
    //if (writeSettings.getConsole() != consoleOut) {
    //    writeSettings.shouldRemoveStructures = true;
    //    std::cout << "[!] Source and target consoles differ. Structure data will be removed.\n";
    //} else
    {
        std::cout << "\n[*] Do you want to remove all structure data from the save files?";
        std::cout << "\n[>] Option (y/n):";
        char wipeChoice;
        if (takeInput) {
            std::cin >> wipeChoice;
        } else {
            wipeChoice = 'y';
        }
        if (wipeChoice == 'y' || wipeChoice == 'Y') {
            writeSettings.shouldRemoveStructures = true;
            std::cout << "[!] Structure data will be removed.\n";
        } else {
            writeSettings.shouldRemoveStructures = false;
            std::cout << "[!] Structure data will NOT be removed.\n";
        }
    }


    // remove structure data
    //if (writeSettings.getConsole() != consoleOut) {
    //    writeSettings.shouldRemoveStructures = true;
    //    std::cout << "[!] Source and target consoles differ. Structure data will be removed.\n";
    //} else
    /*
    {
        std::cout << "\n[*] Do you want to remove all entity data from the save files?";
        std::cout << "\n[*] Only try this if there is an issue with loading your world.";
        std::cout << "\n[>] Option (y/n):";
        char wipeChoice;
        if (takeInput) {
            std::cin >> wipeChoice;
        } else {
            wipeChoice = 'y';
        }
        if (wipeChoice == 'y' || wipeChoice == 'Y') {
            writeSettings.shouldRemoveEntities = true;
            std::cout << "[!] Entity data will be removed.\n";
        } else {
            writeSettings.shouldRemoveEntities = false;
            std::cout << "[!] Entity data will NOT be removed.\n";
        }
    }
    */






    std::cout << "\n";


    // iterate over all the files they gave
    for (int saveIndex = 1; saveIndex < argc; ++saveIndex) {

        // ensure file exists
        fs::path filePath(argv[saveIndex]);
        if (!fs::exists(filePath)) {
            std::cerr << "File does not exist: " << filePath.make_preferred() << "\n";
            continue;
        }

        // read the file
        Timer readTimer;
        editor::FileListing fileListing;
        if (fileListing.read(filePath.string()) != 0) {
            std::cerr << "Failed to load file: " << filePath.make_preferred() << "\n";
            continue;
        }
        std::cout << "[%] Time to read: " << readTimer.getSeconds() << " sec\n";

        // preprocess write settings
        const int statusProcess = fileListing.preprocess(writeSettings);
        if (statusProcess != 0) {
            std::cerr << "Preprocessing " << consoleToStr(consoleOut)
                      << " failed for file: " << filePath << "\n";
            continue;
        }

        fileListing.removeFileTypes({lce::FILETYPE::REGION_END, lce::FILETYPE::REGION_NETHER});

        // print save details, dump to folder
        fileListing.printDetails();
        MU int stat = fileListing.dumpToFolder("");

        // write to new console
        Timer writeTimer;
        const int statusOut = fileListing.write(writeSettings);
        if (statusOut != 0) {
            std::cerr << "Converting to " << consoleToStr(consoleOut)
                      << " failed for file: " << filePath << "\n";
            continue;
        }
        std::cout << "[%] Time to write: " << readTimer.getSeconds() << " sec\n";

        // alert user of new location
        std::cout << "[>]: " << filePath.make_preferred() << "\n";
        std::cout << "[<]: " << writeSettings.getOutFilePath().make_preferred() << "\n";
    }

    std::cout << "\n";
    waitForEnter();
    return 0;
}