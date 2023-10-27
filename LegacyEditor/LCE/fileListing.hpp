#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "LegacyEditor/utils/managers/dataInManager.hpp"
#include "LegacyEditor/utils/managers/dataOutManager.hpp"

#include <vector>


class FileListing {
public:
    // TODO IMPORTANT: RESIZING OF VECTORS BREAKS DE_ALLOCATION
    std::vector<File> allFiles;

    /// maybe use this later for creating file dir
    std::vector<std::string> allFolders;

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

    void read(DataInManager& managerIn);
    DataOutManager write();


};