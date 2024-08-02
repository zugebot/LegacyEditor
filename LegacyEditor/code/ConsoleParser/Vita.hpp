#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"

#include "ConsoleParser.hpp"
#include "LegacyEditor/utils/RLE/rle_vita.hpp"


namespace editor {


    class Vita : public ConsoleParser {
    public:


        Vita() {
            myConsole = lce::CONSOLE::VITA;
        }


        ~Vita() override = default;


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
            if(!src.allocate(input_size)) {
                fclose(f_in);
                return printf_err(MALLOC_FAILED, ERROR_1, input_size);
            }

            // goto offset 8 for the data, read data into src
            fseek(f_in, 8, SEEK_SET);
            fread(src.data, 1, input_size, f_in);
            fclose(f_in);

            RLEVITA_DECOMPRESS(src.data, src.size, data.data, data.size);

            int status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        ND int write(editor::FileListing* theListing, MU const editor::ConvSettings& theSettings) const override {
            fs::path gameDataPath = theSettings.getFilePath();

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
            fs::path fileInfoPath = gameDataPath.parent_path();
            fileInfoPath /= "/THUMBDATA.BIN";
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
            Data outData1;
            outData1.setScopeDealloc(true);


            outData1.allocate(deflatedData.size + 2);
            outData1.size = RLEVITA_COMPRESS(
                    deflatedData.data, deflatedData.size,
                    outData1.data, outData1.size);
            // file operations
            FILE *f_out = fopen(gameDataPath.string().c_str(), "wb");
            if (f_out == nullptr) {
                return printf_err(FILE_ERROR,
                                  "failed to write savefile to \"%s\"\n",
                                  gameDataPath.string().c_str());
            }
            // 4-bytes of '0'
            // 4-bytes of total decompressed fileListing size
            // N-bytes fileListing data
            // TODO: figure out endianness
            constexpr int num = 0;
            fwrite(&num, sizeof(u32), 1, f_out);
            u32 compSize = deflatedData.size;
            if (!isSystemLittleEndian()) {
                compSize = swapEndian32(compSize);
            }
            fwrite(&compSize, sizeof(u32), 1, f_out);
            fwrite(outData1.data, 1, outData1.size, f_out);
            fclose(f_out);

            return SUCCESS;
        }


    };


}