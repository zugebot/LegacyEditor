#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {

    class PS4 : public ConsoleParser {
    public:


        PS4() {
            myConsole = lce::CONSOLE::PS4;
        }


        ~PS4() override = default;


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


        static int readExternalFiles(editor::FileListing* theListing) {
            auto folders = PS4::findExternalFolder(theListing);
            int status;
            for (c_auto& folder : folders) {
                status = readExternalFolder(theListing, folder);
                if (status != 0) {
                    printf("Failed to read associated external files.\n");
                    break;
                }
            }
            theListing->myHasSeparateRegions = true;
            return status;
        }


        static std::vector<fs::path> findExternalFolder(FileListing* theFileListing) {
            // go from "root/00000001/savedata0/GAMEDATA" to "root/00000001/savedata0"
            const fs::path mainDirPath = theFileListing->myFilePath.parent_path();


            // get sfo data from "root/00000001/savedata0/sce_sys/param.sfo"
            const fs::path sfoFilePath = mainDirPath / "sce_sys" / "param.sfo";
            if (!fs::exists(sfoFilePath)) {
                printf("input folder does not have a sce_sys/param.sfo, returning early");
                return {""};
            }

            // get the "CUSA00744-240620222358.0"-alike str from the main "param.sfo"
            SFOManager mainSFO(sfoFilePath.string());
            const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUBTITLE"));
            theFileListing->fileInfo.basesavename = subtitle;


            std::string mainAttr = mainSFO.getAttribute("SAVEDATA_DIRECTORY");
            auto mainAttrParts = split(mainAttr, '.');

            if (mainAttrParts.size() != 2) {
                printf("main param.sfo does not seem to be formatted correctly.");
                return {""};
            }

            // go from "root/00000001/savedata0" to "root"
            const fs::path toCheckDirPath = mainDirPath.parent_path().parent_path();
            // the vector of directories to add regions from
            std::vector<fs::path> directoriesToReturn{};
            // checks each folder in "root"
            for (c_auto &entry: fs::directory_iterator(toCheckDirPath)) {
                // skip entries that are not directories
                if (!fs::is_directory(entry.status())) {
                    continue;
                }

                // skips checking the input folder
                if (mainDirPath.parent_path() == entry.path()) {
                    continue;
                }

                fs::path tempCheckDirPath = entry.path() / "savedata0";
                if (!fs::exists(tempCheckDirPath)) {
                    continue;
                }

                fs::path tempSFOFilePath = entry.path() / "savedata0" / "sce_sys" / "param.sfo";
                if (!fs::exists(tempSFOFilePath)) {
                    continue;
                }

                // get the "CUSA00744-240620222358.0"-alike str from the temp "param.sfo"
                SFOManager tempSFO(tempSFOFilePath.string());
                std::string tempAttr = tempSFO.getAttribute("SAVEDATA_DIRECTORY");
                auto tempAttrParts = split(tempAttr, '.');

                if (tempAttrParts.size() != 2) {
                    printf("to check param.sfo does not seem to be formatted correctly.");
                    continue;
                }

                // skip PS4 worlds that are not the same as the one being looked for
                if (mainAttrParts[0] != tempAttrParts[0]) {
                    continue;
                }

                directoriesToReturn.push_back(tempCheckDirPath);
            }

            return directoriesToReturn;
        }


        ND int write(MU editor::FileListing* theListing, MU const fs::path& gameDataPath) const override {
            printf("PS4.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }



        MU static int writeExternalFolder(MU FileListing* theFileListing, MU const fs::path& outDirPath) {
            return NOT_IMPLEMENTED;
        }


    };
}

