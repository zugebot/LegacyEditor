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
        void supplyRequiredDefaults(MU SaveProject* saveProject) const override {}

        ND int inflateFromLayout(const fs::path& theFilePath, SaveProject* saveProject) override;
        ND int deflateToSave(MU SaveProject* saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

        std::vector<fs::path> findExternalFolder(SaveProject* saveProject);
        int readExternalFiles(SaveProject* saveProject);

        MU static int writeExternalFolder(MU const fs::path& outDirPath) {
            return NOT_IMPLEMENTED;
        }
    };
}


