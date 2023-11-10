#include "ConsoleParser.hpp"
#include "fileListing.hpp"



MU ND int ConsoleParser::convertTo(const std::string& inFileStr, const std::string& outFileStr, CONSOLE consoleOut) {
    int status = readConsoleFile(inFileStr);
    if (status != 0) return status;

    FileListing fileListing(this); // read  file listing
    Data dataOut = fileListing.write(consoleOut); // write file listing

    switch (consoleOut) {
        case CONSOLE::WIIU:
            return ConsoleParser::saveWiiU(outFileStr, dataOut);
        case CONSOLE::VITA:
            return ConsoleParser::saveVita(outFileStr, dataOut);
        case CONSOLE::XBOX360:
        case CONSOLE::PS3:
        default:
            return -1;
    }
}






MU ND int ConsoleParser::convertAndReplaceRegions(const std::string& inFileStr,
                                                  const std::string& inFileRegionReplacementStr,
                                                  const std::string& outFileStr, CONSOLE consoleOut) {
    auto replace = ConsoleParser();

    int status1 = readConsoleFile(inFileStr);
    if (status1 != 0) return status1;
    FileListing fL(this);

    int status2 = replace.readConsoleFile(inFileRegionReplacementStr);
    if (status2 != 0) return status2;
    FileListing fLR(replace);

    fL.removeFileTypes(FileType::REGION_NETHER, FileType::REGION_OVERWORLD, FileType::REGION_END);
    fL.addFiles(fLR.collectFiles(FileType::REGION_NETHER));
    fL.addFiles(fLR.collectFiles(FileType::REGION_OVERWORLD));
    fL.addFiles(fLR.collectFiles(FileType::REGION_END));


    Data dataOut = fL.write(consoleOut); // write file listing

    switch (consoleOut) {
        case CONSOLE::WIIU:
            return ConsoleParser::saveWiiU(outFileStr, dataOut);
        case CONSOLE::VITA:
            return ConsoleParser::saveVita(outFileStr, dataOut);
        case CONSOLE::XBOX360:
        case CONSOLE::PS3:
        default:
            return -1;
    }
}


