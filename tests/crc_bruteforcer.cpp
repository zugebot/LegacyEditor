
#include <iostream>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "common/windows/force_utf8.hpp"
#include "common/timer.hpp"

#include "code/include.hpp"
#include "png/crc.hpp"

int main() {

    fs::path root = "C:\\Users\\jerrin\\AppData\\Roaming\\yuzu\\nand\\user\\save\\0000000000000000\\D5FA36C00E9BAF9DB378592F82847B9E\\01006BD001E06000";

    fs::path in = root / "250623165128.ext";
    fs::path out = root / "250623165128.ext2";

    editor::DisplayMetadata meta;
    Buffer buf = DataReader::readFile(in);
    meta.read(buf, lce::CONSOLE::SWITCH);

    Buffer dat = meta.write(lce::CONSOLE::SWITCH);
    DataWriter::writeFile(out, dat.span());


    return 0;

    std::cout << "\n";

    fs::path ext = R"(C:\Users\jerrin\AppData\Roaming\yuzu\nand\user\save\0000000000000000\D5FA36C00E9BAF9DB378592F82847B9E\01006BD001E06000\250621185728.ext)";

    static constexpr u32 CORRECT_CRC1 = 0x2B'63'57'68;
    std::cout << (CORRECT_CRC1 & 0xFF'FF'00'00) << "\n";

    Buffer buffer = DataReader::readFile(ext);
    DataReader reader(buffer);

    std::cout << "File Size: " << reader.size() << "\n";

    u32 size = reader.size();
    for (u32 i = 0; i < size; i++) {
        for (u32 len = 1; len <= size - i; len++) {
            u32 ret = crc(reader.data() + i, len);
            if (ret == (CORRECT_CRC1 & 0xFF'FF'00'00)) {
                std::cout << "start=" << i << " len=" << len << "\n";
                // goto FINISH;
            }
        }
    }
    std::cout << "checksum not found\n";
    return -1;
FINISH:
    return 0;
}