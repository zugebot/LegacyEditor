#include "fileListing.hpp"

#include "common/nbt.hpp"
#include "common/fmt.hpp"

#include "code/scripts.hpp"



struct Coordinate {
    i32 x, z;

    bool operator==(const Coordinate&) const = default;
};

namespace std {
    template <>
    struct hash<Coordinate> {
        size_t operator()(const Coordinate& rc) const noexcept {
            return std::hash<i32>{}(rc.x) ^ (std::hash<i32>{}(rc.z) << 1);
        }
    };
}



namespace editor {


    FileListing::~FileListing() {
        deallocate();
    }


    int FileListing::readListing(const Buffer & bufferIn, lce::CONSOLE consoleIn) {
        static constexpr u32 WSTRING_SIZE = 64;
        MU static constexpr u32 FILELISTING_HEADER_SIZE = 12;

        DataReader reader(bufferIn.data(), bufferIn.size(), getConsoleEndian(consoleIn));

        c_u32 indexOffset = reader.read<u32>();
        u32 fileCount = reader.read<u32>();
        setOldestVersion(reader.read<u16>());
        setCurrentVersion(reader.read<u16>());

        u32 FOOTER_ENTRY_SIZE = 144;
        if (currentVersion() <= 1) {
            FOOTER_ENTRY_SIZE = 136;
            fileCount /= 136;
        }

        m_allFiles.clear();

        MU u32 totalSize = 0;
        for (u32 fileIndex = 0; fileIndex < fileCount; fileIndex++) {
            reader.seek(indexOffset + fileIndex * FOOTER_ENTRY_SIZE);

            std::string fileName = reader.readWAsString(WSTRING_SIZE);

            u32 fileSize = reader.read<u32>();
            c_u32 index = reader.read<u32>();
            u64 timestamp = 0;
            if (currentVersion() > 1) {
                timestamp = reader.read<u64>();
            }
            totalSize += fileSize;

            reader.seek(index);

            Buffer buffer = reader.readBuffer(fileSize);
            m_allFiles.emplace_back(consoleIn, std::move(buffer), timestamp);
            editor::LCEFile &file = m_allFiles.back();
            file.setFileName(fileName);

            if (fileName.ends_with(".mcr")) {
                if (fileName.starts_with("DIM-1")) {
                    file.setType(lce::FILETYPE::OLD_REGION_NETHER);
                } else if (fileName.starts_with("DIM1")) {
                    file.setType(lce::FILETYPE::OLD_REGION_END);
                } else if (fileName.starts_with("r")) {
                    file.setType(lce::FILETYPE::OLD_REGION_OVERWORLD);
                }
                c_auto [fst, snd] = extractRegionCoords(fileName);
                file.setRegionX(static_cast<i16>(fst));
                file.setRegionZ(static_cast<i16>(snd));

            } else if (fileName == "entities.dat") {
                file.setType(lce::FILETYPE::ENTITY_OVERWORLD);

            } else if (fileName.ends_with("entities.dat")) {
                file.setFileName(fileName);
                if (fileName.starts_with("DIM-1")) {
                    file.setType(lce::FILETYPE::ENTITY_NETHER);
                } else if (fileName.starts_with("DIM1/")) {
                    file.setType(lce::FILETYPE::ENTITY_END);
                }
                file.setFileName(fileName);

            } else if (fileName == "level.dat") {
                file.setType(lce::FILETYPE::LEVEL);

            } else if (fileName.starts_with("data/map_")) {
                c_i16 mapNumber = extractMapNumber(fileName);
                file.setMapNumber(mapNumber);
                file.setType(lce::FILETYPE::MAP);

            }else if (fileName == "data/villages.dat") {
                file.setType(lce::FILETYPE::VILLAGE);

            } else if (fileName == "data/largeMapDataMappings.dat") {
                file.setType(lce::FILETYPE::DATA_MAPPING);

            } else if (fileName.starts_with("data/")) {
                file.setType(lce::FILETYPE::STRUCTURE);

            } else if (fileName.ends_with(".grf")) {
                file.setType(lce::FILETYPE::GRF);

            } else if (fileName.starts_with("players/") ||
                       fileName.find('/') == -1LLU) {
                file.setType(lce::FILETYPE::PLAYER);

            } else {
                printf("Unknown File: %s\n", fileName.c_str());
            }

        }

        return SUCCESS;
    }


