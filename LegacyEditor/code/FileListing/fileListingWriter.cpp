#include "fileListing.hpp"

#include "lce/processor.hpp"

#include <cstdio>

#include "include/ghc/fs_std.hpp"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/utils/RLE/rle_vita.hpp"
#include "LegacyEditor/utils/endian.hpp"

// #include "LegacyEditor/utils/LZX/XboxCompression.hpp"
// #include "LegacyEditor/utils/tinf/tinf.h"



namespace editor {


    int FileListing::write(const fs::path& outFilePath,
                           const lce::CONSOLE consoleOut) {
        if (outFilePath.empty()) {
            printf("FileListing::write: filename is empty!");
            return INVALID_ARGUMENT;
        }

        c_bool differentConsole = myConsole != consoleOut;

        if (AUTO_REMOVE_PLAYERS && differentConsole) {
            removeFileTypes({editor::LCEFileType::PLAYER});
        }

        if (AUTO_REMOVE_DATA_MAPPING && differentConsole) {
            removeFileTypes({editor::LCEFileType::DATA_MAPPING});
        }

        convertRegions(consoleOut);

        c_int status = writeFile(outFilePath, consoleOut);

        if (fileInfo.isLoaded) {
            MU c_int status2 = writeFileInfo(outFilePath, consoleOut);
        }

        return status;
    }



    Data FileListing::writeData(const lce::CONSOLE consoleOut) {

        // step 1: get the file count and size of all sub-files
        u32 fileCount = 0;
        u32 fileDataSize = 0;
        for (const LCEFile& file: myAllFiles) {
            fileCount++;
            fileDataSize += file.data.getSize();
        }
        c_u32 fileInfoOffset = fileDataSize + 12;

        // step 2: find total binary size and create its data buffer
        c_u32 totalFileSize = fileInfoOffset + FILE_HEADER_SIZE * fileCount;

        const Data dataOut(totalFileSize);
        DataManager managerOut(dataOut, consoleIsBigEndian(consoleOut));

        // step 3: write start
        managerOut.writeInt32(fileInfoOffset);
        managerOut.writeInt32(fileCount);
        managerOut.writeInt16(myOldestVersion);
        managerOut.writeInt16(myCurrentVersion);

        // step 5: write each files data
        // I am using additionalData as the offset into the file its data is at
        u32 index = FILELISTING_HEADER_SIZE;
        for (LCEFile& fileIter : myAllFiles) {
            fileIter.additionalData = index;
            index += fileIter.data.getSize();
            managerOut.writeFile(fileIter);
        }

        // step 6: write file metadata
        for (const LCEFile& fileIter: myAllFiles) {
            // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter
            // .additionalData, fileIter.size, fileIter.name.c_str());
            std::string fileIterName = fileIter.constructFileName(consoleOut, myHasSeparateRegions);
            managerOut.writeWStringFromString(fileIterName, WSTRING_SIZE);
            managerOut.writeInt32(fileIter.data.getSize());
            managerOut.writeInt32(fileIter.additionalData);
            managerOut.writeInt64(fileIter.timestamp);
        }

        return dataOut;
    }


    int FileListing::writeFile(const fs::path& outFilePath,
                               const lce::CONSOLE consoleOut) {
        const Data dataOut = writeData(consoleOut);
        FILE* f_out;

        {
            std::string outFileStr = outFilePath.string();
            f_out = fopen(outFileStr.c_str(), "wb");
            if (f_out == nullptr) {
                printf("cannot create file \"%s\"", outFileStr.c_str());
                return FILE_ERROR;
            }
        }

        int status;
        switch (consoleOut) {
            case lce::CONSOLE::PS3:
                status = writePS3();
                break;
            case lce::CONSOLE::RPCS3:
                status = writeRPCS3(f_out, dataOut);
                break;
            case lce::CONSOLE::XBOX360:
                status = writeXbox360_BIN();
                break;
            case lce::CONSOLE::WIIU:
                status = writeWiiU(f_out, dataOut);
                break;
            case lce::CONSOLE::VITA:
                status = writeVita(f_out, dataOut);
                break;
            case lce::CONSOLE::SWITCH:
                status = writeNSX();
                break;
            case lce::CONSOLE::PS4:
                status = writePs4();
                break;
            default:
                status = INVALID_CONSOLE;
                break;
        }
        return status;
    }


