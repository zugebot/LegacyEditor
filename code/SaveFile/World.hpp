// World.hpp
#pragma once

#include <map>
#include <optional>
#include <set>
#include <utility>

#include "SaveProject.hpp"
#include "code/Region/Region.hpp"
#include "common/Pos2DTemplate.hpp"

namespace editor {

    class World {
    private:
        struct RegionKey {
            lce::DIMENSION dim{};
            Pos2D pos{};

            bool operator<(const RegionKey& rhs) const {
                if (dim != rhs.dim) return static_cast<int>(dim) < static_cast<int>(rhs.dim);
                if (pos.x != rhs.pos.x) return pos.x < rhs.pos.x;
                return pos.z < rhs.pos.z;
            }
        };

        struct RegionFilePair {
            LCEFile* m_file = nullptr;
            Region   m_region;
            bool     m_dirty = false;
        };

    private:
        SaveProject m_sp{};
        std::map<RegionKey, RegionFilePair> m_regions;
        eChunkVersion m_defaultChunkVersion = V_NONE;

    public:
        explicit World(eChunkVersion defaultChunkVersion) : m_defaultChunkVersion(defaultChunkVersion) {}
        ~World() = default;

        void newWorld();
        int read(const fs::path& theFilePath);
        int write(WriteSettings& theWriteSettings);
        WriteSettings write();

        MU ND lce::CONSOLE getConsole() const { return m_sp.m_stateSettings.console(); }
        MU void printDetails() { m_sp.printDetails(); }

        MU void setWorldName(std::wstring worldNameIn) { m_sp.m_displayMetadata.worldName = std::move(worldNameIn); }
        MU ND std::wstring getWorldName() const { return m_sp.m_displayMetadata.worldName; }

        MU void setSubmerged(i32 xIn, i32 yIn, i32 zIn, u16 block, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);
        MU u16  getSubmerged(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);

        MU void setBlock(i32 xIn, i32 yIn, i32 zIn, u16 block, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);
        MU u16  getBlock(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);

        MU void setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);
        MU u8   getBlockLight(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);

        MU void setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);
        MU u8   getSkyLight(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);

        MU void addTileEntity(i32 xIn, i32 yIn, i32 zIn, NBTBase te, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);
        MU std::optional<NBTBase> getTileEntity(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim = lce::DIMENSION::OVERWORLD);

    private:
        void flushOpenRegions(WriteSettings& settings);

        ND std::set<lce::FILETYPE> dimensionToFileType(lce::DIMENSION dim) const;

        void markChunkDirty(int cx, int cz, lce::DIMENSION dim);
        std::optional<RegionFilePair*> getRegionPair(Pos2D rPos, lce::DIMENSION dim);
        std::optional<ChunkHandle*>    getChunkByChunkCoord(int cx, int cz, lce::DIMENSION dim);
    };

} // namespace editor
