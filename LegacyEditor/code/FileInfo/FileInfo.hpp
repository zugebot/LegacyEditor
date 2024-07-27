#pragma once

#include "lce/processor.hpp"
#include "lce/enums.hpp"

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/data.hpp"


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

        MU void loadIngameThumbnail(const std::string& inFilePath);
        void defaultSettings();


        void readFile(const fs::path& inFilePath, const lce::CONSOLE inConsole);
        ND Data writeFile(const fs::path& outFilePath,
                         const lce::CONSOLE outConsole) const;
    };

}
