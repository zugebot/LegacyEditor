#pragma once

#include "code/ConsoleParser/ConsoleParser.hpp"
#include "code/ConsoleParser/NewGenConsoleParser.hpp"
#include "include/ghc/fs_std.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    class Windurango final : public NewGenConsoleParser {
    public:

        Windurango() { m_console = lce::CONSOLE::WINDURANGO; }
        ~Windurango() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        int inflateListing(SaveProject& saveProject);

        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

        std::vector<fs::path> findExternalFolder(editor::SaveProject &saveProject) override;
        int readExternalFolders(SaveProject& saveProject) override;

        MU int writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) override;
    };
}
