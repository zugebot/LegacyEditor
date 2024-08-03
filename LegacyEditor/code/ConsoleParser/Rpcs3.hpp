#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"
#include "LegacyEditor/utils/RLE/rle_vita.hpp"

#include "ConsoleParser.hpp"
#include "include/png/crc.hpp"


namespace editor {

    class FileListing;


    class RPCS3 : public ConsoleParser {
    public:
        friend class FileListing;


        RPCS3() {
            myConsole = lce::CONSOLE::RPCS3;
        }


        ~RPCS3() override = default;


        int read(editor::FileListing* theListing, const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            int status = inflateListing();
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }
            readFileInfo();
            readPARAM_SFO();

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
            c_u64 input_size = ftell(f_in);
            fseek(f_in, 0, SEEK_SET);
            if (input_size < 12) {
                fclose(f_in);
                return printf_err(FILE_ERROR, ERROR_5);
            }
            HeaderUnion headerUnion{};
            fread(&headerUnion, 1, 12, f_in);

            if(!data.allocate(input_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            fseek(f_in, 0, SEEK_SET);
            fread(data.start(), 1, data.size, f_in);
            fclose(f_in);

            int status = readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        // TODO: make it not case-specific!
        // TODO: make it return a status!
        int readPARAM_SFO() {
            fs::path sfoFilePath = myFilePath.parent_path();
            sfoFilePath += "/PARAM.SFO";

            // TODO: make it cache the ACCOUNT_ID for later converting
            SFOManager mainSFO(sfoFilePath.string());
            const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUB_TITLE"));
            myListingPtr->fileInfo.basesavename = subtitle;

            return SUCCESS;
        }


        // TODO: missing other files
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
            if (status != 0) return printf_err(status,
                "failed to write fileInfo to \"%s\"\n",
                fileInfoPath.string().c_str());


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
            Data inflatedData = writeListing(myConsole);
            inflatedData.setScopeDealloc(true);
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
            int status3 = managerMETADATA.writeToFile(metadataPath);
            if (status3 != 0) return printf_err(status3,
                "failed to write metadata to \"%s\"\n",
                metadataPath.string().c_str());


            // PARAM.SFO
            fs::path sfoPath = rootPath / "PARAM.SFO";
            SFOManager sfo;
            sfo.addParam(eSFO_FMT::INT, "*GAMEDATA", "0");
            sfo.addParam(eSFO_FMT::INT, "*ICON0.PNG", "0");
            sfo.addParam(eSFO_FMT::INT, "*METADATA", "1");
            sfo.addParam(eSFO_FMT::INT, "*THUMB", "0");
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", "0000000000000000");
            sfo.addParam(eSFO_FMT::INT, "ATTRIBUTE", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "CATEGORY", "SD");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", " ");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS", "");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS2", "");
            sfo.addParam(eSFO_FMT::INT, "PARENTAL_LEVEL", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "RPCS3_BLIST", "ICON0.PNG/METADATA/THUMB/GAMEDATA");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_DIRECTORY", folderName);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_LIST_PARAM", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SUB_TITLE", wStringToString(myListingPtr->fileInfo.basesavename));
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


        ND int deflateListing(const fs::path& gameDataPath, Data& inflatedData, MU Data& deflatedData) const override {
            deflatedData.steal(inflatedData);

            // file operations
            int status = DataManager(deflatedData).writeToFile(gameDataPath);
            if (status != 0) return printf_err(status,
                "failed to write savefile to \"%s\"\n",
                gameDataPath.string().c_str());
            return SUCCESS;
        }





    };


}
