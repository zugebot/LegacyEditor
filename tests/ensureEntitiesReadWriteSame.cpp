#include <iostream>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "code/Impl/EntityFile.hpp"
#include "code/LCEFile/LCEFile.hpp"
#include "common/windows/force_utf8.hpp"

int main(int argc, char* argv[]) {

    fs::path folder = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\build\dump)";
    fs::path temp = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\build\dump\temp)";
    std::string fileName = "entitiesCorrect.dat";

    editor::LCEFile entityFile(lce::CONSOLE::SWITCH, 0, folder, temp, fileName);

    auto entities = editor::EntityFile::readEntityList(entityFile);

    editor::EntityFile::writeEntityList(entityFile, entities);

}