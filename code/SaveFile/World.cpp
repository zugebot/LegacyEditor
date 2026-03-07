// World.cpp
#include "World.hpp"

#include <tuple>

namespace {

    // IN & OUT assume DENOMINATOR is 2, but this is the general mapping.
    // IN:  -4 -3 -2 -1  0  1  2  3  4  5
    // OUT: -2 -2 -1 -1  0  0  1  1  2  2
    template<int DENOMINATOR = 2>
    int makeSmaller(const int v) {
        return (v >= 0) ? v / DENOMINATOR : (v - (DENOMINATOR - 1)) / DENOMINATOR;
    };

    int makeSmaller(const int v, int DENOMINATOR = 2) {
        return (v >= 0) ? v / DENOMINATOR : (v - (DENOMINATOR - 1)) / DENOMINATOR;
    };

    inline std::tuple<int, int, int, int> block_to_chunk_and_local(int x, int z) {
        const int cx = makeSmaller<16>(x);
        const int cz = makeSmaller<16>(z);
        // TODO: Black magic. or a bug in the code somewhere. who cares
        const int bx = z & 15;
        const int bz = x & 15;
        return {cx, cz, bx, bz};
    }

    inline bool region_has_dirty_chunks(const editor::Region& r) {
        for (const auto& h: r.m_handles)
            if (h.header.dirty()) return true;

        return false;
    }

    inline std::optional<lce::FILETYPE> dimensionToRegionFileType(const bool isNewGen, const lce::DIMENSION dim) {
        switch (dim) {
            case lce::DIMENSION::OVERWORLD:
                return isNewGen ? lce::FILETYPE::NEW_REGION_OVERWORLD
                                : lce::FILETYPE::OLD_REGION_OVERWORLD;
            case lce::DIMENSION::NETHER:
                return isNewGen ? lce::FILETYPE::NEW_REGION_NETHER
                                : lce::FILETYPE::OLD_REGION_NETHER;
            case lce::DIMENSION::END:
                return isNewGen ? lce::FILETYPE::NEW_REGION_END
                                : lce::FILETYPE::OLD_REGION_END;
            default:
                return std::nullopt;
        }
    }

} // namespace

namespace editor {

    void World::newWorld() {
        m_sp.m_stateSettings.setFilePath("");
        void newWorld();
    }

    int World::read(const fs::path& theFilePath) {
        m_regions.clear();
        return m_sp.read(theFilePath);
    }

    int World::write(WriteSettings& theWriteSettings) {
        flushOpenRegions(theWriteSettings);
        m_sp.m_stateSettings.setConsole(theWriteSettings.m_schematic.save_console);
        // TODO: this will not work for xbox360 bin for now
        auto it = makeParserForConsole(
                m_sp.m_stateSettings.console(),
                m_sp.m_stateSettings.isXbox360Bin()
        );
        it->m_filePath = m_sp.m_stateSettings.filePath();
        it->readFileInfo(m_sp);

        return m_sp.write(theWriteSettings);
    }

    WriteSettings World::write() {
        const editor::sch::Schematic* sch;

        const auto c = getConsole();
        if (lce::is_xbox1_family(c) || lce::is_switch_family(c)) {
            sch = &editor::sch::ElytraLatest;
        } else {
            sch = &editor::sch::AquaticTU69;
        }

        WriteSettings settings{*sch, getConsole()};


        if (lce::is_ps3_family(c)) {
            settings = WriteSettings(*sch, getConsole(), m_sp.m_stateSettings.getPC_PS3().value_or(editor::ePS3ProductCode::NONE), fs::path("out"));
        } else if (lce::is_psvita_family(c)) {
            settings = WriteSettings(*sch, getConsole(), m_sp.m_stateSettings.getPC_VITA().value_or(editor::eVITAProductCode::NONE), fs::path("out"));
        } else if (lce::is_ps4_family(c)) {
            settings = WriteSettings(*sch, getConsole(), m_sp.m_stateSettings.getPC_PS4().value_or(editor::ePS4ProductCode::NONE), fs::path("out"));
        }

        write(settings);
        return settings;
    }

    void World::markChunkDirty(const int cx, const int cz, const lce::DIMENSION dim) {
        const bool isNewGen = m_sp.m_stateSettings.isNewGen();
        const int regionSizeChunks = isNewGen ? 16 : 32;

        const int rx = makeSmaller(cx, regionSizeChunks);
        const int rz = makeSmaller(cz, regionSizeChunks);

        if (auto pair = getRegionPair({rx, rz}, dim)) {
            pair.value()->m_dirty = true;
        }
    }

    void World::flushOpenRegions(WriteSettings& settings) {
        for (auto& [key, pair] : m_regions) {
            if (!pair.m_file) continue;

            const bool needsFlush = pair.m_dirty || region_has_dirty_chunks(pair.m_region);
            if (!needsFlush) continue;

            Buffer buffer = pair.m_region.write(settings);
            pair.m_file->setBuffer(std::move(buffer));
            pair.m_dirty = false;
        }
        m_regions.clear();
    }

