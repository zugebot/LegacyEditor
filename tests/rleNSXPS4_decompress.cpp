#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"
#include "common/rle/rle_nsxps4.hpp"


std::string EXE_CURRENT_PATH;

int main(int argc, char* argv[]) {
    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();

    std::vector<fs::path> files;

    std::cout << "Scanning directory: " << fs::current_path() << "\n";

    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        std::cout << "Trying " << entry << "\n";

        fs::path file = entry.path().filename();
        if (file.string().starts_with("decompressed")) {
            continue;
        }
        if (file.extension() == ".exe") {
            continue;
        }

        if (file.string() == "regionIn.dat") {
            continue;
        }

        std::cout << "Reading: " << entry.path() << "\n";
        files.emplace_back(entry.path());
    }

    for (auto& file : files) {
        std::string name = file.filename().string();

        Buffer buffer = DataReader::readFile(file);

        Buffer backup(buffer.size());
        memcpy(backup.data(), buffer.data(), buffer.size());

        buffer = std::move(codec::RLE_NSXPS4_DECOMPRESS(buffer));

        buffer = std::move(codec::RLE_NSXPS4_COMPRESS(buffer));
        // if (backup != buffer) {
        //     std::cerr << "COMPRESS and DECOMPRESS do not work." << std::endl;
        //     DataWriter::writeFile(name + "_wrong", buffer.span());
        //     std::cin.get();
        //     return 1;
        // }

        // buffer = std::move(codec::RLE_NSXPS4_DECOMPRESS(buffer));
        // buffer = std::move(codec::RLE_NSXPS4_COMPRESS(buffer));
        // buffer = std::move(codec::RLE_NSXPS4_DECOMPRESS(buffer));

        fs::path out = file.parent_path() / ("decompressed_" + name);
        DataWriter::writeFile(out, buffer.span());
    }

    return 0;
}
