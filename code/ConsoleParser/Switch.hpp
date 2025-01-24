#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/tinf/tinf.h"

#include "code/FileListing/fileListing.hpp"
#include "common/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {

    class Switch : public ConsoleParser {
    public:


        Switch() {
            myConsole = lce::CONSOLE::SWITCH;
        }


        ~Switch() override = default;


        int read(editor::FileListing* theListing, const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            int status = inflateListing();
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            readFileInfo();
            readExternalFiles();

            return SUCCESS;
        }


        int inflateListing() override {
            Data data;
            data.setScopeDealloc(true);

            FILE *f_in = fopen(myFilePath.string().c_str(), "rb");
            if (f_in == nullptr) {
                return printf_err(FILE_ERROR, ERROR_4, myFilePath.string().c_str());
            }

            fseek(f_in, 0, SEEK_END);
            u64 input_size = ftell(f_in);
            fseek(f_in, 0, SEEK_SET);
            if (input_size < 12) {
                fclose(f_in);
                return printf_err(FILE_ERROR, ERROR_5);
            }
            HeaderUnion headerUnion{};
            fread(&headerUnion, 1, 12, f_in);

            u32 final_size = headerUnion.getInt2Swap();
            if(!data.allocate(final_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, final_size);
            }

            input_size -= 8;
            Data src;
            src.setScopeDealloc(true);
            if(!src.allocate(input_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            fseek(f_in, 8, SEEK_SET);
            fread(src.start(), 1, input_size, f_in);
            fclose(f_in);

            int status = tinf_zlib_uncompress(data.start(), &data.size, src.start(), input_size);
            if (status != 0) {
                return DECOMPRESS;
            }

            status = ConsoleParser::readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        int readExternalFiles() {
            fs::path folder = myFilePath;
            folder.replace_extension(".sub");

            int status;
            if (!fs::is_directory(folder)) {
                return printf_err(FILE_ERROR, "Failed to read associated external files.\n");
            } else if (!folder.empty()) {
                status = readExternalFolder(folder);
                myListingPtr->myReadSettings.setHasSepRegions(true);
            } else {
                return printf_err(FILE_ERROR, "Failed to find associated external files.\n");
            }
            return status;
        }


        ND int write(MU editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;

            printf("Switch.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int deflateListing(MU const fs::path& gameDataPath, MU Data& inflatedData, MU Data& deflatedData) const override {
            return NOT_IMPLEMENTED;
        }


        MU static int writeExternalFolder(MU const fs::path& outDirPath) {
            printf("FileListing::writeExternalFolder: not implemented!");
            return NOT_IMPLEMENTED;
        }
    };
}
