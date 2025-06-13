#pragma once

#include "include/lce/processor.hpp"

#include "common/data/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {
    class FileListing;
    class SaveProject;

    class PS3 : public ConsoleParser {
    public:
        PS3() { m_console = lce::CONSOLE::PS3; }
        ~PS3() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int inflateListing(SaveProject& saveProject) override;
        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;


        int readParamSfo(SaveProject& saveProject);
    };
}

