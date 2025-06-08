#pragma once

#include "include/lce/processor.hpp"

#include "include/lce/blocks/blockID.hpp"

#include "code/Region/Region.hpp"

#include "code/Chunk/helpers/NBTFixer.hpp"
#include "code/Impl/level.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


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


    void convertReadChunkToAquatic(ChunkManager& chunk) {
        chunk::ChunkData* chunkData = chunk.chunkData;
        if (chunkData->lastVersion == 7) {
            // fix shit old xbox NBT
            if (chunkData->entities.subType() != eNBT::COMPOUND)
                chunkData->entities = NBTList(eNBT::COMPOUND);

            if (chunkData->tileEntities.subType() != eNBT::COMPOUND)
                chunkData->tileEntities = NBTList(eNBT::COMPOUND);

            if (chunkData->tileTicks.subType() != eNBT::COMPOUND)
                chunkData->tileTicks = NBTList(eNBT::COMPOUND);

            if (chunkData->terrainPopulatedFlags == 1) {
                chunkData->terrainPopulatedFlags = 2046;
            }

        } else if (chunkData->lastVersion == 8 ||
                   chunkData->lastVersion == 9 ||
                   chunkData->lastVersion == 10 ||
                   chunkData->lastVersion == 11) {

        } else if (chunkData->lastVersion == 13) {

            for (int i = 0; i < 65536; i++) {
                c_u16 id1 = chunkData->blocks[i] >> 4 & 1023;
                if ((id1 > 259 && id1 < 263) || id1 > 318) {
                    chunkData->blocks[i] = lce::blocks::COBBLESTONE_ID << 4;
                }
                continue;
                c_u16 id2 = chunkData->submerged[i] >> 4 & 1023;
                if (id2 > 250) {
                    chunkData->submerged[i] = lce::blocks::COBBLESTONE_ID << 4;
                }
            }

        }

        chunk.chunkHeader.setNewSaveFlag(1);
        chunkData->lastVersion = 12;
    }



    void convertReadChunkToElytra(ChunkManager& chunk) {
        auto chunkData = chunk.chunkData;
        if (chunkData->lastVersion == 7) {

        } else if (chunkData->lastVersion == 8 ||
                   chunkData->lastVersion == 9 ||
                   chunkData->lastVersion == 10 ||
                   chunkData->lastVersion == 11) {


        }
        else if (chunkData->lastVersion == 12 ||
                 chunkData->lastVersion == 13) {
            for (int i = 0; i < 65536; i++) {
                u16 block = chunkData->blocks[i];
                if (((block & 0x1FF0) >> 4) > 255) {
                    block = lce::blocks::COBBLESTONE_ID << 4;
                }

                chunkData->blocks[i] = block;
            }

        }


        chunk.chunkHeader.setNewSaveFlag(0);
        chunkData->lastVersion = 10;
    }





    void convertReadChunkToElytra(LCEFile& file,
                                const lce::CONSOLE inConsole, const lce::CONSOLE outConsole) {
        Region region;
        region.read(&file);

        for (int i = 0; i < 1024; i++) {
            ChunkManager& chunkManager = region.m_chunks[i];
            chunkManager.readChunk(inConsole);
            if (!chunkManager.chunkData->validChunk) continue;
            convertReadChunkToElytra(chunkManager);
            chunkManager.writeChunk(outConsole);
        }

        file.setBuffer(region.write(outConsole));
        file.m_console = outConsole;
    }


    void convertReadChunkToAquatic(LCEFile& file,
                                  const lce::CONSOLE inConsole, const lce::CONSOLE outConsole) {
        Region region;
        region.read(&file);

        for (int i = 0; i < 1024; i++) {
            ChunkManager& chunkManager = region.m_chunks[i];
            chunkManager.readChunk(inConsole);
            if (!chunkManager.chunkData->validChunk) continue;
            convertReadChunkToAquatic(chunkManager);
            chunkManager.writeChunk(outConsole);
        }

        file.setBuffer(region.write(outConsole));
        file.m_console = outConsole;
    }








    void convertOldGenChunksToNewGen(SaveProject& saveProject,
                                     WriteSettings& writeSettings) {

        // -2     -1     0     1     2     3
        // -4,-3  -2,-1  0,1   2,3   4,5   6,7
        auto makeBigger = [](int v) {
            return v * 2;
        };

        struct EntityStruct {
            Coordinate coordinate{};
            NBTBase nbt;
        };

        using ft = lce::FILETYPE;
        using EntityList = std::list<EntityStruct>;

        std::vector<std::tuple<ft, ft, ft, EntityList>> dimensions;

        dimensions.reserve(3);
        dimensions.emplace_back(ft::NEW_REGION_OVERWORLD, ft::OLD_REGION_OVERWORLD, ft::ENTITY_OVERWORLD, EntityList{});
        dimensions.emplace_back(ft::NEW_REGION_NETHER,    ft::OLD_REGION_NETHER,    ft::ENTITY_NETHER,    EntityList{});
        dimensions.emplace_back(ft::NEW_REGION_END,       ft::OLD_REGION_END,       ft::ENTITY_END,       EntityList{});


        auto consoleWrite = writeSettings.getConsole();
        std::list<LCEFile> convertedFiles;

        for (auto& [newFmt, oldFmt, entityFmt, entityList] : dimensions) {

            // (3) collect old format
            std::list<LCEFile> regionFiles = saveProject.collectFiles(oldFmt);

            // (4) place contents into old regions
            for (auto& regionFile: regionFiles) {

                Coordinate bigCoord = {
                        makeBigger(regionFile.getRegionX()),
                        makeBigger(regionFile.getRegionZ())
                };

                Region tinyRegions[4] = {
                    {bigCoord.x    , bigCoord.z    },
                    {bigCoord.x    , bigCoord.z + 1},
                    {bigCoord.x + 1, bigCoord.z    },
                    {bigCoord.x + 1, bigCoord.z + 1}
                };

                for (int sx = 0; sx < 32; ++sx) {
                    for (int sz = 0; sz < 32; ++sz) {
                        int rx = sx / 16;
                        int rz = sz / 16;
                        int cx = sx & 15;
                        int cz = sx & 15;

                        auto& tinyRegion = tinyRegions[rx * 2 + rz];

                        ChunkManager chunk;
                        if (!tinyRegion.extractChunk(sx, sz, chunk)) {
                            continue;
                        }

                        chunk.readChunk(saveProject.m_stateSettings.console());
                        if (!chunk.chunkData->validChunk) continue;
                        convertReadChunkToAquatic(chunk);

                        // move entities from the chunk, into the ``entityList``
                        if (!chunk.chunkData->entities.empty()) {
                            entityList.emplace_back(
                                Coordinate{chunk.chunkData->chunkX, chunk.chunkData->chunkZ},
                                    makeCompound( { {"", makeList(chunk.chunkData->entities) } } )
                            );
                            chunk.chunkData->entities = NBTList(eNBT::COMPOUND);
                        }


                        chunk.writeChunk(consoleWrite);

                        if (!tinyRegion.insertChunk(cx, cz, std::move(chunk))) {
                            continue;
                        }

                        // std::cout << "moved chunk(" << sx << ", " << sz << ") "
                        //           << "tiny reg[" << tinyRegion.x() << ", " << tinyRegion.z() << "] "
                        //           << "to chunk(" <<  dx << ", " << dz << ") "
                        //           << "big reg[" << bigRegion.x() << ", " << bigRegion.z() << "]\n";

                    }
                }

                // save regions
                for (auto& tinyRegion : tinyRegions) {
                    Buffer buffer = tinyRegion.write(consoleWrite);
                    if (buffer.empty()) continue;
                    auto& file = convertedFiles.emplace_back(
                            consoleWrite,
                            0,
                            saveProject.m_tempFolder,
                            ""
                    );
                    file.setType(newFmt);
                    file.setRegionX((i16)tinyRegion.x());
                    file.setRegionZ((i16)tinyRegion.z());
                    std::string fileName = file.constructFileName(consoleWrite);
                    file.setFileName(fileName);
                    file.setBuffer(std::move(buffer));
                }

                // save entities
                DataWriter writer;
                writer.write<u32>(entityList.size());
                for (auto& [coord, entity] : entityList) {
                    writer.write<i32>(coord.x);
                    writer.write<i32>(coord.z);
                    entity.write(writer);
                }
                auto& entityFile = convertedFiles.emplace_back(
                        consoleWrite,
                        0,
                        saveProject.m_tempFolder,
                        ""
                );
                entityFile.setType(entityFmt);
                std::string fileName = entityFile.constructFileName(consoleWrite);
                entityFile.setFileName(fileName);
                entityFile.setBuffer(std::move(writer.take()));




            }




        }

        saveProject.addFiles(std::move(convertedFiles));
    }











    void convertNewGenChunksToOldGen(SaveProject& saveProject,
                                     WriteSettings& writeSettings) {

        std::vector<std::vector<int>> positions = {
                {-1, -1}, {-1, 0}, {0, -1}, {0, 0}
        };

        // -4 -3 -2 -1  0  1  2  3  4  5
        // -2 -2 -1 -1  0  0  1  1  2  2
        auto makeSmaller = [](int v) {
            return (v >= 0) ? v / 2 : (v - 1) / 2;
        };

        using ft = lce::FILETYPE;
        using Map = std::unordered_map<Coordinate, editor::Region>;

        std::vector<std::tuple<ft, ft, ft, Map>> dimensions;
        dimensions.reserve(3);
        dimensions.emplace_back(ft::NEW_REGION_OVERWORLD, ft::OLD_REGION_OVERWORLD, ft::ENTITY_OVERWORLD, Map{});
        dimensions.emplace_back(ft::NEW_REGION_NETHER,    ft::OLD_REGION_NETHER,    ft::ENTITY_NETHER,    Map{});
        dimensions.emplace_back(ft::NEW_REGION_END,       ft::OLD_REGION_END,       ft::ENTITY_END,       Map{});


        saveProject.removeFileTypes({lce::FILETYPE::NEW_REGION_NETHER, lce::FILETYPE::NEW_REGION_NETHER});

        for (auto& [newFmt, oldFmt, entityFmt, regionMap] : dimensions) {

            // (1) read entities file
            std::list<LCEFile> entityFileList = saveProject.collectFiles(entityFmt);
            std::unordered_map<Coordinate, NBTBase> entityMap;
            if (!entityFileList.empty()) {
                LCEFile& entityFile = entityFileList.front();
                Buffer entityBuffer = entityFile.getBuffer();
                DataReader entityReader(entityBuffer.span());
                int entityCount = entityReader.read<i32>();
                for (int i = 0; i < entityCount; i++) {
                    int chunkX = entityReader.read<i32>();
                    int chunkZ = entityReader.read<i32>();
                    NBTBase nbt = NBTBase::read(entityReader);
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
            std::list<LCEFile> regionFiles = saveProject.collectFiles(newFmt);

            // (4) place contents into old regions
            for (auto& regionFile: regionFiles) {
                Coordinate tinyCoord(
                        makeSmaller(regionFile.getRegionX()),
                        makeSmaller(regionFile.getRegionZ())
                );
                if (tinyCoord.x < -1 || tinyCoord.x > 0 || tinyCoord.x < -1 || tinyCoord.x > 0) {
                    continue;
                }

                Region tinyRegion;
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
                Region& bigRegion = bigIt->second;
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


                        chunk.readChunk(saveProject.m_stateSettings.console());
                        if (!chunk.chunkData->validChunk) continue;
                        // convertReadChunkToAquatic(chunk);
                        convertReadChunkToElytra(chunk);

                        auto entityIt = entityMap.extract(realChunkCoord);
                        if (!entityIt.empty()) {
                            if (chunk.chunkData->validChunk) {

                                NBTBase& nbtRoot = entityIt.mapped();
                                if (nbtRoot("")) { nbtRoot = nbtRoot[""]; }

                                NBTList& entityList = nbtRoot.ensureList("Entities", eNBT::COMPOUND);
                                chunk.chunkData->entities = std::move(entityList);
                            }
                        }

                        NBTFixer::fixEntities(chunk.chunkData);
                        NBTFixer::fixTileEntities(chunk.chunkData);

                        chunk.writeChunk(writeSettings.getConsole());

                        if (!bigRegion.insertChunk(dx, dz, std::move(chunk))) {
                            continue;
                        }

                        // std::cout << "moved chunk(" << sx << ", " << sz << ") "
                        //           << "tiny reg[" << tinyRegion.x() << ", " << tinyRegion.z() << "] "
                        //           << "to chunk(" <<  dx << ", " << dz << ") "
                        //           << "big reg[" << bigRegion.x() << ", " << bigRegion.z() << "]\n";

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
                        consoleWrite,
                        0,
                        saveProject.m_tempFolder,
                        ""
                );
                file.setType(oldFmt);
                file.setRegionX((i16)coord.x);
                file.setRegionZ((i16)coord.z);
                std::string fileName = file.constructFileName(consoleWrite);
                file.setFileName(fileName);
                file.setBuffer(std::move(buffer));
            }
        }

        saveProject.m_stateSettings.setNewGen(false);
        saveProject.addFiles(std::move(convertedFiles));
    }


    MU void convertRegions(SaveProject& saveProject, lce::CONSOLE consoleOut) {
        for (LCEFile& file : saveProject.view_of(lce::FILETYPE::OLD_REGION_ANY)) {
            Region region;
            region.read(&file);
            region.convertChunks(consoleOut);

            file.setBuffer(region.write(consoleOut));
            file.m_console = consoleOut;
        }
    }



    MU ND int preprocess(SaveProject& saveProject, StateSettings& stateSettings, WriteSettings& theWriteSettings) {
        if (!theWriteSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        const bool diffConsoles = stateSettings.console() != theWriteSettings.getConsole();

        // TODO: create default output file path if not set
        if (theWriteSettings.shouldRemovePlayers || diffConsoles) {
            saveProject.removeFileTypes({lce::FILETYPE::PLAYER});
        }
        if (theWriteSettings.shouldRemoveDataMapping || diffConsoles) {
            saveProject.removeFileTypes({lce::FILETYPE::DATA_MAPPING});
        }
        if (theWriteSettings.shouldRemoveMaps) {
            saveProject.removeFileTypes({lce::FILETYPE::MAP});
        }
        if (theWriteSettings.shouldRemoveStructures || diffConsoles) {
            saveProject.removeFileTypes({lce::FILETYPE::STRUCTURE, lce::FILETYPE::VILLAGE});
        }
        if (theWriteSettings.shouldRemoveRegionsOverworld) {
            saveProject.removeFileTypes({lce::FILETYPE::OLD_REGION_OVERWORLD, lce::FILETYPE::NEW_REGION_OVERWORLD});
        }
        if (theWriteSettings.shouldRemoveRegionsNether) {
            saveProject.removeFileTypes({lce::FILETYPE::OLD_REGION_NETHER, lce::FILETYPE::NEW_REGION_NETHER});
        }
        if (theWriteSettings.shouldRemoveRegionsEnd) {
            saveProject.removeFileTypes({lce::FILETYPE::OLD_REGION_END, lce::FILETYPE::NEW_REGION_END});
        }
        // TODO: properly convert GRF
        if (lce::getConsoleEndian(stateSettings.console()) != lce::getConsoleEndian(theWriteSettings.getConsole())) {
            saveProject.removeFileTypes({lce::FILETYPE::GRF});
        }

        return 0;
    }




    void convert(SaveProject& saveProject, WriteSettings& writeSettings) {
        StateSettings& stateSettings = saveProject.m_stateSettings;

        auto consoleIn = stateSettings.console();
        auto consoleOut = writeSettings.getConsole();

        // saveProject.removeFileTypes(SaveProject::s_OLD_REGION_ANY);

        // old gen -> old gen
        if (!lce::isConsoleNewGen(consoleIn) && !lce::isConsoleNewGen(consoleOut)) {
            std::cout << "[-] rewriting all region chunks to adjust endian, this may take a minute.\n";
            for (LCEFile& file : saveProject.view_of(lce::FILETYPE::OLD_REGION_ANY)) {
                editor::convertReadChunkToElytra(file, consoleIn, consoleOut);
            }

            // auto files = saveProject.collectFiles(lce::FILETYPE::LEVEL);

            if (auto levelFileOpt1 = saveProject.findFile(lce::FILETYPE::LEVEL);
                levelFileOpt1.has_value()) {
                LCEFile& levelFile = levelFileOpt1.value().get();
                editor::Level level;
                level.read(levelFile);
                // level.m_generatorName = "buffet";
                // level.m_RandomSeed = -101;
                level.write(levelFile, TU46);
            }


            // new gen -> old gen
        } else if (lce::isConsoleNewGen(consoleIn) && !lce::isConsoleNewGen(consoleOut)) {
            std::cout << "[-] re-writing/ordering all region chunks, this may take a minute.\n";
            std::cout << std::flush;

            if (auto levelFileOpt2 = saveProject.findFile(lce::FILETYPE::LEVEL);
                levelFileOpt2.has_value()) {
                LCEFile& levelFile = levelFileOpt2.value().get();

                editor::Level level;
                level.read(levelFile);
                if (level.m_XZSize && level.m_XZSize != 54) {
                    level.m_DataVersion = 922;
                    level.m_HellScale = 3;
                    level.m_XZSize = 54;
                    level.m_StrongholdX = 0;
                    level.m_StrongholdZ = 0;
                    level.m_StrongholdEndPortalX = 0;
                    level.m_StrongholdEndPortalZ = 0;
                    level.m_hasStrongholdEndPortal = 0;
                }
                level.write(levelFile, TU46);
            }
            convertNewGenChunksToOldGen(saveProject, writeSettings);
        } else {
            convertRegions(saveProject, consoleOut);
        }
    }





}
