#pragma once

#include "include/lce/processor.hpp"

#include "common/data/ghc/fs_std.hpp"
#include "code/ConsoleParser/ConsoleParser.hpp"


namespace editor {

    class NewGenConsoleParser : public ConsoleParser {
    public:
        ~NewGenConsoleParser() override = default;

    protected:
        int inflateListing(SaveProject& saveProject) override;

        virtual std::vector<fs::path> findExternalFolder(SaveProject& saveProject) = 0;
        virtual int readExternalFolders(SaveProject& saveProject) = 0;

        virtual int writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) = 0;


        static int readExternalFolder(SaveProject& saveProject, const fs::path& inDirPath);


    };

}
