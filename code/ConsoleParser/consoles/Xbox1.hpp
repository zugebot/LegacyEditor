#pragma once

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "code/BinFile/BINSupport.hpp"
#include "code/FileListing/fileListing.hpp"
#include "common/codec/XDecompress.hpp"
#include "common/utils.hpp"

#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class FileListing;
    class SaveProject;

    /// NOT FINISHED
    class Xbox1 : public ConsoleParser {
    public:

        Xbox1() { m_console = lce::CONSOLE::XBOX1; }
        ~Xbox1() override = default;

        ND SaveLayout discoverSaveLayout(MU const fs::path& rootFolder) override { return {}; }
        void supplyRequiredDefaults(MU SaveProject* saveProject) const override {}

        ND int inflateFromLayout(const fs::path& theFilePath, MU SaveProject* saveProject) override;
        ND int inflateListing(MU SaveProject* saveProject) override;
        ND int deflateToSave(MU SaveProject* saveProject, MU WriteSettings& theSettings) const override;
        ND int deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const override;


    };

}
