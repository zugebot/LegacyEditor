#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/utils.hpp"
#include "LegacyEditor/code/BinFile/BINSupport.hpp"
#include "LegacyEditor/utils/XBOX_LZX/XboxCompression.hpp"

#include "ConsoleParser.hpp"


namespace editor {


    /// NOT FINISHED
    class MU Xbox360BIN : public ConsoleParser {
    public:


        Xbox360BIN() {
            myConsole = lce::CONSOLE::XBOX360;
        }


        ~Xbox360BIN() override = default;



        // TODO: IDK if it should but it is for now, get fileInfo out of it, fix memory leaks
        int read(editor::FileListing* theListing, const fs::path& inFilePath) override {
            FILE *f_in = fopen(inFilePath.string().c_str(), "rb");
            if (f_in == nullptr) {
                return printf_err(FILE_ERROR, ERROR_4, inFilePath.string().c_str());
            }

            fseek(f_in, 0, SEEK_END);
            c_u64 input_size = ftell(f_in);
            fseek(f_in, 0, SEEK_SET);
            if (input_size < 12) {
                return printf_err(FILE_ERROR, ERROR_5);
            }
            HeaderUnion headerUnion{};
            fread(&headerUnion, 1, 12, f_in);

            Data bin;
            bin.allocate(input_size);

            fseek(f_in, 0, SEEK_SET);
            fread(bin.start(), 1, input_size, f_in);

            DataManager binFile(bin.data, input_size);
            StfsPackage stfsInfo(binFile);
            stfsInfo.parse();
            StfsFileListing listing = stfsInfo.getFileListing();

            StfsFileEntry* entry = findSavegameFileEntry(listing);
            if (entry == nullptr) {
                bin.deallocate();
                return {};
            }

            /*
            BINHeader meta = stfsInfo.getMetaData();
            fileInfo.basesavename = meta.displayName;
            fileInfo.thumbnail = meta.thumbnailImage;
            fileInfo.exploredchunks;
            fileInfo.extradata;
            fileInfo.hostoptions;
            fileInfo.loads;
            fileInfo.seed;
            fileInfo.settings;
            fileInfo.texturepack;
            */

            const Data _ = stfsInfo.extractFile(entry);
            DataManager out(_);

            bin.deallocate();

            c_u32 srcSize = out.readInt32() - 8;

            Data data;
            data.setScopeDealloc(true);
            data.size = out.readInt64();

            if (!data.allocate(data.size)) {
                return MALLOC_FAILED;
            }

            data.size = XDecompress(
                    data.start(), &data.size,
                    out.ptr, srcSize);

            int status = ConsoleParser::readListing(theListing, data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        int deflateListing(MU editor::FileListing* theListing) override {
            return NOT_IMPLEMENTED;
        }


        int readFileInfo(MU editor::FileListing* theListing) const override {
            return NOT_IMPLEMENTED;
        }


        ND int write(MU editor::FileListing* theListing, MU editor::ConvSettings& theSettings) const override {
            printf("Xbox360BIN.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int inflateListing(MU const fs::path& gameDataPath, MU const Data& deflatedData, MU Data& inflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
