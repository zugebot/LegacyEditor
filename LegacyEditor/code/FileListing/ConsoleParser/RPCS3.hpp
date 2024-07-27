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


        int read(editor::FileListing* theListing, const fs::path& inFilePath) override {
            myFilePath = inFilePath;

            int status = deflateListing(theListing);
            if (status != 0) {
                printf("failed to extract listing\n");
                return status;
            }

            status = readFileInfo(theListing);

            theListing->fileInfo.basesavename = L"hi";

            readPARAM_SFO(theListing);

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

            int status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        // TODO: make it not case-specific!
        // TODO: make it return a status!
        int readPARAM_SFO(editor::FileListing* theListing) {
            fs::path sfoFilePath = myFilePath.parent_path();
            sfoFilePath += "/PARAM.SFO";

            // TODO: make it cache the ACCOUNT_ID for later converting
            SFOManager mainSFO(sfoFilePath.string());
            auto attrs = mainSFO.getAttributes();
            // for (const auto& attr : attrs) {
            //     std::cout << attr.toString() << std::endl;
            // }
            // mainSFO.editParam("ACCOUNT_ID", "123456789ABCDEF0");
            // mainSFO.editParam("TITLE", "Fortnite Gaming Edition");
            // mainSFO.editParam("SUB_TITLE", "Extreme Golfing");
            // fs::path path = getFolderDirectory() / "testing 1 2 3";
            // mainSFO.saveToFile(path.string());

            const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUB_TITLE"));
            theListing->fileInfo.basesavename = subtitle;

            return SUCCESS;
        }


        // TODO: missing other files
        ND int write(editor::FileListing* theListing, const fs::path& gameDataPath) const override {

            // fileInfo
            fs::path fileInfoPath = gameDataPath.parent_path();
            fileInfoPath /= "THUMB";
            Data fileInfoData = theListing->fileInfo.writeFile(fileInfoPath, myConsole);
            fileInfoData.setScopeDealloc(true);
            printf("fileInfo final size: %u\n", fileInfoData.size);
            // file operations
            int status2 = DataManager(fileInfoData).writeToFile(fileInfoPath);
            if (status2 != 0) return printf_err(status2,
                                                "failed to write fileInfo to \"%s\"\n",
                                                fileInfoPath.string().c_str());


            // write ICON0.PNG
            if (theListing->icon0png.myData == nullptr) {
                Picture fileInfoPng;
                fileInfoPng.loadFromFile(fileInfoPath.string().c_str());
                theListing->icon0png.allocate(320, 176, 4);
                theListing->icon0png.fillColor(0, 0, 0);
                theListing->icon0png.placeAndStretchSubImage(&fileInfoPng, 72, 0, 176, 176);
                // theListing->icon0png.placeSubImage(&fileInfoPng, 128, 56);
            }
            fs::path icon0pngPath = gameDataPath.parent_path();
            icon0pngPath /= "ICON0.PNG";
            icon0pngPath.make_preferred();
            theListing->icon0png.saveWithName(icon0pngPath.string());


            // GAMEDATA
            Data deflatedData = ConsoleParser::writeListing(theListing, myConsole);
            Data inflatedData;
            inflatedData.setScopeDealloc(true);
            int status = inflateListing(gameDataPath, deflatedData, inflatedData);
            if (status != 0) {
                return printf_err(status, "failed to compress fileListing");
            }


            // METADATA
            // TODO: ensure endian is correct
            fs::path metadataPath = gameDataPath.parent_path();
            metadataPath /= "METADATA";
            // TODO: replace with u8[256]
            Data _;
            _.setScopeDealloc(true);
            _.allocate(256);
            DataManager managerMETADATA(_);
            std::memset(managerMETADATA.data, 0, 256);
            managerMETADATA.writeInt32(3);
            managerMETADATA.writeInt32(deflatedData.size);
            managerMETADATA.writeInt32(fileInfoData.size);

            u32 crc1 = crc(deflatedData.data, deflatedData.size);
            managerMETADATA.writeInt32(crc1);

            u32 crc2 = crc(fileInfoData.data, fileInfoData.size);
            managerMETADATA.writeInt32(crc2);
            // file operations
            int status3 = managerMETADATA.writeToFile(metadataPath);
            if (status3 != 0) return printf_err(status3,
                "failed to write metadata to \"%s\"\n",
                metadataPath.string().c_str());


            // write PARAM.SFO
            // ...


            return SUCCESS;
        }


        ND int inflateListing(const fs::path& gameDataPath, const Data& deflatedData, MU Data& inflatedData) const override {
            printf("gamedata final size: %u\n", deflatedData.size);
            // file operations
            int status1 = DataManager(deflatedData).writeToFile(gameDataPath);
            if (status1 != 0) return printf_err(status1,
                "failed to write savefile to \"%s\"\n",
                gameDataPath.string().c_str());
            return SUCCESS;
        }





    };


}