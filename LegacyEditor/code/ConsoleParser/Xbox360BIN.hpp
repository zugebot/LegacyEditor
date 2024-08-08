#pragma once

#include <vector>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/code/BinFile/BINSupport.hpp"
#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/XBOX_LZX/XDecompress.hpp"
#include "LegacyEditor/utils/utils.hpp"

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
        int read(editor::FileListing* theListing, const fs::path& theFilePath) override {
            myListingPtr = theListing;
            myFilePath = theFilePath;

            FILE *f_in = fopen(myFilePath.string().c_str(), "rb");
            if (f_in == nullptr) {
                return printf_err(FILE_ERROR, ERROR_4, myFilePath.string().c_str());
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
            fread(bin.start(), 1, bin.size, f_in);

            DataManager binFile(bin.data, bin.size);
            StfsPackage stfsInfo(binFile);
            stfsInfo.parse();
            StfsFileListing listing = stfsInfo.getFileListing();

            StfsFileEntry* entry = findSavegameFileEntry(listing);
            if (entry == nullptr) {
                bin.deallocate();
                return {};
            }



            const Data _ = stfsInfo.extractFile(entry);
            DataManager deflatedData(_);

            // stuff I need to figure out
            MU auto createdTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
            BINHeader meta = stfsInfo.getMetaData();
            if (meta.thumbnailImage.size) {
                myListingPtr->fileInfo.readPNG(meta.thumbnailImage);
            }
            myListingPtr->fileInfo.basesavename = stfsInfo.getMetaData().displayName;




            bin.deallocate();

            c_u32 srcSize = deflatedData.readInt32() - 8;

            Data data;
            data.setScopeDealloc(true);
            c_u32 inflatedSize = deflatedData.readInt64();

            if (!data.allocate(inflatedSize)) {
                return MALLOC_FAILED;
            }

            data.size = XDecompress(
                    data.start(), &data.size,
                    deflatedData.ptr, srcSize);

            int status = ConsoleParser::readListing(data);
            if (status != 0) {
                return -1;
            }

            return SUCCESS;
        }


        int inflateListing() override {
            return NOT_IMPLEMENTED;
        }


        void readFileInfo() const override {

        }


        ND int write(MU editor::FileListing* theListing, MU editor::WriteSettings& theSettings) const override {
            myListingPtr = theListing;

            printf("Xbox360BIN.write(): not implemented!\n");
            return NOT_IMPLEMENTED;
        }


        ND int deflateListing(MU const fs::path& gameDataPath, MU Data& inflatedData, MU Data& deflatedData) const override {
            return NOT_IMPLEMENTED;
        }


    };


}
