#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"
#include "LegacyEditor/code/BinFile/BINSupport.hpp"
#include "LegacyEditor/utils/XBOX_LZX/XboxCompression.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    /// NOT FINISHED
    class Xbox1 : public ConsoleParser {
    public:


        Xbox1() {
            myConsole = lce::CONSOLE::XBOX1;
        }


        ~Xbox1() override = default;


        // TODO: IDK if it should but it is for now, get fileInfo out of it, fix memory leaks
        int read(MU editor::FileListing* theListing, MU const fs::path& inFilePath) override {
            return NOT_IMPLEMENTED;
        }


        int deflateListing(MU editor::FileListing* theListing) override {
            return NOT_IMPLEMENTED;
        }


        int readFileInfo(MU editor::FileListing* theListing) const override {
            return NOT_IMPLEMENTED;
        }


        ND int write(MU editor::FileListing* theListing, MU const editor::ConvSettings& theSettings) const override {
            printf("Xbox1.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
