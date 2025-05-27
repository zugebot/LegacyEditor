#pragma once

#include "include/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    class Vita : public ConsoleParser {
    public:

        Vita() { m_console = lce::CONSOLE::VITA; }
        ~Vita() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int inflateListing(SaveProject& saveProject) override;
        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, MU Buffer& deflatedData) const override;
    };

}