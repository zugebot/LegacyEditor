#pragma once

#include "include/lce/processor.hpp"

#include "code/ConsoleParser/ConsoleParser.hpp"
#include "code/ConsoleParser/NewGenConsoleParser.hpp"
#include "common/error_status.hpp"
#include "include/ghc/fs_std.hpp"

namespace editor {
    class FileListing;
    class SaveProject;

    class PS4 final : public NewGenConsoleParser {
    public:

        PS4() { m_console = lce::CONSOLE::PS4; }
        ~PS4() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override;
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

        ND std::optional<fs::path> getFileInfoPath(SaveProject& saveProject) const override;

        std::vector<fs::path> findExternalFolder(SaveProject& saveProject) override;
        int readExternalFolders(SaveProject& saveProject) override;
        
        MU int writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) override;
    };
}


