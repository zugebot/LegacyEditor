#include "ConsoleParser.hpp"
#include "LegacyEditor/LCE/Chunk/v12Chunk.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"
#include "fileListing.hpp"




MU ND int ConsoleParser::convertTo(const std::string& inFileStr, const std::string& outFileStr, CONSOLE consoleOut) {
    int status = readConsoleFile(inFileStr);
    if (status != 0) return status;

    FileListing fileListing(this); // read  file listing
    fileListing.saveToFolder(dir_path + "dump_" + consoleToStr(console));


    fileListing.removeFileTypes({FileType::PLAYER, FileType::DATA_MAPPING});









    for (auto* fileList : fileListing.dimFileLists) {
        for (File* file: *fileList) {
            RegionManager region(this->console);
            region.read(file);
            Data data = region.write(consoleOut);
            delete[] file->data.data;
            file->data = data;
        }
    }



    Data dataOut = fileListing.write(consoleOut); // write file listing
    fileListing.deallocate();

    switch (consoleOut) {
        case CONSOLE::WIIU:
            return ConsoleParser::saveWiiU(outFileStr, dataOut);
        case CONSOLE::VITA:
            return ConsoleParser::saveVita(outFileStr, dataOut);
        case CONSOLE::XBOX360:
        case CONSOLE::RPCS3:
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

    fL.removeFileTypes({FileType::REGION_NETHER,
                        FileType::REGION_OVERWORLD,
                        FileType::REGION_END});

    fL.addFiles(fLR.collectFiles(FileType::REGION_NETHER));
    fL.addFiles(fLR.collectFiles(FileType::REGION_OVERWORLD));
    fL.addFiles(fLR.collectFiles(FileType::REGION_END));

    for (auto* fileList : fL.dimFileLists) {
        for (File* file: *fileList) {
            RegionManager region(fLR.console);
            region.read(file);
            Data data = region.write(consoleOut);
            delete[] file->data.data;
            file->data = data;
        }
    }

    Data dataOut = fL.write(consoleOut); // write file listing

    fL.deallocate();
    fLR.deallocate();

    switch (consoleOut) {
        case CONSOLE::WIIU:
            return ConsoleParser::saveWiiU(outFileStr, dataOut);
        case CONSOLE::VITA:
            return ConsoleParser::saveVita(outFileStr, dataOut);
        case CONSOLE::XBOX360:
        case CONSOLE::RPCS3:
        case CONSOLE::PS3:
        default:
            return -1;
    }
}


