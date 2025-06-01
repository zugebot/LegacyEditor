#pragma once

#include "include/lce/processor.hpp"

#include "include/lce/blocks/blockID.hpp"

#include "code/Region/Region.hpp"

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

    /*
    void processRegion(size_t regionIndex, FileListing& fileListing) {
        const lce::CONSOLE console = fileListing.myReadSettings.console();
        if (regionIndex >= fileListing.ptrs.old_reg_overworld.size()) { return; }

    // read a region file
    Region region;
    region.read(fileListing.ptrs.old_reg_overworld[regionIndex]);
    for (ChunkManager& chunkManager: region.chunks) {
        if (chunkManager.size == 0) {
            continue;
        }

        chunkManager.ensureDecompress(console);
        chunkManager.readChunk(console);
        auto* chunkData = chunkManager.chunkData;

        if (!chunkData->validChunk) {
            continue;
        }

        u16 blocks[65536];
        for (u16 x = 0; x < 16; x++) {
            for (u16 z = 0; z < 16; z++) {
                for (u16 y = 0; y < 256; y++) {
                    u16 block1 = chunkManager.chunkData->getBlock(x, y, z);
                    u16 data_1 = 0;
                    c_int offset1 = y + 256 * z + 4096 * x;

                    c_u16 compare1 = (block1 & 0x1FF0) >> 4;
                    if ((block1 & 0x8000) != 0) { // fix stupid blocks
                        if (compare1 == 271) {    // sea pickle
                            block1 = (block1 & 0x9FF7) | 0x08;
                        }
                        if (compare1 == 272) { // bubble column
                            block1 = (block1 & 0x7FFF) | 0b1111;
                        }
                    }

                    blocks[offset1] = block1 | data_1;



                    // block1 = BlockID::AIR_ID;
                    // if (y == 0) { block1 = BlockID::BEDROCK_ID; }
                    //
                    // u32 jumpBL, jumpDA;
                    //
                    // if (y == 1) {
                    //     if (x % 2 != 0 || z % 2 != 0) {
                    //         if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                    //             chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                    //             block1 = BlockID::AIR_ID;
                    //             goto END;
                    //         }
                    //         block1 = BlockID::BEDROCK_ID;
                    //         goto END;
                    //     }
                    // }
                    //
                    // if (y == 2) {
                    //     if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                    //         chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                    //         goto END;
                    //     }
                    //
                    //     if (x % 2 != 0 || z % 2 != 0) {
                    //         goto END;
                    //     }
                    //
                    //     jumpBL = 8 * (chunkData->chunkX + 16);
                    //     jumpDA = 8 *  chunkData->chunkZ;
                    //
                    //     block1 = (jumpBL + x / 2) << 4;
                    //     data_1 = jumpDA + z / 2;
                    //
                    //     if (block1 > 252 << 4 || block1 == BlockID::BEACON_ID) {
                    //         block1 = 0;
                    //         data_1 = 0;
                    //     }
                    // }

                END:

                    blocks[offset1] = block1 | data_1;

                }
            }
        }

        std::memcpy(&chunkData->newBlocks[0], &blocks[0], 131072);
        // shuffleArray(&chunkData->newBlocks[0], 65535);
        // memset(&chunkData->biomes[0], 0x0B, 256);
        // memset(&chunkData->blockLight[0], 0xFF, 32768);
        // memset(&chunkData->skyLight[0], 0xFF, 32768);
        // memset(&chunkData->heightMap[0], 0xFF, 256);
        chunkData->terrainPopulated = 2046;
        chunkData->lastUpdate = 100;
        chunkData->inhabitedTime = 200;

        chunkData->defaultNBT();
        chunkManager.writeChunk(console);
        chunkManager.ensureCompressed(console);
    }

    fileListing.ptrs.old_reg_overworld[regionIndex]->data.deallocate();
    fileListing.ptrs.old_reg_overworld[regionIndex]->data = region.write(console);
}
*/


        /**
     * Removes all blocks in the nether except for netherrack
     * DO NOT USE THIS IT NEEDS FIXED
     * @param regionIndex
     * @param fileListing
     */
        /*
    void removeNetherrack(size_t regionIndex, FileListing& fileListing) {
        const lce::CONSOLE console = fileListing.myReadSettings.console();
        if (regionIndex >= fileListing.ptrs.old_reg_nether.size()) { return; }

        // read a region file
        Region region;
        region.read(fileListing.ptrs.old_reg_nether[regionIndex]);

        for (ChunkManager& chunkManager: region.chunks) {
            if (chunkManager.size == 0) {
                continue;
            }

            chunkManager.ensureDecompress(console);
            chunkManager.readChunk(console);
            auto* chunkData = chunkManager.chunkData;

            if (!chunkData->validChunk) {
                continue;
            }

            u16 blocks[65536];
            for (u16 x = 0; x < 16; x++) {
                for (u16 z = 0; z < 16; z++) {
                    for (u16 y = 0; y < 256; y++) {
                        u16 block1 = chunkManager.chunkData->getBlock(x, y, z);
                        c_int offset1 = y + 256 * z + 4096 * x;

                        if ((block1 & 0x1FF0) >> 4 != 7) {
                            block1 = 0;
                        }

                        blocks[offset1] = block1;
                    }
                }
            }

            std::memcpy(chunkData->newBlocks.data(), &blocks[0], 131072);
            memset(chunkData->blockLight.data(), 0xFF, 32768);
            memset(chunkData->skyLight.data(), 0xFF, 32768);
            chunkData->terrainPopulated = 2046;

            chunkData->defaultNBT();
            chunkManager.writeChunk(console);
            chunkManager.ensureCompressed(console);
        }

        fileListing.ptrs.old_reg_nether[regionIndex]->data.deallocate();
        fileListing.ptrs.old_reg_nether[regionIndex]->data = region.write(console);
    }
      */


    void convertReadChunkToAquatic(ChunkManager& chunk) {
        chunk::ChunkData* chunkData = chunk.chunkData;
        if (chunkData->lastVersion == 7) {
            chunk.chunkHeader.setNewSaveFlag(1);
            // fix shit old xbox NBT
            if (chunkData->entities.get<NBTList>().subType() != eNBT::COMPOUND)
                chunkData->entities = makeList(eNBT::COMPOUND, {});

            if (chunkData->tileEntities.get<NBTList>().subType() != eNBT::COMPOUND)
                chunkData->tileEntities = makeList(eNBT::COMPOUND, {});

            if (chunkData->tileTicks.get<NBTList>().subType() != eNBT::COMPOUND)
                chunkData->tileTicks = makeList(eNBT::COMPOUND, {});

            if (chunkData->chunkHeight == 128) {
                chunkData->convertNBT128ToAquatic();
            } else {
                chunkData->convertNBT256ToAquatic();
            }

            if (chunkData->terrainPopulated == 1) {
                chunkData->terrainPopulated = 2046;
            }

        } else if (chunkData->lastVersion == 10) {
            chunkData->convertNBT256ToAquatic();
            chunk.chunkHeader.setNewSaveFlag(1);
            if (chunkData->terrainPopulated == 1) {
                chunkData->terrainPopulated = 2046;
            }

        } else if (chunkData->lastVersion == 8 ||
                   chunkData->lastVersion == 9 ||
                   chunkData->lastVersion == 11) {
            chunk.chunkHeader.setNewSaveFlag(1);
            chunkData->convertOldToAquatic();

        } else if (chunkData->lastVersion == 13) {
            chunk.chunkHeader.setNewSaveFlag(1);
            chunkData->convert114ToAquatic();

        }
        /*
        else if (chunkManager.chunkData->lastVersion != 12) {
            std::cout << "[?] Chunk with version " << chunkManager.chunkData->lastVersion << "\n";
        }
         */
    }



    void convertReadChunkToElytra(ChunkManager& chunk) {
        chunk::ChunkData* chunkData = chunk.chunkData;
        if (chunkData->lastVersion == 7) {
            chunk.chunkHeader.setNewSaveFlag(1);
            // fix shit old xbox NBT
            // if (chunkData->entities.get<NBTList>().subType() != eNBT::COMPOUND)
            //     chunkData->entities = makeList(eNBT::COMPOUND, {});
            //
            // if (chunkData->tileEntities.get<NBTList>().subType() != eNBT::COMPOUND)
            //     chunkData->tileEntities = makeList(eNBT::COMPOUND, {});

            // if (chunkData->tileTicks.get<NBTList>().subType() != eNBT::COMPOUND)
            //     chunkData->tileTicks = makeList(eNBT::COMPOUND, {});

            // if (chunkData->chunkHeight == 128) {
            //     chunkData->convertNBT128ToAquatic();
            // } else {
            //     chunkData->convertNBT256ToAquatic();
            // }

            // if (chunkData->terrainPopulated == 1) {
            //     chunkData->terrainPopulated = 2046;
            // }

        } else if (chunkData->lastVersion == 10) {
            // chunkData->convertNBT256ToAquatic();
            // chunk.chunkHeader.setNewSaveFlag(1);
            // if (chunkData->terrainPopulated == 1) {
            //     chunkData->terrainPopulated = 2046;
            // }

        } else if (chunkData->lastVersion == 8 ||
                   chunkData->lastVersion == 9 ||
                   chunkData->lastVersion == 11) {
            // chunk.chunkHeader.setNewSaveFlag(1);

        }
        else if (chunkData->lastVersion == 12) {
            chunkData->convertAquaticToElytra();
        } else if (chunkData->lastVersion == 13) {
            // chunk.chunkHeader.setNewSaveFlag(1);
            // chunkData->convert114ToAquatic();

        }
        /*
        else if (chunkManager.chunkData->lastVersion != 12) {
            std::cout << "[?] Chunk with version " << chunkManager.chunkData->lastVersion << "\n";
        }
         */
    }





    void convertChunksToAquatic(LCEFile& file,
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
                        if (!chunk.chunkData->entities.get<NBTList>().empty()) {
                            entityList.emplace_back(
                                Coordinate{chunk.chunkData->chunkX, chunk.chunkData->chunkZ},
                                    makeCompound( { {"", chunk.chunkData->entities} } )
                            );
                            chunk.chunkData->entities = NBTBase();
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
                    NBTBase nbt;
                    entityReader.skip(3);
                    nbt.read(entityReader);
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
                        convertReadChunkToAquatic(chunk);

                        auto entityIt = entityMap.extract(realChunkCoord);
                        if (!entityIt.empty()) {
                            if (chunk.chunkData->validChunk) {
                                NBTBase nbt = std::move(entityIt.mapped().get<NBTCompound>().extract("Entities")
                                                                .value_or(makeList(eNBT::COMPOUND)));
                                chunk.chunkData->entities = std::move(nbt);
                            }
                        }

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

        saveProject.addFiles(std::move(convertedFiles));
    }


    MU void convertRegions(SaveProject& saveProject, lce::CONSOLE consoleOut) {
        static const std::set<lce::FILETYPE> regionTypes = {
                lce::FILETYPE::OLD_REGION_NETHER,
                lce::FILETYPE::OLD_REGION_OVERWORLD,
                lce::FILETYPE::OLD_REGION_END
        };

        for (LCEFile& file : saveProject.view_of(regionTypes)) {
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
            for (LCEFile& file : saveProject.view_of(kOldGenRegions)) {
                editor::convertChunksToAquatic(file, consoleIn, consoleOut);
            }

            // new gen -> old gen
        } else if (lce::isConsoleNewGen(consoleIn) && !lce::isConsoleNewGen(consoleOut)) {
            std::cout << "[-] re-writing/ordering all region chunks, this may take a minute.\n";
            std::cout << std::flush;
            convertNewGenChunksToOldGen(saveProject, writeSettings);
            std::set<lce::FILETYPE> levelSet = {lce::FILETYPE::LEVEL};
            for (auto& level : saveProject.view_of(levelSet)) {
                Buffer levelBuffer = level.getBuffer();
                DataReader reader(levelBuffer);
                NBTBase nbt;
                nbt.read(reader);
                if (!nbt.hasKey("Data")) return;
                auto nbtData = nbt["Data"];
                int xzSize = nbtData->value<i32>("XZSize").value_or(54);
                if (xzSize != 54) {
                    nbtData["HellScale"] = makeInt(3);
                    nbtData["XZSize"] = makeInt(54);
                    nbtData["StrongholdX"] = makeInt(0);
                    nbtData["StrongholdZ"] = makeInt(0);
                    nbtData["StrongholdEndPortalX"] = makeInt(0);
                    nbtData["StrongholdEndPortalZ"] = makeInt(0);
                }
                DataWriter writer;
                nbt.write(writer);
                level.setBuffer(std::move(writer.take()));
            }
        } else {
            convertRegions(saveProject, consoleOut);
        }
    }





}
