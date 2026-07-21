// convertChunk.cpp
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

    static inline ChunkData* touchData(ChunkHandle& handle) {
        if (handle.state() == ChunkState::EMPTY) return nullptr;
        return handle.data.operator->(); // auto-decodes and marks dirty
    }

    void downdateBlocks(ChunkHandle& handle, WriteSettings& settings) {
        ChunkData* chunkData = touchData(handle);
        if (!chunkData) return;

        const int tu = settings.m_schematic.save_tu.value();
        const lce::compat::FinalMap& finalMap = lce::compat::final_map(tu);

        for (int i = 0; i < 65536; ++i) {
            u16 raw  = chunkData->blocks[i];
            u16 id   = static_cast<u16>((raw & 0x1FF0) >> 4);
            u8  meta = static_cast<u8>(raw & 0x0F);

            if (id < lce::compat::FinalMap::MAX_ID) {
                lce::BlockState in{ id, meta };
                lce::BlockState out = lce::compat::map_state(finalMap, in);

                raw = static_cast<u16>((out.getID() << 4) | (out.getDataTag() & 0x0F));
                chunkData->blocks[i] = raw;
            } else {
                chunkData->blocks[i] = static_cast<u16>(lce::blocks::COBBLESTONE_ID << 4);
            }
        }
    }

    void convertReadChunkToAquatic(ChunkHandle& handle, WriteSettings& settings) {
        ChunkData* chunkData = touchData(handle);
        if (!chunkData) return;

        if (chunkData->lastVersion == 7) {
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
            downdateBlocks(handle, settings);
        }

        handle.header.setNewSave(static_cast<bool>(settings.m_schematic.chunk_isNewSave));
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }

    void convertReadChunkToElytra(ChunkHandle& handle, WriteSettings& settings) {
        ChunkData* chunkData = touchData(handle);
        if (!chunkData) return;

        downdateBlocks(handle, settings);

        handle.header.setNewSave(static_cast<bool>(settings.m_schematic.chunk_isNewSave));
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }

    void convertReadChunkToPotions(ChunkHandle& handle, WriteSettings& settings) {
        ChunkData* chunkData = touchData(handle);
        if (!chunkData) return;

        downdateBlocks(handle, settings);

        chunkData->entities.clear();
        chunkData->tileEntities.clear();
        chunkData->tileTicks.clear();

        handle.header.setNewSave(static_cast<bool>(settings.m_schematic.chunk_isNewSave));
        chunkData->lastVersion = settings.m_schematic.chunk_lastVersion;
    }

} // namespace editor
