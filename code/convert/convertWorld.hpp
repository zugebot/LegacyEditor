#pragma once

#include "include/lce/processor.hpp"

#include "include/lce/blocks/blockID.hpp"

#include "code/Region/Region.hpp"

#include "code/Impl/levelFile.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "code/chunk/helpers/NBTFixer.hpp"
#include "code/Impl/EntityFile.hpp"


#include "code/Impl/EntityFile.hpp"
#include "code/convert/convertChunk.hpp"
#include "common/Pos2DTemplate.hpp"


namespace editor {


    template<bool NewGen>
    MU void convertRegions(SaveProject& saveProject,
                           WriteSettings& settings) {

        static constexpr lce::FILETYPE REGION_TYPE
                = NewGen ? lce::FILETYPE::NEW_REGION_ANY : lce::FILETYPE::OLD_REGION_ANY;

        c_auto consoleOut = settings.m_schematic.save_console;

        for (LCEFile& file : saveProject.view_of(REGION_TYPE)) {

            Region region;

            // Buffer buf = file.getBuffer();
            // DataWriter::writeFile("regionIn.dat", buf.span());

            region.read(&file);
            // DataWriter::writeFile("regionOut.dat", bufOut.span());


            for (int i = 0; i < 1024; i++) {
                ChunkHandle& handle = region.m_handles[i];
                handle.decodeChunk(file.m_console);
                if (!handle.data->validChunk) continue;
                settings.m_schematic.func_chunk_convert(handle, settings);
                handle.encodeChunk(settings);
            }

            Buffer bufOut = region.write(settings);

            file.setBuffer(std::move(bufOut));
            file.m_console = consoleOut;
        }
    }


