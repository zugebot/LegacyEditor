#include <string>
#include "LegacyEditor/code/include.hpp"


const char* error1 = "failed to load file '%s'\n";
const char* error2 = "converting to %s failed...\n";

int main() {
    const std::string path = R"(D:\wiiu\mlc\usr\save\00050000\101dbe00\user\80000001\)";
    // const std::string FILE_IN = path + "240606161549";
    const std::string FILE_IN = R"(C:\Users\Jerrin\CLionProjects\LegacyEditor\tests\elytra_tutorial)";

    const std::string FILE_OUT = path + "240606161549_out";
    lce::CONSOLE consoleOut = lce::CONSOLE::WIIU;

    editor::FileListing fileListing;
    int status = fileListing.read(FILE_IN);
    if (status != 0) return printf_err(status, error1, FILE_IN.c_str());



    editor::RegionManager region;
    region.read(fileListing.region_overworld[0]);
    editor::ChunkManager *chunk = region.getNonEmptyChunk();
    chunk->ensureDecompress(fileListing.myConsole);
    chunk->readChunk(fileListing.myConsole);


    editor::WriteSettings settings(consoleOut, FILE_OUT);
    const int statusOut = fileListing.write(settings);
    if (statusOut != 0) return printf_err(statusOut, error2, consoleToCStr(consoleOut));

    printf("Finished!\nFile Out: %s", FILE_OUT.c_str());
    return 0;
}