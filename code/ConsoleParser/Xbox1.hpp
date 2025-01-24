#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "code/BinFile/BINSupport.hpp"
#include "code/FileListing/fileListing.hpp"
#include "common/XBOX_LZX/XDecompress.hpp"
#include "common/utils.hpp"

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
        int read(MU editor::FileListing* theListing, MU const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            return NOT_IMPLEMENTED;
        }


        int inflateListing() override {
            return NOT_IMPLEMENTED;
        }


        ND int write(MU editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;

            printf("Xbox1.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int deflateListing(MU const fs::path& gameDataPath, MU Data& inflatedData, MU Data& deflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
