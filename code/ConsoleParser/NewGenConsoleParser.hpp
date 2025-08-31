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
        static int readExternalFolder(SaveProject& saveProject, const fs::path& inDirPath);

        int deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const override {
            return -1;
        }
        virtual int deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) {
            return -1;
        }
        virtual int writeExternalFolders(SaveProject& saveProject, WriteSettings& theSettings) const {
            return -1;
        }




    };

}
