#include "ConsoleParser.hpp"

#include <cstdio>
#include "LegacyEditor/utils/RLEVITA/rlevita.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"

#include "LegacyEditor/LCE/BinFile/BINSupport.hpp"


int ConsoleParser::readConsoleFile(const std::string& inFileStr) {
    const char* inFileCStr = inFileStr.c_str();
    f_in = fopen(inFileCStr, "rb");
    if (f_in == nullptr) {
        printf("Cannot open infile %s", inFileCStr);
        return -1;
    }

    fseek(f_in, 0, SEEK_END);
    source_binary_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);
    fread(&headerUnion, 1, 12, f_in);

    int result;
    // std::cout << headerUnion.getInt1() << std::endl;
    // std::cout << headerUnion.getInt2() << std::endl;

    if (headerUnion.getInt1() <= 2) {
        u32 file_size = headerUnion.getDestSize();
        u32 indexFromSaveFile;
        /// if (int1 == 0) it is a WiiU savefile unless it's a massive file
        if (headerUnion.getZlibMagic() == ZLIB_MAGIC) {
            result = readWiiU(file_size);
            /// idk utter coded it
        } else if (indexFromSaveFile = headerUnion.getVitaFileSize() - headerUnion.getVitaFileListing(),
                indexFromSaveFile > 0 && indexFromSaveFile < 65536) {
            std::cout << indexFromSaveFile << std::endl;
            result = readVita();
        } else {
            result = readPs3(file_size);
        }
    } else if (headerUnion.getInt2() <= 2) {
        /// if (int2 == 0) it is an xbox savefile unless it's a massive
        /// file, but there won't be 2 files in a savegame file for PS3
        result = readXbox360_DAT();
    } else if (headerUnion.getInt2() < 100) {
        /// otherwise if (int2) > 50 then it is a random file
        /// because likely ps3 won't have more than 50 files
        result = readRpcs3();
    } else if (headerUnion.getInt1() == CON_MAGIC) {
        result = readXbox360_BIN();
    } else {
        printf("%s", error3);
        result = -1;
    }

    fclose(f_in);
    return result;
}



/**
 * 0- 3 bytes: 00 00 00 00
 * 4- 7 bytes: file size
 * 8-11 bytes: file listing?
 * @return
 */
int ConsoleParser::readVita() {
    printf("Detected Vita savefile, converting\n");
    console = CONSOLE::VITA;

    // total size of file
    source_binary_size -= 8;

    size = headerUnion.getVitaFileSize();

    // allocate memory
    Data src(source_binary_size);
    allocate(size);

    // goto offset 8 for the data, read data into src
    fseek(f_in, 8, SEEK_SET);
    fread(src.data, 1, source_binary_size, f_in);

    RLEVITA_DECOMPRESS(src.data, src.size, data, size);

    src.deallocate();


    return 0;
}


int ConsoleParser::readWiiU(u32 file_size) {
    printf("Detected WiiU savefile, converting\n");
    console = CONSOLE::WIIU;

    // total size of file
    source_binary_size -= 8;

    Data src = Data(source_binary_size);

    allocate(file_size);

    fseek(f_in, 8, SEEK_SET);
    fread(src.start(), 1, source_binary_size, f_in);

    tinf_zlib_uncompress((Bytef*) start(), &size, (Bytef*) src.start(), source_binary_size);
    src.deallocate();

    if (getSize() == 0) return printf_err("%s", error3);


    return 0;
}


/// ps3 save files don't need decompressing\n
/// TODO: IMPORTANT check from a region file chunk what console it is if it is uncompressed
int ConsoleParser::readPs3(u32 dest_size) {
    printf("Detected compressed PS3 savefile, converting\n");
    console = CONSOLE::PS3;

    // source
    Data src = Data(source_binary_size);

    // destination
    allocate(dest_size);

    // decompress src -> data
    fseek(f_in, 12, SEEK_SET);
    src.size -= 12;
    fread(src.start(), 1, src.size, f_in);
    tinf_uncompress(start(), &dest_size, src.start(), src.getSize());
    src.deallocate();

    if (dest_size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleParser::readRpcs3() {
    printf("Detected uncompressed PS3 / RPCS3 savefile, converting\n");
    console = CONSOLE::RPCS3;
    allocate(source_binary_size);
    fseek(f_in, 0, SEEK_SET);
    fread(start(), 1, size, f_in);
    return 0;
}


int ConsoleParser::readXbox360_DAT() {
    printf("Detected Xbox360 .dat savefile, converting\n");
    console = CONSOLE::XBOX360;

    // allocate source memory
    Data src(headerUnion.getSrcSize() - 8);

    // allocate destination memory
    allocate(headerUnion.getFileSize());

    // decompress src -> data
    fread(src.start(), 1, src.size, f_in);
    size = XDecompress(start(), &size, src.start(), src.getSize());
    src.deallocate();

    if (size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleParser::readXbox360_BIN() {
    console = CONSOLE::XBOX360;
    printf("Detected Xbox360 .bin savefile, converting\n");

    fseek(f_in, 0, SEEK_SET);


    Data bin(source_binary_size);
    fread(bin.start(), 1, source_binary_size, f_in);

    saveGameInfo = extractSaveGameDat(bin.start(), (i64) source_binary_size);
    bin.deallocate(); // TODO: idk if it should but it is for now

    u32 src_size = saveGameInfo.saveFileData.readInt32() - 8;

    size = saveGameInfo.saveFileData.readInt64(); // at offset 8
    allocate(size);
    size = XDecompress(start(), &size, saveGameInfo.saveFileData.data, src_size);
    return 0;
}