    int FileListing::writeFileInfo(const fs::path& outFilePath,
                                   const lce::CONSOLE consoleOut) const {
        fs::path resultFilePath = outFilePath.parent_path();


        switch (consoleOut) {
            case lce::CONSOLE::PS3:
            case lce::CONSOLE::RPCS3:
            case lce::CONSOLE::PS4:
                resultFilePath /= "THUMB";
                break;
            case lce::CONSOLE::VITA:
                resultFilePath /= "THUMBDATA.BIN";
                break;
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::SWITCH: {
                resultFilePath = outFilePath;
                resultFilePath += ".ext";
                break;
            }
            case lce::CONSOLE::XBOX360:
                printf("FileListing::writeFileInfo: not implemented! (xbox360)");
                return NOT_IMPLEMENTED;
            case lce::CONSOLE::NONE:
            default:
                return INVALID_CONSOLE;
        }

        c_int status = fileInfo.writeFile(resultFilePath, consoleOut);
        return status;
    }





    /**
     * \brief Done.
     * \param f_out
     * \param dataOut
     * \return
     */
    int FileListing::writeWiiU(FILE* f_out, const Data& dataOut) {

        u64 src_size = dataOut.size;
        uLong compressedSize = compressBound(src_size);
        printf("compressed bound: %lu\n", compressedSize);

        u8_vec compressedData(compressedSize);
        if (compress(compressedData.data(), &compressedSize,
                     dataOut.data, dataOut.size) != Z_OK) {
            return COMPRESS;
        }

        compressedData.resize(compressedSize);
        printf("Writing final size: %zu\n", compressedData.size());

        if (isSystemLittleEndian()) {
            src_size = swapEndian64(src_size);
        }

        fwrite(&src_size, sizeof(u64), 1, f_out);
        fwrite(compressedData.data(), 1, compressedData.size(), f_out);
        fclose(f_out);

        return SUCCESS;
    }


    /**
     * \brief Done.
     * \param f_out
     * \param dataOut
     * \return
     */
    int FileListing::writeVita(FILE* f_out, const Data& dataOut) {
        Data self;
        self.allocate(dataOut.size + 2);

        self.size = RLEVITA_COMPRESS(
            dataOut.data, dataOut.size,
            self.data, self.size);

        constexpr int num = 0;
        u32 compSize = dataOut.size;
        if (!isSystemLittleEndian()) {
            compSize = swapEndian32(compSize);
        }

        // 4-bytes of '0'
        // 4-bytes of total decompressed fileListing size
        // N-bytes fileListing data
        fwrite(&num, sizeof(u32), 1, f_out);
        fwrite(&compSize, sizeof(u32), 1, f_out);
        fwrite(self.data, 1, self.size, f_out);

        fclose(f_out);

        return SUCCESS;
    }


    /**
     * \brief Done.
     * \param f_out
     * \param dataOut
     * \return
     */
    MU int FileListing::writeRPCS3(FILE* f_out,
                                   const Data& dataOut) {
        printf("Writing final size: %u\n", dataOut.size);
        fwrite(dataOut.data, 1, dataOut.size, f_out);
        fclose(f_out);
        return SUCCESS;
    }


    MU int FileListing::writePS3() {
        printf("FileListing::writePS3: not implemented!");
        return NOT_IMPLEMENTED;
    }


    MU int FileListing::writeXbox360_DAT() {
        printf("FileListing::writeXbox360_DAT: not implemented!");
        return NOT_IMPLEMENTED;
    }


    MU int FileListing::writeXbox360_BIN() {
        printf("FileListing::writeXbox360_BIN: not implemented!");
        return NOT_IMPLEMENTED;
    }

    MU int FileListing::writeNSX() {
        printf("FileListing::writeNSX: not implemented!");
        return NOT_IMPLEMENTED;
    }

    MU int FileListing::writePs4() {
        printf("FileListing::writePs4: not implemented!");
        return NOT_IMPLEMENTED;
    }
}
