#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "code/FileListing/fileListing.hpp"
#include "common/utils.hpp"

#include "ConsoleParser.hpp"
#include "common/RLE/rle_vita.hpp"


namespace editor {


    class Vita : public ConsoleParser {
    public:


        Vita() {
            myConsole = lce::CONSOLE::VITA;
        }


        ~Vita() override = default;


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

            input_size -= 8;
            Data src;
            src.setScopeDealloc(true);
            if(!src.allocate(input_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            // goto offset 8 for the data, read data into src
            fseek(f_in, 8, SEEK_SET);
            fread(src.data, 1, input_size, f_in);
            fclose(f_in);

            RLEVITA_DECOMPRESS(src.data, src.size, data.data, data.size);

            int status = ConsoleParser::readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        ND int write(editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;
            int status;
            fs::path rootPath = theSettings.getInFolderPath();


            // FIND PRODUCT CODE
            auto productCode = theSettings.myProductCodes.getVITA();
            std::string strProductCode = ProductCodes::toString(productCode);
            std::string strCurrentTime = getCurrentDateTimeString();
            std::string folderName = strProductCode + "--" + strCurrentTime;
            rootPath /= folderName;
            fs::create_directories(rootPath);


            // GAMEDATA
            fs::path gameDataPath = rootPath / "GAMEDATA.bin";
            Data deflatedData = ConsoleParser::writeListing(myConsole);
            deflatedData.setScopeDealloc(true);
            Data inflatedData;
            inflatedData.setScopeDealloc(true);
            status = deflateListing(gameDataPath, deflatedData, inflatedData);
            if (status != 0) return printf_err(status,
                "failed to compress fileListing\n");
            theSettings.setOutFilePath(gameDataPath);
            printf("gamedata final size: %u\n", deflatedData.size);


            // FILE INFO
            fs::path fileInfoPath = rootPath / "THUMBDATA.bin";
            Data fileInfoData = myListingPtr->fileInfo.writeFile(fileInfoPath, myConsole);
            fileInfoData.setScopeDealloc(true);
            status = DataManager(fileInfoData).writeToFile(fileInfoPath);
            if (status != 0) return printf_err(status,
                "failed to write fileInfo to \"%s\"\n",
                fileInfoPath.string().c_str());


            return SUCCESS;
        }


        ND int deflateListing(const fs::path& gameDataPath, Data& inflatedData, MU Data& deflatedData) const override {
            deflatedData.allocate(inflatedData.size + 2);

            deflatedData.size = RLEVITA_COMPRESS(
                    inflatedData.data, inflatedData.size,
                    deflatedData.data, deflatedData.size);
            FILE *f_out = fopen(gameDataPath.string().c_str(), "wb");
            if (f_out == nullptr) return printf_err(FILE_ERROR,
                "failed to write savefile to \"%s\"\n",
                gameDataPath.string().c_str());

            // 4-bytes of '0'
            // 4-bytes of total decompressed fileListing size
            // N-bytes fileListing data
            constexpr int num = 0;
            fwrite(&num, sizeof(u32), 1, f_out);
            u32 compSize = inflatedData.size;
            if (!isSystemLittleEndian()) {
                compSize = swapEndian32(compSize);
            }
            fwrite(&compSize, sizeof(u32), 1, f_out);
            fwrite(deflatedData.data, 1, deflatedData.size, f_out);
            fclose(f_out);

            return SUCCESS;
        }


    };


}