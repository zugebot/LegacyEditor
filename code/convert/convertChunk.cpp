#include "convertChunk.hpp"


#include "include/lce/processor.hpp"

#include "include/lce/blocks/blockID.hpp"

#include "code/SaveFile/writeSettings.hpp"
#include "code/chunk/chunkHandle.hpp"

#include "code/Impl/levelFile.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/chunk/helpers/NBTFixer.hpp"

#include "code/Region/Region.hpp"
#include "lce/blocks/downgrade.hpp"
#include "lce/registry/blockRegistry.hpp"


namespace editor {


    void downdateBlocks(ChunkHandle& handle, WriteSettings& settings) {
        auto* chunkData = handle.data.get();

        // Build the one-shot (id,data) -> final (id,data) map for this target TU.
        // TIP: if you convert many chunks with the same TU, cache this outside the function.
        const auto finalMap =
                lce::compat::build_final_map_for_TU(settings.m_schematic.save_tu.value());

        for (int i = 0; i < 65536; ++i) {
            u16 raw   = chunkData->blocks[i];
            u16 id    = static_cast<u16>((raw & 0x1FF0) >> 4); // 9-bit ID
            u8  meta  = static_cast<u8 >(raw & 0x0F);          // 4-bit data

            if (id < lce::compat::FinalMap::MAX_ID) {
                lce::BlockState in{ id, meta };
                lce::BlockState out = lce::compat::map_state(finalMap, in);

                // If you prefer COBBLESTONE over AIR for "no-rule" cases, uncomment:
                // if (out == lce::BlocksInit::AIR.getState()) {
                //     out = { lce::blocks::COBBLESTONE_ID, 0 };
                // }

                raw = static_cast<u16>((out.getID() << 4) | (out.getDataTag() & 0x0F));
                chunkData->blocks[i] = raw;
            } else {
                // ID outside table range — keep as-is (matches previous behavior),
                // or force a fallback:
                chunkData->blocks[i] = static_cast<u16>(lce::blocks::COBBLESTONE_ID << 4);
            }
        }
    }





    void convertReadChunkToAquatic(ChunkHandle& handle, WriteSettings& settings) {
        ChunkData* chunkData = handle.data.get();
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

        } else if (chunkData->lastVersion == 12 ||
                chunkData->lastVersion == 13) {

            // for (int i = 0; i < 65536; i++) {
            //     c_u16 id1 = chunkData->blocks[i] >> 4 & 1023;
            //     if ((id1 > 259 && id1 < 263) || id1 > 318) {
            //         chunkData->blocks[i] = lce::blocks::COBBLESTONE_ID << 4;
            //     }
            //     continue;
            //     c_u16 id2 = chunkData->submerged[i] >> 4 & 1023;
            //     if (id2 > 250) {
            //         chunkData->submerged[i] = lce::blocks::COBBLESTONE_ID << 4;
            //     }
            // }

            downdateBlocks(handle, settings);
        }

        handle.header.setNewSave((bool)settings.m_schematic.chunk_isNewSave);
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }


    void convertReadChunkToElytra(ChunkHandle& handle, WriteSettings& settings) {
        auto chunkData = handle.data.get();

        downdateBlocks(handle, settings);

        // if (chunkData->lastVersion == 7) {
        // } else if (chunkData->lastVersion == 8 ||
        //            chunkData->lastVersion == 9 ||
        //            chunkData->lastVersion == 10 ||
        //            chunkData->lastVersion == 11) {
        // }
        // else if (chunkData->lastVersion == 12 ||
        //          chunkData->lastVersion == 13) {
        //     for (int i = 0; i < 65536; i++) {
        //         u16 block = chunkData->blocks[i];
        //         if (((block & 0x1FF0) >> 4) > 255) {
        //             block = lce::blocks::COBBLESTONE_ID << 4;
        //         }
        //         chunkData->blocks[i] = block;
        //     }
        // }


        handle.header.setNewSave((bool)settings.m_schematic.chunk_isNewSave);
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }



    void convertReadChunkToPotions(ChunkHandle& handle, WriteSettings& settings) {
        auto* chunkData = handle.data.get();

        downdateBlocks(handle, settings);

        chunkData->entities.clear();
        chunkData->tileEntities.clear();
        chunkData->tileTicks.clear();

        // chunkData->terrainPopulatedFlags = 2046;
        handle.header.setNewSave(static_cast<bool>(settings.m_schematic.chunk_isNewSave));
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }



    /*
    void convertReadChunkToPotions(ChunkHandle& handle, WriteSettings& settings) {
        auto chunkData = handle.data.get();

        bool allowed[512];
        std::fill(std::begin(allowed), std::end(allowed), false);

        lce::registry::BlockRegistry::forEachBlock([&allowed, settings](const lce::Block* b){
            if (b->getTU() <= settings.m_schematic.save_tu.value()) {
                int id = b->getID();
                if (id >= 0 && id < 512) {
                    allowed[id] = true;
                }
            }
        });

        for (int i = 0; i < 65536; i++) {
            u16 block = chunkData->blocks[i];
            if ( !allowed[(block & 0x1FF0) >> 4] ) {
                block = lce::blocks::COBBLESTONE_ID << 4;
            }

            chunkData->blocks[i] = block;
        }

        chunkData->entities.clear();
        chunkData->tileEntities.clear();
        chunkData->entities.clear();


        handle.header.setNewSave((bool)settings.m_schematic.chunk_isNewSave);
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }*/




}
