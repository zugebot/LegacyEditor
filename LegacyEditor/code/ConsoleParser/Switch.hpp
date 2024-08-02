#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/tinf/tinf.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {

    class Switch : public ConsoleParser {
    public:


        Switch() {
            myConsole = lce::CONSOLE::SWITCH;
        }


        ~Switch() override = default;


        int read(editor::FileListing* theListing, const fs::path& inFilePath) override {
            myFilePath = inFilePath;

            int status = deflateListing(theListing);
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            status = readFileInfo(theListing);
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            readExternalFiles(theListing);

            return SUCCESS;
        }


        int deflateListing(editor::FileListing* theListing) override {
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

            status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        int readExternalFiles(editor::FileListing* theListing) {
            fs::path folder = myFilePath;
            folder.replace_extension(".sub");

            int status;
            if (!fs::is_directory(folder)) {
                printf("Failed to read associated external files.\n");
                return STATUS::FILE_ERROR;
            } else if (!folder.empty()) {
                status = readExternalFolder(theListing, folder);
                theListing->myHasSeparateRegions = true;
            } else {
                printf("Failed to find associated external files.\n");
                status = STATUS::FILE_ERROR;
            }
            return status;
        }


        ND int write(MU editor::FileListing* theListing, MU const editor::ConvSettings& theSettings) const override {
            printf("Switch.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }


        MU static int writeExternalFolder(MU FileListing* theListing,
                                           MU const fs::path& outDirPath) {
            printf("FileListing::writeExternalFolder: not implemented!");
            return NOT_IMPLEMENTED;
        }
    };
}
