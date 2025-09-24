#pragma once



#include "common/data/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    class NewGenMCS : public ConsoleParser {
    public:

        NewGenMCS() { m_console = lce::CONSOLE::NEWGENMCS; }
        ~NewGenMCS() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject& saveProject) const override {}

        ND int inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) override;
        ND int inflateListing(SaveProject& saveProject) override;
        ND int deflateToSave(SaveProject& saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, MU Buffer& deflatedData) const override;

        ND std::optional<fs::path> getFileInfoPath(SaveProject& saveProject) const override;
    };


}
