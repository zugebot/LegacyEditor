#pragma once

#include "include/lce/processor.hpp"
#include "common/data/ghc/fs_std.hpp"
#include "productcodes.hpp"


namespace editor {

    struct SaveLayout {
        fs::path baseFolder;

        fs::path worldDataFile;
        std::optional<fs::path> displayMetadata;
        std::optional<fs::path> displayIcon;

        std::vector<fs::path> externalRegionFiles;

        std::unordered_map<std::string, fs::path> customFiles;

        MU bool isValid() const { return !worldDataFile.empty(); }
    };
}
