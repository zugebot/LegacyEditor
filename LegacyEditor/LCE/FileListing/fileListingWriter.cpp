#include "fileListing.hpp"

#include <cstdio>

#include "LegacyEditor/utils/RLEVITA/rlevita.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"
// #include "LegacyEditor/utils/LZX/XboxCompression.hpp"
// #include "LegacyEditor/utils/tinf/tinf.h"

#include "LegacyEditor/utils/endian.hpp"


Data FileListing::writeData(CONSOLE consoleOut) {

    // step 1: get the file count and size of all sub-files
    u32 fileCount = 0, fileDataSize = 0;
    for (const File &file: allFiles) {
        fileCount++;
        fileDataSize += file.data.getSize();
    }
    u32 fileInfoOffset = fileDataSize + 12;

    // step 2: find total binary size and create its data buffer
    u32 totalFileSize = fileInfoOffset + FILE_HEADER_SIZE * fileCount;

    Data dataOut(totalFileSize);
    DataManager managerOut(dataOut, consoleIsBigEndian(consoleOut));

    // step 3: write start
    managerOut.writeInt32(fileInfoOffset);
    managerOut.writeInt32(fileCount);
    managerOut.writeInt16(oldestVersion);
    managerOut.writeInt16(currentVersion);


    // step 4: write each files data
    // I am using additionalData as the offset into the file its data is at
    u32 index = 12;
    for (File &fileIter: allFiles) {
        fileIter.additionalData = index;
        index += fileIter.data.getSize();
        managerOut.writeFile(fileIter);
    }

    // step 5: write file metadata
    for (File &fileIter: allFiles) {
        // printf("%2u. (@%7u)[%7u] - %s\n", count + 1, fileIter.additionalData, fileIter.size, fileIter.name.c_str());
        std::string fileIterName = fileIter.constructFileName(consoleOut);
        managerOut.writeWString(fileIterName, 64);
        managerOut.writeInt32(fileIter.data.getSize());
        managerOut.writeInt32(fileIter.additionalData);
        managerOut.writeInt64(fileIter.timestamp);
    }

    return dataOut;
}


MU int FileListing::writeFile(CONSOLE consoleOut, stringRef_t outfileStr) {
    Data dataOut = writeData(consoleOut);
    int status;
    switch (consoleOut) {
        case CONSOLE::PS3:
            status = writePS3Compressed();
            break;
        case CONSOLE::RPCS3:
            status = writePS3Uncompressed();
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
            status = STATUS::INVALID_CONSOLE;
            break;
    }
    return status;
}


int FileListing::writeWiiU(stringRef_t outfileStr, Data& dataOut) {
    DataManager managerOut(dataOut);
    u64 src_size = managerOut.size;

    FILE* f_out = fopen(outfileStr.c_str(), "wb");
    if (!f_out) { return STATUS::FILE_NOT_FOUND; }

    // Write src_size to the file
    uLong compressedSize = compressBound(src_size);
    printf("compressed bound: %lu\n", compressedSize);

    u8_vec compressedData(compressedSize);
    if (compress(compressedData.data(), &compressedSize,
                 managerOut.data, managerOut.size) != Z_OK) {
        return STATUS::COMPRESS;
    }
    compressedData.resize(compressedSize);

    if (isSystemLittleEndian()) {
        src_size = swapEndian64(src_size);
    }

    fwrite(&src_size, sizeof(u64), 1, f_out);
    printf("Writing final size: %zu\n", compressedData.size());

    fwrite(compressedData.data(), 1, compressedData.size(), f_out);

    fclose(f_out);

    return STATUS::SUCCESS;
}


int FileListing::writeVita(stringRef_t outfileStr, Data& dataOut) {
    FILE* f_out = fopen(outfileStr.c_str(), "wb");
    if (!f_out) { return STATUS::FILE_NOT_FOUND; }

    Data self;
    self.allocate(dataOut.size + 2);

    self.size = RLEVITA_COMPRESS(dataOut.data, dataOut.size, self.data, self.size);

    int num = 0;
    fwrite(&num, sizeof(u32), 1, f_out);

    u32 val;
    memcpy(&val, &self.data[0], 4);
    val += 0x0900;

    // might need to swap endianness
    fwrite(&val, sizeof(u32), 1, f_out);
    fwrite(self.data, sizeof(u8), self.size, f_out);
    fclose(f_out);

    return STATUS::SUCCESS;
}


MU int FileListing::writePS3Uncompressed() {
    return STATUS::SUCCESS;
}


MU int FileListing::writePS3Compressed() {
    return STATUS::SUCCESS;
}


MU int FileListing::writeXbox360_DAT() {
    return STATUS::SUCCESS;
}


MU int FileListing::writeXbox360_BIN() {
    return STATUS::SUCCESS;
}