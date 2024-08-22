#pragma once

#include "lce/processor.hpp"
#include "lce/enums.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


namespace editor {

    class FileInfo {
    public:
        Data thumbnail;
        i64 seed{};
        i64 loads{};
        i64 hostOptions{};
        i64 texturePack{};
        i64 extraData{};
        i64 exploredChunks{};
        std::wstring baseSaveName;
        bool isLoaded;

        void defaultSettings();
        MU void loadFileAsThumbnail(const std::string& inFilePath);

        int readFile(const fs::path& inFilePath, lce::CONSOLE inConsole);
        int readCacheFile(const fs::path& inFilePath, const std::string& folderName);

        ND Data writeFile(const fs::path& outFilePath, lce::CONSOLE outConsole) const;

        int readPNG(DataManager& theManager);

    private:
        int readHeader(DataManager& theManager, lce::CONSOLE theConsole);
    };

}
