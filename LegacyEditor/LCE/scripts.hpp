#pragma once

#include <cstring>
#include "LegacyEditor/LCE/Chunk/modifiers.hpp"
#include "LegacyEditor/LCE/FileListing/fileListing.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"


namespace editor {

    void processRegion(int regionIndex, FileListing& fileListing) {
        const CONSOLE console = fileListing.console;
        if (regionIndex >= fileListing.region_overworld.size()) { return; }

        // read a region file
        RegionManager region(console);
        region.read(fileListing.region_overworld[regionIndex]);

        int h = -1;

        for (ChunkManager& chunkManager : region.chunks) {
            h++;
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
                        u16 block1 = getBlock(chunkManager.chunkData, x, y, z);
                        u16 data_1 = 0;
                        const int offset1 = y + 256 * z + 4096 * x;

                        const u16 compare1 = (block1 & 0x1FF0) >> 4;
                        if ((block1 & 0x8000) != 0) { // fix stupid blocks
                            if (compare1 == 271) { // sea pickle
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

            memcpy(&chunkData->newBlocks[0], &blocks[0], 131072);
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

        fileListing.region_overworld[regionIndex]->data.deallocate();
        fileListing.region_overworld[regionIndex]->data = region.write(console);
    }



    /**
     * Removes all blocks in the nether except for netherrack
     * @param regionIndex
     * @param fileListing
     */
    void removeNetherrack(int regionIndex, FileListing& fileListing) {
        const CONSOLE console = fileListing.console;
        if (regionIndex >= fileListing.region_nether.size()) { return; }

        // read a region file
        RegionManager region(console);
        region.read(fileListing.region_nether[regionIndex]);

        for (ChunkManager& chunkManager : region.chunks) {
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
                        u16 block1 = getBlock(chunkManager.chunkData, x, y, z);
                        const int offset1 = y + 256 * z + 4096 * x;

                        if ((block1 & 0x1FF0) >> 4 != 7) {
                            block1 = 0;
                        }

                        blocks[offset1] = block1;
                    }
                }
            }

            memcpy(chunkData->newBlocks.data(), &blocks[0], 131072);
            memset(chunkData->blockLight.data(), 0xFF, 32768);
            memset(chunkData->skyLight.data(), 0xFF, 32768);
            chunkData->terrainPopulated = 2046;

            chunkData->defaultNBT();
            chunkManager.writeChunk(console);
            chunkManager.ensureCompressed(console);
        }

        fileListing.region_nether[regionIndex]->data.deallocate();
        fileListing.region_nether[regionIndex]->data = region.write(console);
    }



    /**
     * Removes all blocks in the nether except for netherrack
     * @param regionIndex
     * @param fileListing
     */
    void convertElytraToAquaticChunks(int regionIndex, FileListing& fileListing) {
        const CONSOLE console = fileListing.console;
        if (regionIndex >= fileListing.region_nether.size()) { return; }

        // read a region file
        RegionManager region(console);
        region.read(fileListing.region_nether[regionIndex]);

        for (int index = 0; index < 1024; index++) {
            ChunkManager *chunkManager = &region.chunks[index];

            if (chunkManager->size == 0) {
                continue;
            }

            chunkManager->ensureDecompress(console);
            chunkManager->readChunk(console);
            auto* chunkData = chunkManager->chunkData;

            if (!chunkData->validChunk) {
                continue;
            }

            convertOldToNew(chunkData);


            chunkData->defaultNBT();
            chunkManager->writeChunk(console);
            chunkManager->ensureCompressed(console);
        }

        fileListing.region_nether[regionIndex]->data.deallocate();
        fileListing.region_nether[regionIndex]->data = region.write(console);
    }


    /**
     * Removes all blocks in the nether except for netherrack
     * @param regionIndex
     * @param fileListing
     */
    void ConvertPillagerToAquaticChunks(const int regionIndex, const FileListing& fileListing) {
        const CONSOLE console = fileListing.console;
        if (regionIndex >= fileListing.region_overworld.size()) { return; }

        // read a region file
        RegionManager region(console);
        region.read(fileListing.region_overworld[regionIndex]);

        for (auto & chunk : region.chunks) {
            ChunkManager *chunkManager = &chunk;
            if (chunkManager->size == 0) {
                continue;
            }
            chunkManager->ensureDecompress(console);
            chunkManager->readChunk(console);
            auto* chunkData = chunkManager->chunkData;
            if (!chunkData->validChunk) {
                continue;
            }

            // remove 1.14 blocks and items here...
            for (int i = 0; i < 65536; i++) {
                const u16 id = chunkData->newBlocks[i] >> 4 & 1023;
                if (id > 318) {
                    chunkData->newBlocks[i] = 0;
                }
            }

            chunkData->defaultNBT();
            chunkManager->writeChunk(console);
            chunkManager->ensureCompressed(console);
        }

        fileListing.region_overworld[regionIndex]->data.deallocate();
        fileListing.region_overworld[regionIndex]->data = region.write(console);
    }





}
