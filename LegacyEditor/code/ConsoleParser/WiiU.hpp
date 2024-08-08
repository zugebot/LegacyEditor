#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"
#include "include/tinf/tinf.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    class WiiU : public ConsoleParser {
    public:


        WiiU() {
            myConsole = lce::CONSOLE::WIIU;
        }


        ~WiiU() override = default;


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
            if(!src.allocate(input_size * 2)) { // why is the x2 here?
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size * 2); // why is the x2 here?
            }

            fseek(f_in, 8, SEEK_SET);
            fread(src.start(), 1, input_size, f_in);
            fclose(f_in);

            int status = tinf_zlib_uncompress(data.start(), &data.size, src.start(), input_size);
            if (status != 0) {
                return DECOMPRESS;
            }


            // std::string loc = R"(C:\Users\jerrin\Desktop\New folder\OUT\WiiU)";
            // DataManager(data).writeToFile(loc);

            status = ConsoleParser::readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        ND int write(editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            int status;

            myListingPtr = theListing;
            const fs::path rootPath = theSettings.getInFolderPath();


            // GAMEDATA
            fs::path gameDataPath = rootPath / getCurrentDateTimeString();
            Data inflatedData = ConsoleParser::writeListing(myConsole);
            inflatedData.setScopeDealloc(true);
            Data deflatedData;
            deflatedData.setScopeDealloc(true);
            status = deflateListing(gameDataPath, inflatedData, deflatedData);
            if (status != 0)
                return printf_err(status, "failed to compress fileListing\n");
            theSettings.setOutFilePath(gameDataPath);
            printf("gamedata final size: %u\n", deflatedData.size);


            // FILE INFO
            fs::path fileInfoPath = gameDataPath;
            fileInfoPath += ".ext";

            Data fileInfoData = myListingPtr->fileInfo.writeFile(fileInfoPath, myConsole);
            fileInfoData.setScopeDealloc(true);

            status = DataManager(fileInfoData).writeToFile(fileInfoPath);
            if (status != 0) return printf_err(status,
                "failed to write fileInfo to \"%s\"\n",
                fileInfoPath.string().c_str());

            return SUCCESS;
        }


        ND int deflateListing(const fs::path& gameDataPath, Data& inflatedData, MU Data& deflatedData) const override {
            deflatedData.allocate(compressBound(inflatedData.size));

            if (compress(deflatedData.data, reinterpret_cast<uLongf*>(&deflatedData.size),
                         inflatedData.data, inflatedData.size) != Z_OK) {
                return COMPRESS;
            }

            // file operations
            FILE *f_out = fopen(gameDataPath.string().c_str(), "wb");
            if (f_out == nullptr) return printf_err(FILE_ERROR,
                "failed to write savefile to \"%s\"\n",
                gameDataPath.string().c_str());
            uint64_t sizeToWrite = inflatedData.size;
            if (isSystemLittleEndian())
                sizeToWrite = swapEndian64(sizeToWrite);
            fwrite(&sizeToWrite, 8, 1, f_out);
            fwrite(deflatedData.data, 1, deflatedData.size, f_out);
            fclose(f_out);

            return SUCCESS;
        }


    };


}
