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
            }

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

            status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        ND int write(editor::FileListing* theListing, const fs::path& gameDataPath) const override {


            // GAMEDATA
            Data deflatedData = ConsoleParser::writeListing(theListing, myConsole);
            deflatedData.setScopeDealloc(true);
            Data inflatedData;
            inflatedData.setScopeDealloc(true);
            int status = inflateListing(gameDataPath, deflatedData, inflatedData);
            if (status != 0) {
                return printf_err(status, "failed to compress fileListing");
            }


            // fileInfo
            fs::path fileInfoPath = gameDataPath;
            fileInfoPath += ".ext";

            Data outData2 = theListing->fileInfo.writeFile(fileInfoPath, myConsole);
            outData2.setScopeDealloc(true);
            printf("fileInfo final size: %u\n", outData2.size);
            // file operations
            int status2 = DataManager(outData2).writeToFile(fileInfoPath);
            if (status2 != 0) return printf_err(status2,
                "failed to write fileInfo to \"%s\"\n",
                fileInfoPath.string().c_str());

            return SUCCESS;
        }


        ND int inflateListing(const fs::path& gameDataPath, const Data& deflatedData, MU Data& inflatedData) const override {
            u64 src_size = deflatedData.size;
            uLong compressedSize = compressBound(src_size);
            printf("compressed bound: %lu\n", compressedSize);
            u8_vec outData1(compressedSize);
            if (compress(outData1.data(), &compressedSize,
                         deflatedData.data, deflatedData.size) != Z_OK) {
                return COMPRESS;
            }
            outData1.resize(compressedSize);
            printf("Writing final size: %zu\n", outData1.size());

            // file operations
            FILE *f_out = fopen(gameDataPath.string().c_str(), "wb");
            if (f_out == nullptr) {
                printf("failed to write savefile to \"%s\"\n",
                       gameDataPath.string().c_str());
                return FILE_ERROR;
            }
            if (isSystemLittleEndian()) {
                src_size = swapEndian64(src_size);
            }
            fwrite(&src_size, sizeof(u64), 1, f_out);
            fwrite(outData1.data(), 1, outData1.size(), f_out);
            fclose(f_out);

            return SUCCESS;
        }


    };


}
