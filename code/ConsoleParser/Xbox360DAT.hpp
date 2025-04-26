#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "code/FileListing/fileListing.hpp"
#include "common/XBOX_LZX/XDecompress.hpp"
#include "common/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    /// NOT FINISHED
    class Xbox360DAT : public ConsoleParser {
    public:


        Xbox360DAT() {
            myConsole = lce::CONSOLE::XBOX360;
        }


        ~Xbox360DAT() override = default;


        int read(editor::FileListing* theListing, const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            int status = inflateListing();
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            readFileInfo();

            return SUCCESS;
        }


        // TODO: allocating memory from file_size, but then updating that size from XDecompress?
        int inflateListing() override {
            Data inflatedData;
            inflatedData.setScopeDealloc(true);

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
            if(!inflatedData.allocate(file_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, file_size);
            }

            c_u32 src_size = headerUnion.getInt1() - 8;
            Data deflatedData;
            deflatedData.setScopeDealloc(true);
            if(!deflatedData.allocate(src_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            fseek(f_in, 12, SEEK_SET); // needs to be authenticated
            fread(deflatedData.start(), 1, deflatedData.size, f_in);
            fclose(f_in);

            int error = XDecompress(inflatedData.start(), &inflatedData.size,
                                    deflatedData.start(), deflatedData.getSize());
            if (error != 0) {
                return printf_err(DECOMPRESS, "%s", ERROR_3);
            }

            DataManager out(inflatedData);
            // out.writeToFile(R"(C:\Users\jerrin\Desktop\OUT\XBOX360_TU74.decompressed_dat)");

            if (inflatedData.size == 0) {
                return printf_err(DECOMPRESS, "%s", ERROR_3);
            }

            int status = ConsoleParser::readListing(inflatedData);

            return status;
        }


        ND int write(MU editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;

            printf("Xbox360DAT.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int deflateListing(MU const fs::path& gameDataPath, MU Data& inflatedData, MU Data& deflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
