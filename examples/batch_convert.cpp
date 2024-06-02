#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/LCE/include.hpp"
#include "LegacyEditor/utils/processor.hpp"


void waitForEnter() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Must supply at least one save file to convert.\n";
        std::cerr << "You can do this by dragging and dropping a file onto the executable.\n";
        std::cerr << "click ENTER to exit.\n";
        waitForEnter();
        return -1;
    }

    std::cout << "Supports reading WiiU, PS3, RPCS3, Xbox360, PSVITA\n"
                 "Supports writing to WiiU, PSVITA\n";

    std::cout << "Name the console to output:";
    std::string consoleIn;
    std::cin >> consoleIn;
    const lce::CONSOLE consoleOut = lce::strToConsole(consoleIn);
    if (consoleOut == lce::CONSOLE::NONE) {
        std::cerr << "Invalid console name, exiting\n";
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

    for (int i = 1; i < argc; ++i) {
        fs::path filePath(argv[i]);
        if (!fs::exists(filePath)) {
            std::cerr << "File does not exist: " << filePath << "\n";
            continue;
        }

        editor::FileListing fileListing;
        if (fileListing.read(filePath.string()) != 0) {
            std::cerr << "Failed to load file: " << filePath << "\n";
            continue;
        }

        fs::path outFile = outDir / filePath.filename();
        outFile += "_" + consoleIn;
        const int statusOut = fileListing.write(outFile.string(), consoleOut);
        if (statusOut != 0) {
            std::cerr << "Converting to " << consoleToStr(consoleOut)
                      << " failed for file: " << filePath << "\n";
            continue;
        }
        std::cout << "Finished!\nFile Out: " << outFile << "\n";
    }

    std::cout << "click ENTER to exit.\n";
    waitForEnter();
    return 0;
}