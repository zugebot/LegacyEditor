#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/include.hpp"
#include "common/timer.hpp"

#include <algorithm>


void consumeEnter() {
    std::cout << "[>] Press ENTER to exit...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}


static auto parseLabel(std::string_view s) -> std::pair<int, int> {
    int cat = 2;    // default = “other”
    size_t pos = 0; // position where the digits start

    if (s.starts_with("preTU")) {
        cat = 0;
        pos = 5; // skip "preTU"
    } else if (s.starts_with("TU")) {
        cat = 1;
        pos = 2; // skip "TU"
    }

    int num = 0;
    while (pos < s.size() && std::isdigit(s[pos]))
        num = num * 10 + (s[pos++] - '0');

    return {cat, num};
}

using editor::chunk::eBlockOrder;

template<typename T>
inline u8 get(eBlockOrder O, const std::vector<T>& a, int x, int y, int z) {
    return a[toIndex(O, x, y, z)];
}


struct Score {
    std::size_t solidsBelow64;
    std::size_t airAbove192;
};


/* score YXZ vs YZX ------------------------------------------------------- */
template<typename T, int bits = 8>
eBlockOrder disambiguateXZ(const std::vector<T>& blocks,
                           const std::vector<u8>& heightMap) {
    auto score = [&](eBlockOrder ord) {
        int ok = 0;
        for (int z = 0; z < 16; ++z)
            for (int x = 0; x < 16; ++x) {
                int h = heightMap[x + z * 16];
                if (h < 16 && get(ord, blocks, x, h, z) != 0)
                    ++ok;
            }
        return ok;
    };

    int sYXZ = score(eBlockOrder::YXZ);
    int sYZX = score(eBlockOrder::YZX);

    return (sYXZ >= sYZX) ? eBlockOrder::YXZ : eBlockOrder::YZX; // pick the better match
}

template<typename T, int bits, eBlockOrder Order>
Score scoreChunk(const std::vector<T>& a) {
    const bool isHalf = (Order == eBlockOrder::yXZ); // only 0-127 encoded
    const int YMax = isHalf ? 128 : 256;

    Score s{0, 0};

    for (int z = 0; z < 16; ++z)
        for (int x = 0; x < 16; ++x) {
            // bottom 64 layers
            for (int y = 0; y < 64 && y < YMax; ++y)
                if (get(Order, a, x, y, z) != 0) ++s.solidsBelow64;

            // top quarter: y ∈ [192,255]   (skip if half-height format)
            if (!isHalf)
                for (int y = 192; y < 256; ++y)
                    if (get(Order, a, x, y, z) == 0) ++s.airAbove192;
        }

    return s;
}

template<typename T, int Bits = 8>
eBlockOrder guessOrder(const std::vector<T>& data, const std::vector<u8>& heightMap) {
    struct Candidate {
        eBlockOrder o;
        Score s;
    };
    std::vector<Candidate> cand = {{
            {eBlockOrder::XZY, scoreChunk<T, Bits, eBlockOrder::XZY>(data)},
            {eBlockOrder::YXZ, scoreChunk<T, Bits, eBlockOrder::YXZ>(data)},
            {eBlockOrder::YZX, scoreChunk<T, Bits, eBlockOrder::YZX>(data)},
    }};
    if (data.size() == 32768 / Bits * 8) // Y=128
        cand.emplace_back(eBlockOrder::yXZ, scoreChunk<T, Bits, eBlockOrder::yXZ>(data));
    if (data.size() == 65536 / Bits * 8) // Y=256
        cand.emplace_back(eBlockOrder::yXZy, scoreChunk<T, Bits, eBlockOrder::yXZy>(data));

    // Pick the candidate with the best (solidsBelow64 + airAbove192)
    auto order = std::max_element(cand.begin(), cand.end(),
              [](const Candidate& a, const Candidate& b) {
                  return (a.s.solidsBelow64 + a.s.airAbove192) < (b.s.solidsBelow64 + b.s.airAbove192);
              })
     ->o;

    if (order == eBlockOrder::Yqq) {
        order = disambiguateXZ<T, Bits>(data, heightMap);
    }

    return order;
}


int main(MU int argc, char* argv[]) {


    // Ensure the "out" directory exists
    fs::path dirMain(argv[0]);
    fs::path inDir = "E:\\Xbox360\\Minecraft-Xbox360-Worlds";
    fs::path outDir = R"(C:\Users\jerrin\CLionProjects\LegacyEditor\build\versions)";


    std::vector<std::tuple<std::string, fs::path, fs::path>> versionFiles;

    for (c_auto& folder: fs::directory_iterator(inDir)) {
        if (!is_directory(folder)) { continue; }

        std::string name = folder.path().filename().string();
        std::string tuLabel;
        if (auto pos = name.find(' '); pos != std::string::npos)
            tuLabel = name.substr(0, pos);
        else
            tuLabel = name;


        for (c_auto& file: fs::directory_iterator(folder)) {
            if (file.path().string().ends_with(".bin")) {
                auto thumbnail = file.path() / "__thumbnail.png";
                auto savegame = file.path() / "savegame.dat";
                versionFiles.emplace_back(tuLabel, thumbnail, savegame);
            }
        }
    }

    std::sort(versionFiles.begin(), versionFiles.end(),
              [](const auto& a, const auto& b) { return parseLabel(std::get<0>(a)) < parseLabel(std::get<0>(b)); });


    for (auto [tuLabel, thumbnail, savegame]: versionFiles) {
        editor::SaveProject saveProject;
        if (saveProject.read(savegame.string()) != 0) {
            std::cerr << "Failed to load file: " << savegame.make_preferred() << "\n";
            continue;
        } else {
            std::cout << tuLabel << ": " /*<< savegame.make_preferred()*/ << "\n";

            // 1. fileListing.myReadSettings.m_oldestVersion
            int oldestVersion = saveProject.m_fileListing.oldestVersion();
            std::cout << "fileListing.OldestVersion : " << oldestVersion << "\n";
            // 2. fileListing.myReadSettings.m_currentVersion
            int currentVersion = saveProject.m_fileListing.currentVersion();
            std::cout << "fileListing.CurrentVersion: " << currentVersion << "\n";

            // 3. get valid regionManager and chunkManager
            editor::ChunkManager* chunkManager = nullptr;
            editor::Region regionManager;
            for (auto regionFile: saveProject.m_fileListing.ptrs.old_reg_overworld) {
                regionManager.read(regionFile);
                chunkManager = regionManager.getNonEmptyChunk();
                if (chunkManager != nullptr) {
                    break;
                }
            }
            if (chunkManager == nullptr) {
                std::cout << "Skipping TU" << tuLabel << ", no valid chunk found\n";
                continue;
            }

            chunkManager->ensureDecompress(lce::CONSOLE::XBOX360);
            chunkManager->readChunk(lce::CONSOLE::XBOX360);
            editor::chunk::ChunkData* chunkData = chunkManager->chunkData;

            // 3.a ChunkManager.anon.version
            int chunkVersion = chunkData->lastVersion;
            std::cout << "ChunkManager Version: " << chunkVersion << "\n";

            // 3. ChunkManager.anon.isCompressedZip
            // std::cout << "ChunkManager Version: " << chunkManager->chunkHeader.isZipCompressed() << "\n";

            // 4. ChunkManager.anon.isCompressedRLE
            // std::cout << "ChunkManager isCompressedRLE: " << chunkManager->chunkHeader.isRLECompressed() << "\n";

            // 5. ChunkManager.anon.newSaveFlag
            // std::cout << "ChunkManager newSaveFlag: " << chunkManager->chunkHeader.getNewSaveFlag() << "\n";

            std::cout << "ChunkData chunkHeight: " << chunkData->chunkHeight << "\n";

            eBlockOrder blockOrder;
            if (chunkVersion <= 11) {
                blockOrder = guessOrder<u8, 8>(chunkData->oldBlocks, chunkData->heightMap);
                std::cout << "Chunkdata oldBlocks eBlockOrder: " << toString(blockOrder) << "\n";
            } else {
                blockOrder = guessOrder<u16, 16>(chunkData->newBlocks, chunkData->heightMap);
                std::cout << "Chunkdata newBlocks eBlockOrder: " << toString(blockOrder) << "\n";
            }






            if (currentVersion <= 6) {
                std::cout << "ChunkData Keys: \n";
                for (const auto& key: chunkData->oldNBTData.getTag("Level")->getKeySet()) {
                    std::cout << key << ", ";
                }
                std::cout << "\n";
            }

            // 6. ChunkData->lastVersion

            // 7. (For NBT Chunks) list of all nbt string keys

            // 8. chunk height

            // 9. XYZ format of blocks

            // 10. XYZ format of block data

            // 11. XYZ format of block/sky light

            // 12. whether or not there is biomes in chunkdata
        }
    }


    // print save details, dump to folder
    // fileListing.printDetails();
    // MU int stat = fileListing.dumpToFolder("");


    return 0;
}