    void convertRegionsOld2New(SaveProject& saveProject,
                                WriteSettings& settings) {

        // -2     -1     0     1     2     3
        // -4,-3  -2,-1  0,1   2,3   4,5   6,7
        auto makeBigger = [](int v) {
            return v * 2;
        };

        using ft = lce::FILETYPE;
        using EntityList = std::list<EntityStruct>;

        std::vector<std::tuple<ft, ft, ft, EntityFile::EntityList>> dimensions;
        dimensions.reserve(3);
        auto addDimension = [&](auto a, auto b, auto c) {
            dimensions.emplace_back(a, b, c, EntityFile::EntityList{});
        };
        addDimension(ft::NEW_REGION_OVERWORLD, ft::OLD_REGION_OVERWORLD, ft::ENTITY_OVERWORLD);
        addDimension(ft::NEW_REGION_NETHER,    ft::OLD_REGION_NETHER,    ft::ENTITY_NETHER);
        addDimension(ft::NEW_REGION_END,       ft::OLD_REGION_END,       ft::ENTITY_END);

        auto consoleWrite = settings.m_schematic.save_console;
        std::list<LCEFile> convertedFiles;

        for (auto& [newFmt, oldFmt, entityFmt, entityList] : dimensions) {

            // (3) collect old format
            std::list<LCEFile> oldRegionFiles = saveProject.collectFiles(oldFmt);

            // (4) place contents into old regions
            for (auto& oldRegionFile: oldRegionFiles) {

                Pos2D bigCoord = {
                        makeBigger(oldRegionFile.getRegionX()),
                        makeBigger(oldRegionFile.getRegionZ())
                };

                Region tinyRegions[4] = {
                    {bigCoord.x    , bigCoord.z    },
                    {bigCoord.x    , bigCoord.z + 1},
                    {bigCoord.x + 1, bigCoord.z    },
                    {bigCoord.x + 1, bigCoord.z + 1}
                };

                Region bigRegion;
                bigRegion.read(&oldRegionFile); // does this leak

                for (int sx = 0; sx < 32; ++sx) {
                    for (int sz = 0; sz < 32; ++sz) {
                        int rx = sx / 16;
                        int rz = sz / 16;
                        int cx = sx & 15;
                        int cz = sz & 15;

                        auto& tinyRegion = tinyRegions[rx * 2 + rz];

                        ChunkHandle handle;
                        if (!bigRegion.extractChunk(sx, sz, handle)) {
                            continue;
                        }

                        handle.decodeChunk(saveProject.m_stateSettings.console());
                        if (!handle.data->validChunk) continue;

                        ChunkData* data = handle.data.get();
                        settings.m_schematic.func_chunk_convert(handle, settings);

                        // move entities from the chunk, into the ``entityList``
                        if (!data->entities.empty()) {
                            // entityList.emplace_back(
                            //         Pos2D{data->chunkX, data->chunkZ},
                            //         makeCompound( { { "", makeList(std::move(data->entities)) } } )
                            // );
                            entityList.emplace_back(
                                    Pos2D{data->chunkX, data->chunkZ},
                                    makeList(std::move(data->entities))
                            );
                            data->entities = NBTList(eNBT::COMPOUND);
                        }
                        handle.encodeChunk(settings);

                        if (!tinyRegion.insertChunk(cx, cz, std::move(handle))) {
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
                    Buffer buffer = tinyRegion.write(settings);
                    if (buffer.empty()) continue;
                    // console, folder, region
                    auto& file = convertedFiles.emplace_back(
                            consoleWrite,
                            0,
                            saveProject.m_tempFolder,
                            saveProject.m_tempFolder,
                            ""
                    );
                    file.setType(newFmt);
                    file.setRegionX((i16)tinyRegion.x());
                    file.setRegionZ((i16)tinyRegion.z());
                    std::string fileName = file.constructFileName();
                    file.setFileName(fileName);
                    file.setBuffer(std::move(buffer));
                }
            }


            // save entities
            if (!entityList.empty() && !settings.removeEntities) {

                constexpr u32 timestamp = 0;
                auto& entityFile = convertedFiles.emplace_back(
                        consoleWrite,
                        timestamp,
                        saveProject.m_tempFolder,
                        saveProject.m_tempFolder,
                        ""
                );
                entityFile.setType(entityFmt);
                std::string fileName = entityFile.constructFileName();
                entityFile.setFileName(fileName);
                EntityFile::writeEntityList(entityFile, entityList);
                entityList.clear();
            }

        }

        saveProject.addFiles(std::move(convertedFiles));
    }


    void convertRegionsNew2Old(SaveProject& saveProject,
                                WriteSettings& settings) {

        std::vector<Pos2D> positions = {
                {-1, -1}, {-1, 0}, {0, -1}, {0, 0}
        };

        // IN:  -4 -3 -2 -1  0  1  2  3  4  5
        // OUT: -2 -2 -1 -1  0  0  1  1  2  2
        auto makeSmaller = [](int v) {
            return (v >= 0) ? v / 2 : (v - 1) / 2;
        };

        using ft = lce::FILETYPE;
        using Map = std::unordered_map<Pos2D, editor::Region, Pos2D::Hasher>;

        std::vector<std::tuple<ft, ft, ft, int, Map>> dimensions;
        dimensions.reserve(3);
        auto addDimension = [&](ft newFmt, ft oldFmt, ft entityFmt) {
            dimensions.emplace_back(newFmt, oldFmt, entityFmt, 27, Map{});
        };
        addDimension(ft::NEW_REGION_OVERWORLD, ft::OLD_REGION_OVERWORLD, ft::ENTITY_OVERWORLD);
        addDimension(ft::NEW_REGION_NETHER,    ft::OLD_REGION_NETHER,    ft::ENTITY_NETHER);
        addDimension(ft::NEW_REGION_END,       ft::OLD_REGION_END,       ft::ENTITY_END);


        saveProject.removeFileTypes({lce::FILETYPE::NEW_REGION_NETHER, lce::FILETYPE::NEW_REGION_NETHER});

        for (auto& [newFmt, oldFmt, entityFmt, regionScale, regionMap] : dimensions) {

            // (1) read entities file
            EntityFile::EntityMap entityMap;
            if (auto listFiles = saveProject.findFile(entityFmt); listFiles.has_value()) {
                const LCEFile& entityFile = listFiles->get();

                // ⬇️  grab the list from the helper
                auto entities = editor::EntityFile::readEntityList(entityFile);
                for (auto& [coord, nbt] : entities)
                    entityMap.emplace(coord, std::move(nbt));
            }

            // (2) build regions
            for (auto& pos : positions) {
                Pos2D key = pos;

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
                Pos2D tinyCoord(
                        makeSmaller(regionFile.getRegionX()),
                        makeSmaller(regionFile.getRegionZ())
                );
                if (tinyCoord.x < -1 || tinyCoord.x > 0 || tinyCoord.z < -1 || tinyCoord.z > 0) {
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

                        bool onLeft   = (bigRegion.x() == -1);
                        bool onRight  = (bigRegion.x() ==  0);
                        bool onTop    = (bigRegion.z() == -1);
                        bool onBottom = (bigRegion.z() ==  0);
                        int xMin = onLeft  ? 32 - regionScale : 0;
                        int xMax = onRight ? regionScale : 32;
                        int zMin = onTop    ? 32 - regionScale  : 0;
                        int zMax = onBottom ? regionScale : 32;
                        if (dx < xMin || dx >= xMax || dz < zMin || dz >= zMax) {
                            continue;
                        }

                        if (!inRange(sx, sz, bigRegion.m_regScale)){
                            continue;
                        }
                        if (!inRange(dx, dz, bigRegion.m_regScale)){
                            continue;
                        }
                        ChunkHandle handle;
                        if (!tinyRegion.extractChunk(sx, sz, handle)){
                            continue;
                        }

                        Pos2D realChunkCoord = {
                                bigRegion.x() * 32 + dx,
                                bigRegion.z() * 32 + dz
                        };


                        handle.decodeChunk(saveProject.m_stateSettings.console());
                        ChunkData* data = handle.data.get();

                        if (!data->validChunk) continue;

                        settings.m_schematic.func_chunk_convert(handle, settings);

                        auto entityIt = entityMap.extract(realChunkCoord);
                        if (!entityIt.empty()) {
                            if (data->validChunk) {

                                NBTBase& nbtRoot = entityIt.mapped();
                                if (nbtRoot("")) { nbtRoot = std::move(nbtRoot[""]); }

                                NBTList& entityList = nbtRoot.ensureList("Entities", eNBT::COMPOUND);
                                data->entities = std::move(entityList);
                            }
                        }

                        NBTFixer::fixEntities(data);
                        NBTFixer::fixTileEntities(data);

                        handle.encodeChunk(settings);

                        if (!bigRegion.insertChunk(dx, dz, std::move(handle))) {
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

        auto consoleWrite = settings.m_schematic.save_console;

        // create files from old regions
        std::list<LCEFile> convertedFiles;
        for (auto& [newFmt, oldFmt, entityFmt, regionScale, regionMap] : dimensions) {
            for (auto& [coord, region] : regionMap) {
                Buffer buffer = region.write(settings);
                if (buffer.empty()) continue;
                auto& file = convertedFiles.emplace_back(
                        consoleWrite,
                        0,
                        saveProject.m_tempFolder,
                        saveProject.m_tempFolder,
                        ""
                );
                file.setType(oldFmt);
                file.setRegionX((i16)coord.x);
                file.setRegionZ((i16)coord.z);
                std::string fileName = file.constructFileName();
                file.setFileName(fileName);
                file.setBuffer(std::move(buffer));
            }
        }

        saveProject.m_stateSettings.setNewGen(false);
        saveProject.addFiles(std::move(convertedFiles));
    }


    MU ND int preprocess(SaveProject& saveProject, StateSettings& stateSettings, WriteSettings& theWriteSettings) {
        if (!theWriteSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        std::cout << "Preprocessing savefile (removing things lol)" << std::endl;

        // TODO 7/10/25: i don't understand the below todo
        // TODO: create default output file path if not set
        auto tryRemove = [&](const std::string& sentenceTrue,
                             const std::string& sentenceFalse,
                             bool condition,
                             const std::set<lce::FILETYPE>& types) {
          if (condition) {
              if (!sentenceTrue.empty()) {
                  std::cout << sentenceTrue << std::endl;
              }
              saveProject.removeFileTypes(types);
          } else {
              if (!sentenceFalse.empty()) {
                  std::cout << sentenceFalse << std::endl;
              }
          }
        };

        using lce::FILETYPE;
        const bool diffConsoles = stateSettings.console() != theWriteSettings.m_schematic.save_console;

        tryRemove("[!] Config: removing files: GRF",
                  "",
                  diffConsoles, {lce::FILETYPE::GRF});
        tryRemove("[!] Config: removing files: players",
                  "",
                  theWriteSettings.removePlayers     || diffConsoles,
                  {FILETYPE::PLAYER});
        tryRemove("[!] Config: removing files: dataMapping",
                  "",
                  theWriteSettings.removeDataMapping || diffConsoles,
                  {FILETYPE::DATA_MAPPING});
        tryRemove("[!] Config: removing files: Maps",
                  "",
                  theWriteSettings.removeMaps,
                  {lce::FILETYPE::MAP});

        tryRemove("[!] Config: removing files: Structures",
                  "",
                  theWriteSettings.removeStructures,
                  {FILETYPE::STRUCTURE, FILETYPE::VILLAGE});

        tryRemove("[!] Config: removing files: Regions-Overworld",
                  "",
                  theWriteSettings.removeRegionsOverworld,
                  {FILETYPE::OLD_REGION_OVERWORLD, FILETYPE::NEW_REGION_OVERWORLD});

        tryRemove("[!] Config: removing files: Regions-Nether",
                  "",
                  theWriteSettings.removeRegionsNether,
                  {FILETYPE::OLD_REGION_NETHER, FILETYPE::NEW_REGION_NETHER});

        tryRemove("[!] Config: removing files: Regions-End",
                  "",
                  theWriteSettings.removeRegionsEnd,
                  {FILETYPE::OLD_REGION_END, FILETYPE::NEW_REGION_END});

        tryRemove("[!] Config: removing files: Entities(all dimensions)",
                  "",
                  theWriteSettings.removeEntities,
                  {FILETYPE::ENTITY_END, FILETYPE::ENTITY_NETHER, FILETYPE::ENTITY_OVERWORLD});


        return 0;
    }


    void convert(SaveProject& saveProject, WriteSettings& settings) {
        StateSettings& state = saveProject.m_stateSettings;

        c_auto inConsole = state.console();
        c_auto outConsole = settings.m_schematic.save_console;

        c_bool inNew = lce::isConsoleNewGen(inConsole);
        c_bool inOld = !inNew;
        c_bool outNew = lce::isConsoleNewGen(outConsole);
        c_bool outOld = !outNew;

        if (auto levelFileOpt = saveProject.findFile(lce::FILETYPE::LEVEL);
                levelFileOpt.has_value()) {
            LCEFile& file = levelFileOpt.value().get();
            editor::LevelFile lvl;
            lvl.read(file);
            if (inOld && outNew) {
                if (lvl.m_XZSize && lvl.m_XZSize != 54) {
                    lvl.m_HellScale = 3;
                    lvl.m_XZSize = 54;
                    lvl.m_StrongholdX = 0;
                    lvl.m_StrongholdZ = 0;
                    lvl.m_hasStrongholdEndPortal = 0;
                    lvl.m_StrongholdEndPortalX = 0;
                    lvl.m_StrongholdEndPortalZ = 0;
                }
            }
            lvl.write(file, outConsole, settings.m_schematic.save_tu);

            // Buffer buf(256);
            // for (int i = 0; i < buf.size(); i++) {
            //     buf.data()[0] = 255;
            // }
            // file.setBuffer(std::move(buf));

            file.m_console = outConsole;
        }

        if (inOld == outOld) {
            std::cout << "[-] rewriting all region chunks to adjust endian, "
                         "this may take a minute.\n";
        } else {
            std::cout << "[-] re-writing/ordering all region chunks, "
                         "this may take a minute.\n";
        }

        if (inOld && outOld) convertRegions<false>(saveProject, settings);
        else if (inNew && outOld) convertRegionsNew2Old(saveProject, settings);
        else if (inOld && outNew) convertRegionsOld2New(saveProject, settings);
        else if (inNew && outNew) convertRegions<true> (saveProject, settings);


        saveProject.setLatestVersion(settings.m_schematic.fileListing_latestVersion);
        saveProject.setOldestVersion(settings.m_schematic.fileListing_oldestVersion);
        saveProject.m_stateSettings.setConsole(settings.m_schematic.save_console);
    }
}