    void convertNewGenChunksToOldGen(FileListing* fileListing,
                                     StateSettings& stateSettings,
                                     WriteSettings& writeSettings) {

        std::vector<std::vector<int>> positions = {
                {-1, -1}, {-1, 0}, {0, -1}, {0, 0}
        };

        auto makeSmaller = [](int v) {
            return (v >= 0) ? v / 2 : (v - 1) / 2;
        };

        using ft = lce::FILETYPE;
        using Map = std::unordered_map<Coordinate, editor::RegionManager>;

        std::vector<std::tuple<ft, ft, ft, Map>> dimensions;
        dimensions.reserve(3);
        dimensions.emplace_back(ft::NEW_REGION_OVERWORLD, ft::OLD_REGION_OVERWORLD, ft::ENTITY_OVERWORLD, Map{});
        dimensions.emplace_back(ft::NEW_REGION_NETHER,    ft::OLD_REGION_NETHER,    ft::ENTITY_NETHER,    Map{});
        dimensions.emplace_back(ft::NEW_REGION_END,       ft::OLD_REGION_END,       ft::ENTITY_END,       Map{});


        for (auto& [newFmt, oldFmt, entityFmt, regionMap] : dimensions) {

            // (1) read entities file
            std::list<LCEFile> entityFileList = fileListing->collectFiles(entityFmt);
            std::unordered_map<Coordinate, NBTBase> entityMap;
            if (!entityFileList.empty()) {
                LCEFile& entityFile = entityFileList.front();
                DataReader entityReader(entityFile.m_data.span());
                int entityCount = entityReader.read<i32>();
                for (int i = 0; i < entityCount; i++) {
                    int chunkX = entityReader.read<i32>();
                    int chunkZ = entityReader.read<i32>();
                    NBTBase nbt;
                    entityReader.skip(3);
                    nbt.read(entityReader);
                    // nbt.print();
                    // std::cout << std::flush;
                    entityMap.emplace(Coordinate(chunkX, chunkZ), std::move(nbt));
                }
            }

            // (2) build regions
            for (auto& pos : positions) {
                Coordinate key{pos[0], pos[1]};

                regionMap.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(key),
                        std::forward_as_tuple(key.x, key.z)
                );
            }

            // (3) collect new format
            std::list<LCEFile> regionFiles = fileListing->collectFiles(newFmt);

            // (4) place contents into old regions
            for (auto& regionFile: regionFiles) {
                Coordinate tinyCoord(
                        makeSmaller(regionFile.getRegionX()),
                        makeSmaller(regionFile.getRegionZ())
                );
                if (tinyCoord.x < -1 || tinyCoord.x > 0 || tinyCoord.x < -1 || tinyCoord.x > 0) {
                    continue;
                }

                RegionManager tinyRegion;
                tinyRegion.read(&regionFile);

                auto bigIt = regionMap.find(tinyCoord);
                if (bigIt == regionMap.end()) {
                    continue;
                    // auto [insertedIt, success] = regionMap.emplace(
                    //         std::piecewise_construct,
                    //         std::forward_as_tuple(tinyCoord),
                    //         std::forward_as_tuple(tinyCoord.x, tinyCoord.z)
                    // );
                    // bigIt = insertedIt;
                }

                // (4a) move chunks from tiny regions to big regions
                RegionManager& bigRegion = bigIt->second;
                for (int sx = 0; sx < 16; ++sx) {
                    for (int sz = 0; sz < 16; ++sz) {
                        int dx = sx + 16 * std::abs(tinyRegion.x() & 1);
                        int dz = sz + 16 * std::abs(tinyRegion.z() & 1);

                        int scale = 27;
                        bool onLeft   = (bigRegion.x() == -1);
                        bool onRight  = (bigRegion.x() ==  0);
                        bool onTop    = (bigRegion.z() == -1);
                        bool onBottom = (bigRegion.z() ==  0);
                        int xMin = onLeft  ? 32 - scale : 0;
                        int xMax = onRight ? scale : 32;
                        int zMin = onTop    ? 32 - scale  : 0;
                        int zMax = onBottom ? scale : 32;
                        if (dx < xMin || dx >= xMax || dz < zMin || dz >= zMax) {
                            continue;
                        }



                        if (!inRange(sx, sz, bigRegion.m_regScale)){
                            continue;
                        }
                        if (!inRange(dx, dz, bigRegion.m_regScale)){
                            continue;
                        }
                        ChunkManager chunk;
                        if (!tinyRegion.extractChunk(sx, sz, chunk)){
                            continue;
                        }

                        Coordinate realChunkCoord = {
                                bigRegion.x() * 32 + dx,
                                bigRegion.z() * 32 + dz
                        };

                        chunk.readChunk(stateSettings.console());

                        if (chunk.chunkData->lastVersion == 7) {
                            chunk.chunkHeader.setNewSaveFlag(1);
                            // fix shit old xbox NBT
                            if (chunk.chunkData->entities.get<NBTList>().subType() != eNBT::COMPOUND)
                                chunk.chunkData->entities = makeList(eNBT::COMPOUND, {});

                            if (chunk.chunkData->tileEntities.get<NBTList>().subType() != eNBT::COMPOUND)
                                chunk.chunkData->tileEntities = makeList(eNBT::COMPOUND, {});

                            if (chunk.chunkData->tileTicks.get<NBTList>().subType() != eNBT::COMPOUND)
                                chunk.chunkData->tileTicks = makeList(eNBT::COMPOUND, {});

                            if (chunk.chunkData->chunkHeight == 128) {
                                chunk.chunkData->convertNBT128ToAquatic();
                            } else {
                                chunk.chunkData->convertNBT256ToAquatic();
                            }

                            if (chunk.chunkData->terrainPopulated == 1) {
                                chunk.chunkData->terrainPopulated = 2046;
                            }

                        } else if (chunk.chunkData->lastVersion == 10) {
                            chunk.chunkData->convertNBT256ToAquatic();
                            chunk.chunkHeader.setNewSaveFlag(1);
                            if (chunk.chunkData->terrainPopulated == 1) {
                                chunk.chunkData->terrainPopulated = 2046;
                            }

                        } else if (chunk.chunkData->lastVersion == 8 ||
                                   chunk.chunkData->lastVersion == 9 ||
                                   chunk.chunkData->lastVersion == 11) {
                            chunk.chunkHeader.setNewSaveFlag(1);
                            chunk.chunkData->convertOldToAquatic();

                        } else if (chunk.chunkData->lastVersion == 13) {
                            chunk.chunkHeader.setNewSaveFlag(1);
                            chunk.chunkData->convert114ToAquatic();
                        }

                        auto entityIt = entityMap.extract(realChunkCoord);
                        if (!entityIt.empty()) {
                            if (chunk.chunkData->validChunk) {
                                NBTBase nbt = std::move(entityIt.mapped().get<NBTCompound>().extract("Entities")
                                                                .value_or(makeList(eNBT::COMPOUND))/*.get<NBTList>()*/);
                                chunk.chunkData->entities = std::move(nbt);
                            }
                        }

                        chunk.writeChunk(writeSettings.getConsole());

                        if (!bigRegion.insertChunk(dx, dz, std::move(chunk))){
                            continue;
                        }

                        std::cout << "moved chunk(" << sx << ", " << sz << ") "
                                  << "tiny reg[" << tinyRegion.x() << ", " << tinyRegion.z() << "] "
                                  << "to chunk(" <<  dx << ", " << dz << ") "
                                  << "big reg[" << bigRegion.x() << ", " << bigRegion.z() << "]\n";

                    }
                }


            }
        }

