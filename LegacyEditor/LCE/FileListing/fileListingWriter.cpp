#include "fileListing.hpp"

#include <cstdio>

#include "LegacyEditor/utils/RLEVITA/rlevita.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/libs/zlib-1.2.12/zlib.h"
// #include "LegacyEditor/utils/LZX/XboxCompression.hpp"
// #include "LegacyEditor/utils/tinf/tinf.h"

#include "LegacyEditor/utils/endian.hpp"


namespace editor {


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


        // step 4: create the correct file list order
        std::vector<File*> fileOrder;
        {
            std::vector<File*> regions;
            File* levelFile = nullptr;
            std::vector<File*> regionsEmpty;
            std::vector<File*> otherFiles;
            for (File &fileIter: allFiles) {
                if (fileIter.isRegionType()) {
                    if (fileIter.data.size == 0) {
                        regionsEmpty.push_back(&fileIter);
                    } else {
                        regions.push_back(&fileIter);
                    }

                } else if (fileIter.fileType == FileType::LEVEL) {
                    levelFile = &fileIter;
                } else {
                    otherFiles.push_back(&fileIter);
                }
            }

            fileOrder.insert(fileOrder.end(), regions.begin(), regions.end());
            fileOrder.push_back(levelFile);
            fileOrder.insert(fileOrder.end(), regionsEmpty.begin(), regionsEmpty.end());
            fileOrder.insert(fileOrder.end(), otherFiles.begin(), otherFiles.end());
        }

        // step 5: write each files data
        // I am using additionalData as the offset into the file its data is at
        u32 index = FILELISTING_HEADER_SIZE;
        for (File* fileIter: fileOrder) {
            fileIter->additionalData = index;
            index += fileIter->data.getSize();
            managerOut.writeFile(fileIter);
        }

        // step 6: write file metadata
        for (const File* fileIter: fileOrder) {
            // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.additionalData, fileIter.size, fileIter.name.c_str());
            std::string fileIterName = fileIter->constructFileName(consoleOut);
            managerOut.writeWString(fileIterName, WSTRING_SIZE);
            managerOut.writeInt32(fileIter->data.getSize());
            managerOut.writeInt32(fileIter->additionalData);
            managerOut.writeInt64(fileIter->timestamp);
        }

        return dataOut;
    }


    MU int FileListing::writeFile(const CONSOLE consoleOut, stringRef_t outfileStr) {
        if (outfileStr.empty()) {
            return INVALID_ARGUMENT;
        }

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
            default:
                status = INVALID_CONSOLE;
                break;
        }
        return status;
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
        return SUCCESS;
    }


    MU int FileListing::writeXbox360_DAT() {
        return SUCCESS;
    }


    MU int FileListing::writeXbox360_BIN() {
        return SUCCESS;
    }
}
