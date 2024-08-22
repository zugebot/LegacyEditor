#pragma once

#include <vector>
#include <iostream>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"
#include "include/png/crc.hpp"
#include "include/zlib-1.2.12/zlib.h"


namespace editor {

    class FileListing;


    class PS3 : public ConsoleParser {
    public:
        friend class FileListing;


        PS3() {
            myConsole = lce::CONSOLE::PS3;
        }


        ~PS3() override = default;


        int read(editor::FileListing* theListing, const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            int status = inflateListing();
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            readFileInfo();
            readParamSfo();

            return SUCCESS;
        }


        /// ps3 writeFile files don't need decompressing\n
        /// TODO: figure out if this comment is actually important or not
        /// TODO: check from regionFile chunk what console it is if uncompressed
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

            int status = ConsoleParser::readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        // TODO: make it return a status!
        // TODO: it should read more than just this probably!
        int readParamSfo() {
            fs::path sfoFilePath = myFilePath.parent_path();
            sfoFilePath /= "PARAM.SFO";

            // TODO: make it cache the ACCOUNT_ID for later converting
            SFOManager mainSFO(sfoFilePath.string());
            const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUB_TITLE"));
            myListingPtr->fileInfo.baseSaveName = subtitle;

            return SUCCESS;
        }


        // TODO: missing PARAM.PFD
        ND int write(editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;
            int status;
            fs::path rootPath = theSettings.getInFolderPath();


            // FIND PRODUCT CODE
            auto productCode = theSettings.myProductCodes.getPS3();
            std::string strProductCode = ProductCodes::toString(productCode);
            std::string strCurrentTime = getCurrentDateTimeString();
            std::string folderName = strProductCode + "--" + strCurrentTime;
            rootPath /= folderName;
            fs::create_directories(rootPath);


            // FILE INFO
            fs::path fileInfoPath = rootPath / "THUMB";
            Data fileInfoData = myListingPtr->fileInfo.writeFile(fileInfoPath, myConsole);
            fileInfoData.setScopeDealloc(true);
            status = DataManager(fileInfoData).writeToFile(fileInfoPath);
            if (status != 0)
                return printf_err(status, "failed to write fileInfo to \"%s\"\n", fileInfoPath.string().c_str());


            // ICON0.PNG
            fs::path icon0pngPath = rootPath / "ICON0.PNG";
            if (myListingPtr->icon0png.myData == nullptr) {
                Picture fileInfoPng;
                fileInfoPng.loadFromFile(fileInfoPath.string().c_str());
                myListingPtr->icon0png.allocate(320, 176, 4);
                myListingPtr->icon0png.fillColor(0, 0, 0);
                myListingPtr->icon0png.placeAndStretchSubImage(&fileInfoPng, 72, 0, 176, 176);
            }
            myListingPtr->icon0png.saveWithName(icon0pngPath.string());


            // GAMEDATA
            fs::path gameDataPath = rootPath / "GAMEDATA";
            Data inflatedData = ConsoleParser::writeListing(myConsole);
            Data deflatedData;
            deflatedData.setScopeDealloc(true);
            status = deflateListing(gameDataPath, inflatedData, deflatedData);
            if (status != 0) return printf_err(status,
                "failed to compress fileListing\n");
            theSettings.setOutFilePath(gameDataPath);
            printf("gamedata final size: %u\n", deflatedData.size);


            // METADATA
            fs::path metadataPath = rootPath / "METADATA";
            c_u32 crc1 = crc(deflatedData.data, deflatedData.size);
            c_u32 crc2 = crc(fileInfoData.data, fileInfoData.size);
            u8 metadata[256] = {0};
            DataManager managerMETADATA(metadata, 256);
            managerMETADATA.writeInt32(3);
            managerMETADATA.writeInt32(deflatedData.size);
            managerMETADATA.writeInt32(fileInfoData.size);
            managerMETADATA.writeInt32(crc1);
            managerMETADATA.writeInt32(crc2);
            status = managerMETADATA.writeToFile(metadataPath);
            if (status != 0) return printf_err(status,
                "failed to write metadata to \"%s\"\n",
                metadataPath.string().c_str());


            // PARAM.SFO
            // TODO: One of the PARAM's or SAVEDATA_LIST_PARAM contain a copy of the ACCOUNT_ID, and maybe other data?
            fs::path sfoPath = rootPath / "PARAM.SFO";
            SFOManager sfo;
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", "0000000000000000");
            sfo.addParam(eSFO_FMT::INT, "ATTRIBUTE", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "CATEGORY", "SD");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", " ");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS", "");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS2", "");
            sfo.addParam(eSFO_FMT::INT, "PARENTAL_LEVEL", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_DIRECTORY", folderName);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_LIST_PARAM", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SUB_TITLE", wStringToString(myListingPtr->fileInfo.baseSaveName));
            std::string title = "Minecraft: PlayStation®3 Edition";
            if (productCode == ePS3ProductCode::BLES01976 ||
                productCode == ePS3ProductCode::BLUS31426) {
                title += " (" + ProductCodes::toString(productCode) + ")";
            }
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "TITLE", title);
            sfo.setMagic(eSFO_MAGIC::PS3_HDD);
            sfo.saveToFile(sfoPath.string());


            return SUCCESS;
        }


        ND int deflateListing(MU const fs::path& gameDataPath, MU Data& inflatedData, MU Data& deflatedData) const override {

            MU uLong deflatedSize = compressBound(static_cast<uLong>(inflatedData.size));




            return NOT_IMPLEMENTED;
        }




    };


}

