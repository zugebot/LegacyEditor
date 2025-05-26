#pragma once

#include "include/lce/processor.hpp"

#include "include/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class NewGenConsoleParser : public ConsoleParser {
    public:
        ~NewGenConsoleParser() override = default;

    protected:
        int inflateListing(SaveProject* saveProject) override;

        int readExternalFolder(const fs::path& inDirPath, SaveProject* saveProject);
    };

}
