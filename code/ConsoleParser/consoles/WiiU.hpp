#pragma once



#include "include/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    class WiiU : public ConsoleParser {
    public:

        WiiU() { m_console = lce::CONSOLE::WIIU; }
        ~WiiU() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject* saveProject) const override {}

        ND int inflateFromLayout(const fs::path& theFilePath, SaveProject* saveProject) override;
        ND int inflateListing(SaveProject* saveProject) override;
        ND int deflateToSave(SaveProject* saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, MU Buffer& deflatedData) const override;
    };


}
