#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "LegacyEditor/utils/dataManager.hpp"

#include <vector>


class FileListing {
private:
    CONSOLE console;
public:
    std::vector<File> allFiles;

    // files
    std::vector<File*> overworldFilePtrs;
    std::vector<File*> netherFilePtrs;
    std::vector<File*> endFilePtrs;
    File* levelFilePtr{};
    File* grfFilePtr{};
    std::vector<File*> mapFilePtrs;
    std::vector<File*> structureFilePtrs;
    File* villageFilePtr{};
    std::vector<File*> playerFilePtrs;

    // data
    i32 oldestVersion{};
    i32 currentVersion{};

    FileListing() = default;

    explicit FileListing(CONSOLE consoleIn, Data& dataIn) : console(consoleIn) {
        read(dataIn);
    }

    void read(Data& dataIn);

    void saveToFolder(const std::string& folder);

    Data write(CONSOLE consoleOut);


};