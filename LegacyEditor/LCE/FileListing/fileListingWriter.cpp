#include "fileListing.hpp"

#include "../../utils/RLE/rle_vita.hpp"
#include "LegacyEditor/libs/zlib-1.2.12/zlib.h"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/endian.hpp"
// #include "LegacyEditor/utils/LZX/XboxCompression.hpp"
// #include "LegacyEditor/utils/tinf/tinf.h"

#include <filesystem>
#include <cstdio>






namespace editor {
    namespace fs = std::filesystem;

    Data FileListing::writeData(const CONSOLE consoleOut) {

        ensureAllRegionFilesExist();

        // step 1: get the file count and size of all sub-files
        u32 fileCount = 0;
        u32 fileDataSize = 0;
        for (const File& file: allFiles) {
            fileCount++;
            fileDataSize += file.data.getSize();
        }
        const u32 fileInfoOffset = fileDataSize + 12;

        // step 2: find total binary size and create its data buffer
        const u32 totalFileSize = fileInfoOffset + FILE_HEADER_SIZE * fileCount;

        const Data dataOut(totalFileSize);
        DataManager managerOut(dataOut, consoleIsBigEndian(consoleOut));

        // step 3: write start
        managerOut.writeInt32(fileInfoOffset);
        managerOut.writeInt32(fileCount);
        managerOut.writeInt16(oldestVersion);
        managerOut.writeInt16(currentVersion);

        // step 5: write each files data
        // I am using additionalData as the offset into the file its data is at
        u32 index = FILELISTING_HEADER_SIZE;
        for (File& fileIter : allFiles) {
            fileIter.additionalData = index;
            index += fileIter.data.getSize();
            managerOut.writeFile(fileIter);
        }

        // step 6: write file metadata
        for (const File& fileIter: allFiles) {
            // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.additionalData, fileIter.size, fileIter.name.c_str());
            std::string fileIterName = fileIter.constructFileName(consoleOut, separateRegions);
            managerOut.writeWStringFromString(fileIterName, WSTRING_SIZE);
            managerOut.writeInt32(fileIter.data.getSize());
            managerOut.writeInt32(fileIter.additionalData);
            managerOut.writeInt64(fileIter.timestamp);
        }

        return dataOut;
    }


    int FileListing::write(stringRef_t outfileStr, const CONSOLE consoleOut) {
        if (outfileStr.empty()) {
            return INVALID_ARGUMENT;
        }

        const int status = writeFile(outfileStr, consoleOut);

        if (fileInfo.isLoaded) {
            MU const int status2 = writeFileInfo(outfileStr, consoleOut);
        }


        return status;
    }


    int FileListing::writeFile(stringRef_t outfileStr, const CONSOLE consoleOut) {
        const Data dataOut = writeData(consoleOut);
        int status;
        switch (consoleOut) {
            case CONSOLE::PS3:
                status = writePS3();
                break;
            case CONSOLE::RPCS3:
                status = writeRPCS3(outfileStr, dataOut);
                break;
            case CONSOLE::XBOX360:
                status = writeXbox360_BIN();
                break;
            case CONSOLE::WIIU:
                status = writeWiiU(outfileStr, dataOut);
                break;
            case CONSOLE::VITA:
                status = writeVita(outfileStr, dataOut);
                break;
            case CONSOLE::SWITCH:
                status = writeNSX();
                break;
            case CONSOLE::PS4:
                status = writePs4();
                break;
            default:
                status = INVALID_CONSOLE;
                break;
        }
        return status;
    }


    int FileListing::writeFileInfo(stringRef_t outFilePath, const CONSOLE consoleOut) const {
        std::string filepath = outFilePath;
        while (!(filepath.back() == '\\' || filepath.back() == '/')) {
            filepath.pop_back();
        }

        switch (consoleOut) {
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
            case CONSOLE::PS4:
                filepath += "THUMB";
                break;
            case CONSOLE::VITA:
                filepath += "THUMBDATA.BIN";
                break;
            case CONSOLE::WIIU:
            case CONSOLE::SWITCH: {
                filepath = outFilePath + ".ext";
                break;
            }
            case CONSOLE::XBOX360:
                return NOT_IMPLEMENTED;
            case CONSOLE::NONE:
            default:
                return INVALID_CONSOLE;
        }


        const int status = fileInfo.writeFile(filepath, consoleOut);
        return status;
    }


    int FileListing::writeExternalRegions(MU stringRef_t outFilePath) {
        return NOT_IMPLEMENTED;
    }


    int FileListing::writeWiiU(stringRef_t outfileStr, const Data& dataOut) {
        const DataManager managerOut(dataOut);
        u64 src_size = managerOut.size;

        FILE* f_out = fopen(outfileStr.c_str(), "wb");
        if (f_out == nullptr) { return FILE_ERROR; }

        // Write src_size to the file
        uLong compressedSize = compressBound(src_size);
        printf("compressed bound: %lu\n", compressedSize);

        u8_vec compressedData(compressedSize);
        if (compress(compressedData.data(), &compressedSize,
                     managerOut.data, managerOut.size) != Z_OK) {
            return COMPRESS;
        }
        compressedData.resize(compressedSize);

        if (isSystemLittleEndian()) {
            src_size = swapEndian64(src_size);
        }

        fwrite(&src_size, sizeof(u64), 1, f_out);
        printf("Writing final size: %zu\n", compressedData.size());

        fwrite(compressedData.data(), 1, compressedData.size(), f_out);

        fclose(f_out);

        return SUCCESS;
    }


    int FileListing::writeVita(stringRef_t outfileStr, const Data& dataOut) {
        FILE* f_out = fopen(outfileStr.c_str(), "wb");
        if (f_out == nullptr) { return FILE_ERROR; }

        Data self;
        self.allocate(dataOut.size + 2);

        self.size = RLEVITA_COMPRESS(dataOut.data, dataOut.size, self.data, self.size);

        constexpr int num = 0;
        fwrite(&num, sizeof(u32), 1, f_out);

        u32 val;
        memcpy(&val, &self.data[0], 4);
        static constexpr u32 NUM = 0x0900;
        val += NUM;

        // might need to swap endianness
        fwrite(&val, sizeof(u32), 1, f_out);
        fwrite(self.data, sizeof(u8), self.size, f_out);
        fclose(f_out);

        return SUCCESS;
    }


    MU int FileListing::writeRPCS3(stringRef_t outfileStr, const Data& dataOut) {
        FILE* f_out = fopen(outfileStr.c_str(), "wb");
        if (f_out == nullptr) { return FILE_ERROR; }


        printf("Writing final size: %u\n", dataOut.size);
        fwrite(dataOut.data, 1, dataOut.size, f_out);
        fclose(f_out);
        return SUCCESS;
    }


    MU int FileListing::writePS3() {
        return NOT_IMPLEMENTED;
    }


    MU int FileListing::writeXbox360_DAT() {
        return NOT_IMPLEMENTED;
    }


    MU int FileListing::writeXbox360_BIN() {
        return NOT_IMPLEMENTED;
    }

    MU int FileListing::writeNSX() {
        return NOT_IMPLEMENTED;
    }

    MU int FileListing::writePs4() {
        return NOT_IMPLEMENTED;
    }
}
