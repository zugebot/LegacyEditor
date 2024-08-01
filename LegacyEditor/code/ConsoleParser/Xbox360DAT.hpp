#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"
#include "LegacyEditor/utils/XBOX_LZX/XboxCompression.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    /// NOT FINISHED
    class Xbox360DAT : public ConsoleParser {
    public:


        Xbox360DAT() {
            myConsole = lce::CONSOLE::XBOX360;
        }


        ~Xbox360DAT() override = default;


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

            return SUCCESS;
        }


        // TODO: allocating memory from file_size, but then updating that size from XDecompress?
        int deflateListing(editor::FileListing* theListing) override {
            Data data;
            data.setScopeDealloc(true);

            FILE *f_in = fopen(myFilePath.string().c_str(), "rb");
            if (f_in == nullptr) {
                return printf_err(FILE_ERROR, ERROR_4, myFilePath.string().c_str());
            }

            fseek(f_in, 0, SEEK_END);
            c_u64 input_size = ftell(f_in);
            fseek(f_in, 0, SEEK_SET);
            if (input_size < 12) {
                fclose(f_in);
                return printf_err(FILE_ERROR, ERROR_5);
            }
            HeaderUnion headerUnion{};
            fread(&headerUnion, 1, 12, f_in);

            c_u32 file_size = headerUnion.getInt3();
            if(!data.allocate(file_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, file_size);
            }

            c_u32 src_size = headerUnion.getInt1() - 8;
            Data src;
            src.setScopeDealloc(true);
            if(!src.allocate(src_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            fseek(f_in, 12, SEEK_SET); // needs to be authenticated
            fread(src.start(), 1, src.size, f_in);
            fclose(f_in);

            data.size = XDecompress(data.start(), &data.size, src.start(), src.getSize());
            if (data.size == 0) {
                return printf_err(DECOMPRESS, "%s", ERROR_3);
            }

            int status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return status;
            }

            return SUCCESS;
        }



        ND int write(MU editor::FileListing* theListing, MU const fs::path& gameDataPath) const override {
            printf("Xbox360DAT.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
