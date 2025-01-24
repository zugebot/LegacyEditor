#pragma once

#include <cstring>

#include "include/lce/blocks/blockID.hpp"

#include "code/FileListing/fileListing.hpp"
#include "code/Region/RegionManager.hpp"


namespace editor {

    void processRegion(size_t regionIndex, FileListing& fileListing) {
        const lce::CONSOLE console = fileListing.myReadSettings.getConsole();
        if (regionIndex >= fileListing.ptrs.region_overworld.size()) { return; }

        // read a region file
        RegionManager region;
        region.read(fileListing.ptrs.region_overworld[regionIndex]);
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


                        /*
                    block1 = BlockID::AIR_ID;
                    if (y == 0) { block1 = BlockID::BEDROCK_ID; }

                    u32 jumpBL, jumpDA;

                    if (y == 1) {
                        if (x % 2 != 0 || z % 2 != 0) {
                            if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                                chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                                block1 = BlockID::AIR_ID;
                                goto END;
                            }
                            block1 = BlockID::BEDROCK_ID;
                            goto END;
                        }
                    }

                    if (y == 2) {
                        if (chunkData->chunkX < -16 || chunkData->chunkX > 16 ||
                            chunkData->chunkZ <   0 || chunkData->chunkZ >  1) {
                            goto END;
                        }

                        if (x % 2 != 0 || z % 2 != 0) {
                            goto END;
                        }

                        jumpBL = 8 * (chunkData->chunkX + 16);
                        jumpDA = 8 *  chunkData->chunkZ;

                        block1 = (jumpBL + x / 2) << 4;
                        data_1 = jumpDA + z / 2;

                        if (block1 > 252 << 4 || block1 == BlockID::BEACON_ID) {
                            block1 = 0;
                            data_1 = 0;
                        }
                    }

                    END:

                    blocks[offset1] = block1 | data_1;
                     */
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

        fileListing.ptrs.region_overworld[regionIndex]->data.deallocate();
        fileListing.ptrs.region_overworld[regionIndex]->data = region.write(console);
    }


    /**
     * Removes all blocks in the nether except for netherrack
     * DO NOT USE THIS IT NEEDS FIXED
     * @param regionIndex
     * @param fileListing
     */
    void removeNetherrack(size_t regionIndex, FileListing& fileListing) {
        const lce::CONSOLE console = fileListing.myReadSettings.getConsole();
        if (regionIndex >= fileListing.ptrs.region_nether.size()) { return; }

        // read a region file
        RegionManager region;
        region.read(fileListing.ptrs.region_nether[regionIndex]);

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

        fileListing.ptrs.region_nether[regionIndex]->data.deallocate();
        fileListing.ptrs.region_nether[regionIndex]->data = region.write(console);
    }


    /**
     * .
     *
     * @param regionIndex
     * @param fileListing
     */
    void convertChunksToAquatic(size_t regionIndex, FileList& fileList,
                                      const lce::CONSOLE inConsole, const lce::CONSOLE outConsole) {

        if (regionIndex >= fileList.size()) { return; }

        // read a region file
        RegionManager region;
        region.read(fileList[regionIndex]);

        for (auto & chunkManager : region.chunks) {
            chunkManager.readChunk(inConsole);
            if (!chunkManager.chunkData->validChunk) continue;

            // convert chunks to aquatic
            if (chunkManager.chunkData->lastVersion == 8 ||
                chunkManager.chunkData->lastVersion == 9 ||
                chunkManager.chunkData->lastVersion == 11) {
                chunkManager.chunkData->convertOldToAquatic();
            } else if (chunkManager.chunkData->lastVersion == 10) {
                chunkManager.chunkData->convertNBTToAquatic();
            } else if (chunkManager.chunkData->lastVersion == 13) {
                chunkManager.chunkData->convert114ToAquatic();
            }

            // there is probably a better way to go about this
            memset(chunkManager.chunkData->heightMap.data(), 0, 256);

            chunkManager.writeChunk(outConsole);
            chunkManager.ensureCompressed(outConsole);
        }

        fileList[regionIndex]->data.deallocate();
        fileList[regionIndex]->data = region.write(outConsole);
        fileList[regionIndex]->console = outConsole;
    }


}