    std::set<lce::FILETYPE> World::dimensionToFileType(const lce::DIMENSION dim) const {
        const bool isNewGen = m_sp.m_stateSettings.isNewGen();

        switch (dim) {
            case lce::DIMENSION::OVERWORLD:
                return isNewGen ? std::set{lce::FILETYPE::NEW_REGION_OVERWORLD}
                                : std::set{lce::FILETYPE::OLD_REGION_OVERWORLD};
            case lce::DIMENSION::NETHER:
                return isNewGen ? std::set{lce::FILETYPE::NEW_REGION_NETHER}
                                : std::set{lce::FILETYPE::OLD_REGION_NETHER};
            case lce::DIMENSION::END:
                return isNewGen ? std::set{lce::FILETYPE::NEW_REGION_END}
                                : std::set{lce::FILETYPE::OLD_REGION_END};
            default:
                return {};
        }
    }

    std::optional<World::RegionFilePair*> World::getRegionPair(const Pos2D rPos, const lce::DIMENSION dim) {
        const RegionKey key{dim, rPos};

        if (auto it = m_regions.find(key); it != m_regions.end()) {
            return &it->second;
        }

        const auto fileSet = dimensionToFileType(dim);
        if (fileSet.empty()) return std::nullopt;

        for (LCEFile& file : m_sp.view_of(fileSet)) {
            if (file.getRegionX() == rPos.x && file.getRegionZ() == rPos.z) {
                Region region;
                region.read(&file);

                auto [it, inserted] = m_regions.emplace(key, RegionFilePair{&file, std::move(region), false});
                (void)inserted;
                return &it->second;
            }
        }

        const auto fileTypeOpt = dimensionToRegionFileType(m_sp.m_stateSettings.isNewGen(), dim);
        if (!fileTypeOpt) return std::nullopt;

        LCEFile& file = m_sp.emplaceFile(
                getConsole(),
                static_cast<u64>(std::time(nullptr)),
                m_sp.m_tempFolder,
                m_sp.m_tempFolder,
                ""
        );
        file.setType(fileTypeOpt.value());
        file.setRegionX(static_cast<i16>(rPos.x));
        file.setRegionZ(static_cast<i16>(rPos.z));
        file.setFileName(file.constructFileName());

        Region region(rPos.x, rPos.z, getConsole());
        auto [it, inserted] = m_regions.emplace(key, RegionFilePair{&file, std::move(region), true});
        (void)inserted;
        return &it->second;
    }

    std::optional<ChunkHandle*> World::getChunkByChunkCoord(const int cx, const int cz, const lce::DIMENSION dim) {
        const bool isNewGen = m_sp.m_stateSettings.isNewGen();
        const int regionSizeChunks = isNewGen ? 16 : 32;

        const int rx = makeSmaller(cx, regionSizeChunks);
        const int rz = makeSmaller(cz, regionSizeChunks);


        int cx_local = (cx - rx * regionSizeChunks); while (cx_local < 0) { cx_local += 32; }
        int cz_local = (cz - rz * regionSizeChunks); while (cz_local < 0) { cz_local += 32; }

        const auto pairOpt = getRegionPair({rx, rz}, dim);
        if (!pairOpt) return std::nullopt;

        RegionFilePair* pair = pairOpt.value();
        ChunkHandle* handle = pair->m_region.ensureChunk(cx_local, cz_local, m_defaultChunkVersion);
        if (!handle) return std::nullopt;

        return handle;
    }

    MU void World::setSubmerged(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block, lce::DIMENSION dim) {
        if (m_sp.latestVersion() < 12) {
            throw std::runtime_error("Error: Attempted to set submerged block in version below aquatic");
        }
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            h->data->setSubmerged(bx, yIn, bz, block);
            markChunkDirty(cx, cz, dim);
        }
    }

    MU u16 World::getSubmerged(c_i32 xIn, c_i32 yIn, c_i32 zIn, lce::DIMENSION dim) {
        if (m_sp.latestVersion() < 12) {
            throw std::runtime_error("Error: Attempted to get submerged block in version below aquatic");
        }
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            return h->data->getSubmerged(bx, yIn, bz);
        }
        return 0;
    }

    MU void World::setBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            h->data->setBlock(bx, yIn, bz, block);
            markChunkDirty(cx, cz, dim);
        }
    }

    MU u16 World::getBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            return h->data->getBlock(bx, yIn, bz);
        }
        return 0;
    }

    MU void World::setBlockLight(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u8 light, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            h->data->setBlockLight(bx, yIn, bz, light);
            markChunkDirty(cx, cz, dim);
        }
    }

    MU u8 World::getBlockLight(c_i32 xIn, c_i32 yIn, c_i32 zIn, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            return h->data->getBlockLight(bx, yIn, bz);
        }
        return 0;
    }

    MU void World::setSkyLight(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u8 light, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            h->data->setSkyLight(bx, yIn, bz, light);
            markChunkDirty(cx, cz, dim);
        }
    }

    MU u8 World::getSkyLight(c_i32 xIn, c_i32 yIn, c_i32 zIn, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            return h->data->getSkyLight(bx, yIn, bz);
        }
        return 0;
    }

    MU void World::addTileEntity(i32 xIn, i32 yIn, i32 zIn, NBTBase te, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            h->data->tileEntities.push_back(std::move(te));
        }
    }

    MU std::optional<NBTBase> World::getTileEntity(i32 xIn, i32 yIn, i32 zIn, lce::DIMENSION dim) {
        auto [cx, cz, bx, bz] = block_to_chunk_and_local(xIn, zIn);
        if (const auto handleOpt = getChunkByChunkCoord(cx, cz, dim)) {
            ChunkHandle* h = handleOpt.value();
            std::cerr << "World::getTileEntity is not yet implemented\n";
        }
        return std::nullopt;
    }

} // namespace editor
