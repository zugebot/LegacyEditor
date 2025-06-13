#pragma once

#include "common/data/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    /// NOT FINISHED
    class MU Xbox360BIN : public ConsoleParser {
    public:

        Xbox360BIN() { m_console = lce::CONSOLE::XBOX360; }
        ~Xbox360BIN() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int inflateListing(SaveProject& saveProject) override;
        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;

    };

}
