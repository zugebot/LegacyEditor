#pragma once

#include <vector>
#include <iostream>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    class PS3 : public ConsoleParser {
    public:


        PS3() {
            myConsole = lce::CONSOLE::PS3;
        }


        ~PS3() override = default;


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

            readPARAM_SFO(theListing);

            return SUCCESS;
        }


        /// ps3 writeFile files don't need decompressing\n
        /// TODO: figure out if this comment is actually important or not
        /// TODO: check from regionFile chunk what console it is if uncompressed
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

            u32 final_size = headerUnion.getDestSize();
            if(!data.allocate(final_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, final_size);
            }

            input_size -= 12;
            Data src;
            src.setScopeDealloc(true);
            if(!src.allocate(input_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            fseek(f_in, 12, SEEK_SET);
            fread(src.start(), 1, src.size, f_in);
            fclose(f_in);

            tinf_uncompress(data.start(), &final_size, src.start(), src.getSize());
            if (final_size == 0) {
                return printf_err(DECOMPRESS, "%s", ERROR_3);
            }

            int status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        MU static int readMETADATA() {
            return NOT_IMPLEMENTED;
        }


        // TODO: make it not case-specific!
        // TODO: make it return a status!
        int readPARAM_SFO(editor::FileListing* theListing) {
            fs::path sfoFilePath = myFilePath.parent_path();
            sfoFilePath += "PARAM.SFO";

            SFOManager mainSFO(sfoFilePath.string());
            auto attrs = mainSFO.getAttributes();
            for (const auto& attr : attrs) {
                std::cout << attr.toString() << std::endl;
            }

            const std::wstring sub_title = stringToWstring(mainSFO.getAttribute("SUB_TITLE"));
            theListing->fileInfo.basesavename = sub_title;

            return SUCCESS;
        }


        MU static int readPARAM_PFD() {
            return NOT_IMPLEMENTED;
        }


        int write(MU editor::FileListing* theListing, MU const fs::path& gameDataPath) const override {
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }


        MU static int writeMETADATA() {
            return NOT_IMPLEMENTED;
        }

        MU static int writePARAM_SFO() {
            return NOT_IMPLEMENTED;
        }

        MU static int writePARAM_PFD() {
            return NOT_IMPLEMENTED;
        }


    };


}

