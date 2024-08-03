#pragma once

#include "lce/processor.hpp"
#include "lce/enums.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


namespace editor {

    class FileInfo {
    public:
        Data ingameThumbnail;
        i64 seed{};
        i64 loads{};
        i64 hostoptions{};
        i64 texturepack{};
        i64 extradata{};
        i64 exploredchunks{};
        std::wstring basesavename;
        bool isLoaded;

        void defaultSettings();
        MU void loadFileAsThumbnail(const std::string& inFilePath);

        int readFile(const fs::path& inFilePath, const lce::CONSOLE inConsole);
        int readCacheFile(const fs::path& inFilePath, const std::string& folderName);

        ND Data writeFile(const fs::path& outFilePath, const lce::CONSOLE outConsole) const;

        int readPNG(DataManager& theManager);

    private:
        int readHeader(DataManager& theManager, lce::CONSOLE theConsole);
    };

}
