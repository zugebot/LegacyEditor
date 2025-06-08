#pragma once

#include "code/ConsoleParser/ConsoleParser.hpp"
#include "code/ConsoleParser/NewGenConsoleParser.hpp"
#include "include/ghc/fs_std.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    class Switch final : public NewGenConsoleParser {
    public:

        Switch() { m_console = lce::CONSOLE::SWITCH; }
        ~Switch() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

        ND std::optional<fs::path> getFileInfoPath(SaveProject& saveProject) const override;

        std::vector<fs::path> findExternalFolder(editor::SaveProject &saveProject) override;
        int readExternalFolders(SaveProject& saveProject) override;

        MU int writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) override;
    };
}
