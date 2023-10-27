#include <cstdarg>
#include <cstdio>

#include "ConsoleParser.hpp"

#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include <LegacyEditor/utils/zlib-1.2.12/zlib.h>


static int inline printf_err(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return -1;
}


int ConsoleFileParser::loadWiiU(u32 file_size) {
    printf("Detected WiiU savefile, converting\n");
    console = CONSOLE::WIIU;
    bool status;

    // total size of file
    source_binary_size -= 8;

    Data src = Data();
    status = src.allocate(source_binary_size);
    if (!status) return printf_err(error2, source_binary_size);

    status = allocate(file_size);
    if (!status) return printf_err(error2, file_size);

    // goto offset 8 for zlib, readBytes data into src
    fseek(f_in, 8, SEEK_SET);
    fread(src.getStartPtr(), 1, source_binary_size, f_in);

    // decompress src -> data
    tinf_zlib_uncompress((Bytef*) getStartPtr(), &size, (Bytef*) src.getStartPtr(), source_binary_size);

    if (getSize() == 0) return printf_err("%s", error3);

    return 0;
}


/// ps3 save files don't need decompressing\n
/// TODO: IMPORTANT check from a region file chunk what console it is if it is uncompressed
int ConsoleFileParser::loadPs3Compressed(u32 dest_size) {
    printf("Detected compressed PS3 savefile, converting\n");
    console = CONSOLE::PS3;
    int status;

    // source
    Data src = Data();
    status = src.allocate(source_binary_size);
    if (status == false) return printf_err(error2, size);

    // destination
    status = allocate(dest_size);
    if (status == false) return printf_err(error2, dest_size);

    // decompress src -> data
    fseek(f_in, 12, SEEK_SET);
    src.size -= 12;
    fread(src.getStartPtr(), 1, src.size, f_in);
    tinf_uncompress(getStartPtr(), &dest_size, src.getStartPtr(), src.size);

    if (dest_size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleFileParser::loadPs3Uncompressed() {
    printf("Detected uncompressed PS3 savefile, converting\n");
    console = CONSOLE::PS3;

    // allocate memory
    int status = allocate(source_binary_size);
    if (status == false) return printf_err(error1, size);

    // readBytes into data
    fseek(f_in, 0, SEEK_SET);
    fread(getStartPtr(), 1, size, f_in);
    return 0;
}


int ConsoleFileParser::loadXbox360_DAT() {
    printf("Detected Xbox360 .dat savefile, converting\n");
    console = CONSOLE::XBOX360;

    // allocate source memory
    Data src;
    int status = src.allocate(headerUnion.getSrcSize() - 8);
    if (status == false) return printf_err(error2, src.size);

    // allocate destination memory
    status = allocate(headerUnion.getFileSize());
    if (status == false) return printf_err(error2, size);

    // decompress src -> data
    fread(src.getStartPtr(), 1, src.size, f_in);
    size = XDecompress(getStartPtr(), &size, src.getStartPtr(), src.size);

    if (size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleFileParser::loadXbox360_BIN() {
    console = CONSOLE::XBOX360;
    printf("Detected Xbox360 .bin savefile, converting\n");

    fseek(f_in, 0, SEEK_SET);


    Data  bin;
    int status = bin.allocate(source_binary_size);
    if (status == false) return printf_err(error1, bin.size);
    fread(bin.getStartPtr(), 1, source_binary_size, f_in);

    saveGameInfo = extractSaveGameDat(bin.getStartPtr(), (i64) source_binary_size);

    u32 src_size = saveGameInfo.saveFileData.readInt() - 8;

    size = saveGameInfo.saveFileData.readLong(); // at offset 8
    status = allocate(size);
    if (status == false) return printf_err(error2, size);
    size = XDecompress(getStartPtr(), &size, saveGameInfo.saveFileData.getStartPtr(), src_size);
    return 0;
}


int ConsoleFileParser::loadConsoleFile(const char* infileStr) {

    // open file
    f_in = fopen(infileStr, "rb");
    if (f_in == nullptr) {
        printf("Cannot open infile %s", infileStr);
        return -1;
    }

    fseek(f_in, 0, SEEK_END);
    source_binary_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);
    fread(&headerUnion, 1, 12, f_in);

    int result;

    std::cout << headerUnion.getInt1() << std::endl;
    std::cout << headerUnion.getInt2() << std::endl;

    if (headerUnion.getInt1() <= 2) {
        u32 file_size = headerUnion.getDestSize();
        /// if (int1 == 0) it is a WiiU savefile unless it's a massive file
        if (headerUnion.getZlibMagic() == ZLIB_MAGIC) {
            result = loadWiiU(file_size);
        } else {
            result = loadPs3Compressed(file_size);
        }
    } else if (headerUnion.getInt2() <= 2) {
        /// if (int2 == 0) it is an xbox savefile unless it's a massive
        /// file, but there won't be 2 files in a savegame file for PS3
        result = loadXbox360_DAT();
    } else if (headerUnion.getInt2() < 100) {
        /// otherwise if (int2) > 50 then it is a random file
        /// because likely ps3 won't have more than 50 files
        result = loadPs3Uncompressed();
    } else if (headerUnion.getInt1() == CON_MAGIC) {
        result = loadXbox360_BIN();
    } else {
        printf("%s", error3);
        result = -1;
    }

    fclose(f_in);
    return result;
}

int ConsoleFileParser::saveWiiU(const std::string& outfileStr, DataOutManager& outManager) {
    u64 src_size = outManager.size;

    FILE* f_out = fopen(outfileStr.c_str(), "wb");
    if (!f_out) { return -1; }

    // Write src_size to the file
    uLong compressedSize = compressBound(src_size);
    printf("compressed bound: %lu\n", compressedSize);

    std::vector<uint8_t> compressedData(compressedSize);
    if (compress(compressedData.data(), &compressedSize, outManager.getStartPtr(), outManager.size) != Z_OK) {
        return {};
    }
    compressedData.resize(compressedSize);

    int num = 1;
    bool isLittleEndian = *(char*) &num == 1;
    if (isLittleEndian) {
        src_size = swapEndian64(src_size);
    }

    fwrite(&src_size, sizeof(uint64_t), 1, f_out);
    printf("Writing final size: %zu\n", compressedData.size());

    fwrite(compressedData.data(), 1, compressedData.size(), f_out);

    fclose(f_out);


    return 0;
}


int ConsoleFileParser::savePS3Uncompressed() { return 0; }


int ConsoleFileParser::savePS3Compressed() { return 0; }


int ConsoleFileParser::saveXbox360_DAT() { return 0; }


int ConsoleFileParser::saveXbox360_BIN() { return 0; }


int ConsoleFileParser::saveConsoleFile(const char* outfileStr) { return 0; }
