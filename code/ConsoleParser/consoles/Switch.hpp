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
        void supplyRequiredDefaults(MU SaveProject* saveProject) const override {}

        ND int inflateFromLayout(const fs::path& theFilePath, SaveProject* saveProject) override;
        ND int deflateToSave(SaveProject* saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

        int readExternalFiles(SaveProject* saveProject);
        MU static int writeExternalFolder(MU const fs::path& outDirPath);
    };
}