        auto consoleWrite = writeSettings.getConsole();

        // create files from old regions
        std::list<LCEFile> convertedFiles;
        for (auto& [newFmt, oldFmt, entityFmt, regionMap] : dimensions) {
            for (auto& [coord, region] : regionMap) {
                Buffer buffer = region.write(consoleWrite);
                if (buffer.empty()) continue;
                auto& file = convertedFiles.emplace_back(
                        consoleWrite, std::move(buffer), 0
                );
                file.setType(oldFmt);
                file.setRegionX((i16)coord.x);
                file.setRegionZ((i16)coord.z);
                std::string fileName = file.constructFileName(consoleWrite);
                file.setFileName(fileName);
            }
        }

        fileListing->addFiles(std::move(convertedFiles));
    }







    Buffer FileListing::writeListing(StateSettings& stateSettings, WriteSettings& writeSettings) {
        static constexpr u32 WSTRING_SIZE = 64;
        static constexpr u32 FILELISTING_HEADER_SIZE = 12;
        static const std::set<lce::FILETYPE> kOldGenRegions = {
                lce::FILETYPE::OLD_REGION_OVERWORLD,
                lce::FILETYPE::OLD_REGION_NETHER,
                lce::FILETYPE::OLD_REGION_END
        };
        static const std::set<lce::FILETYPE> kNewGenRegions = {
                lce::FILETYPE::NEW_REGION_OVERWORLD,
                lce::FILETYPE::NEW_REGION_NETHER,
                lce::FILETYPE::NEW_REGION_END
        };
        static const std::set<lce::FILETYPE> kEntities = {
                lce::FILETYPE::ENTITY_OVERWORLD,
                lce::FILETYPE::ENTITY_NETHER,
                lce::FILETYPE::ENTITY_END
        };

        auto consoleIn = stateSettings.console();
        auto consoleOut = writeSettings.getConsole();

        // old gen -> old gen
        if (!lce::isConsoleNewGen(consoleIn) && !lce::isConsoleNewGen(consoleOut)) {
            std::cout << "[-] rewriting all region chunks to adjust endian, this may take a minute.\n";
            for (LCEFile& file : view_of(kOldGenRegions)) {
                editor::convertChunksToAquatic(file, consoleIn, consoleOut);
            }

        // new gen -> old gen
        } else if (lce::isConsoleNewGen(consoleIn) && !lce::isConsoleNewGen(consoleOut)) {
            std::cout << "[-] rewriting + reordering all region chunks, this may take a minute.\n";
            // removeFileTypes(kEntities);
            convertNewGenChunksToOldGen(this, stateSettings, writeSettings);
            std::set<lce::FILETYPE> levelSet = {lce::FILETYPE::LEVEL};
            for (auto& level : view_of(levelSet)) {
                DataReader reader(level.m_data.span());
                NBTBase nbt;
                nbt.read(reader);
                if (auto* nbtData = nbt.getTag("Data"); nbtData) {
                    int xzSize = nbtData->value<i32>("XZSize").value_or(54);
                    if (xzSize != 54) {
                        nbtData->setTag("HellScale", makeInt(3));
                        nbtData->setTag("XZSize", makeInt(54));
                        nbtData->setTag("StrongholdX", makeInt(0));
                        nbtData->setTag("StrongholdZ", makeInt(0));
                        nbtData->setTag("StrongholdEndPortalX", makeInt(0));
                        nbtData->setTag("StrongholdEndPortalZ", makeInt(0));
                    }
                }
                DataWriter writer;
                nbt.write(writer);
                level.m_data = std::move(writer.take());
            }
        } else {
            convertRegions(consoleOut);
        }

        // step 1: get the file count and size of all sub-files
        c_u32 fileCount = m_allFiles.size();

        u32 fileDataSize = 0;
        for (const editor::LCEFile& file: m_allFiles) {
            fileDataSize += file.m_data.size();
        }

        c_u32 fileInfoOffset = fileDataSize + FILELISTING_HEADER_SIZE;
        u32 FOOTER_ENTRY_SIZE = (currentVersion() > 1) ? 144 : 136;

        // step 2: find total binary size and create its data buffer
        c_u32 totalFileSize = fileInfoOffset + FOOTER_ENTRY_SIZE * fileCount;
        DataWriter writer(totalFileSize, getConsoleEndian(consoleOut));

        // step 3: write start
        writer.write<u32>(fileInfoOffset);
        u32 innocuousVariableName = fileCount;
        if (currentVersion() <= 1) {
            innocuousVariableName *= 136;
        }
        writer.write<u32>(innocuousVariableName);
        writer.write<u16>(oldestVersion());
        writer.write<u16>(currentVersion());


        // step 4: write each files data
        u32 offsetIndex = 0;
        u32 totalOffset = FILELISTING_HEADER_SIZE;
        u32* fileOffsets = new u32[m_allFiles.size()];

        for (editor::LCEFile& fileIter : m_allFiles) {
            fileOffsets[offsetIndex++] = totalOffset;
            totalOffset += fileIter.m_data.size();
            writer.writeBytes(fileIter.m_data.data(), fileIter.m_data.size());
        }

        // step 5: write file metadata
        offsetIndex = 0;
        for (const editor::LCEFile& fileIter: m_allFiles) {
            std::string fileIterName = fileIter.constructFileName(consoleOut);
            writer.writeWStringFromString(fileIterName, WSTRING_SIZE);
            writer.write<u32>(fileIter.m_data.size());
            writer.write<u32>(fileOffsets[offsetIndex]);
            if (currentVersion() > 1) {
                writer.write<u64>(fileIter.m_timestamp);
            }
            offsetIndex++;
        }

        delete[] fileOffsets;

        return writer.take();
    }







    MU ND int FileListing::preprocess(StateSettings& stateSettings, WriteSettings& theWriteSettings) {
        if (!theWriteSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        const bool diffConsoles = stateSettings.console() != theWriteSettings.getConsole();

        // TODO: create default output file path if not set
        if (theWriteSettings.shouldRemovePlayers/* || diffConsoles*/) {
            removeFileTypes({lce::FILETYPE::PLAYER});
        }
        if (theWriteSettings.shouldRemoveDataMapping/* || diffConsoles*/) {
            removeFileTypes({lce::FILETYPE::DATA_MAPPING});
        }
        if (theWriteSettings.shouldRemoveMaps) {
            removeFileTypes({lce::FILETYPE::MAP});
        }
        if (theWriteSettings.shouldRemoveStructures/* || diffConsoles*/) {
            removeFileTypes({lce::FILETYPE::STRUCTURE, lce::FILETYPE::VILLAGE});
        }
        if (theWriteSettings.shouldRemoveRegionsOverworld) {
            removeFileTypes({lce::FILETYPE::OLD_REGION_OVERWORLD, lce::FILETYPE::NEW_REGION_OVERWORLD});
        }
        if (theWriteSettings.shouldRemoveRegionsNether) {
            removeFileTypes({lce::FILETYPE::OLD_REGION_NETHER, lce::FILETYPE::NEW_REGION_NETHER});
        }
        if (theWriteSettings.shouldRemoveRegionsEnd) {
            removeFileTypes({lce::FILETYPE::OLD_REGION_END, lce::FILETYPE::NEW_REGION_END});
        }
        // TODO: properly convert GRF
        if (lce::getConsoleEndian(stateSettings.console()) != lce::getConsoleEndian(theWriteSettings.getConsole())) {
            removeFileTypes({lce::FILETYPE::GRF});
        }

        return 0;
    }

}