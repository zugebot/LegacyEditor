#pragma once

#include "code/Region/ChunkManager.hpp"
#include "code/Region/Region.hpp"
#include "code/SaveFile/SaveProject.hpp"


namespace editor::summary {
    struct SaveSummary {
        fs::path m_savegamePath;
        NBTCompound m_summary = NBTCompound();
        NBTCompound m_level = NBTCompound();
        bool m_isValid = false;
    };


    using Row = std::map<std::string, std::string>;


    Row flatten(const NBTCompound& cmp) {
        Row out;
        for (const auto& [k, v] : cmp)
            out[k] = v.to_string_shallow();
        return out;
    }


    MU SaveSummary createSummary(const fs::path& savegamePath) {
        SaveSummary retSummary;
        retSummary.m_savegamePath = savegamePath;

        editor::SaveProject saveProject;
        if (saveProject.read(retSummary.m_savegamePath.string()) != 0) {
            return {};
        }

        editor::ChunkManager* chunk = nullptr;
        editor::Region regionManager;

        for (auto& regionFile : saveProject.view_of(lce::FILETYPE::OLD_REGION_ANY)) {
            regionManager.read(&regionFile);
            chunk = regionManager.getNonEmptyChunk();
            if (chunk != nullptr) { break; }
        }

        if (chunk == nullptr) {
            return {};
        }

        c_bool chunkIsZIP = chunk->chunkHeader.isZipCompressed();
        c_bool chunkIsRLE = chunk->chunkHeader.isRLECompressed();
        c_bool chunkIsNEW = chunk->chunkHeader.getNewSaveFlag();

        chunk->readChunk(lce::CONSOLE::XBOX360);
        editor::chunk::ChunkData* chunkData = chunk->chunkData;

        // eBlockOrder blockOrder = guessOrder<u16, 16>(chunkData->blocks, chunkData->heightMap);

        if (auto levelDat = saveProject.findFile(lce::FILETYPE::LEVEL);
            levelDat) {
            Buffer buffer = levelDat->get().getBuffer();
            DataReader reader(buffer);

            NBTCompound summary;
            summary.insert("_TU", makeString(""));
            summary.insert("_SP_Oldest", makeInt(saveProject.oldestVersion()));
            summary.insert("_SP_Latest", makeInt(saveProject.latestVersion()));
            summary.insert("_C_Version", makeInt(chunkData->lastVersion));
            summary.insert("_C_Height", makeInt(chunkData->chunkHeight));
            summary.insert("_C_isZIP", makeByte(chunkIsZIP));
            summary.insert("_C_isRLE", makeByte(chunkIsRLE));
            summary.insert("_C_isNEW", makeByte(chunkIsNEW));

            if (chunk->chunkData->intel.wasNBTChunk) {
                if (!chunk->chunkData->intel.hasBiomes) {
                    summary.insert("_C_MissingBiomes", makeByte(1));
                }
                if (chunk->chunkData->intel.hasTerraFlagVariant) {
                    summary.insert("_C_NBTTerraFlagVariant", makeByte(1));
                } else {
                    summary.insert("_C_NBTTerraFlagVariant", makeByte(0));
                }
            }
            retSummary.m_summary = std::move(summary);

            NBTBase nbt = NBTBase::read(reader);
            auto level = nbt[""]["Data"];
            retSummary.m_level = level->get<NBTCompound>();
        }

        retSummary.m_isValid = true;

        // 9. XYZ format of blocks
        // 10. XYZ format of block data
        // 11. XYZ format of block/skylight
        // 12. whether there is biomes in chunkdata

        return retSummary;
    }


    // build & print table of data :fire:
    MU void print_table(const std::vector<SaveSummary>& summaries,
                        std::ostream& out = std::cout) {

        // collect headers
        std::vector<std::string> headerKeysMain;
        std::vector<std::string> headerKeysLevel;
        std::vector<std::string> headerKeysSummary;
        std::map<std::string, std::string> headerTypes;
        std::unordered_set<std::string> seen;

        std::vector<Row> rows;
        rows.reserve(summaries.size());

        for (auto&& summary : summaries) {
            std::array<const NBTCompound*,2> container = {
                    &summary.m_summary, &summary.m_level };
            NBTCompound all;
            for (const auto& nbt : container) {
                for (const auto& [key, value] : *nbt) {
                    all.insert(key, value);
                    if (auto it = headerTypes.find(key); it == headerTypes.end()) {
                        if (nbt == &summary.m_summary) {
                            headerKeysSummary.push_back(key);
                        } else {
                            headerKeysLevel.push_back(key);
                        }
                        headerTypes[key] = to_string(value.getType());
                    }
                }
            }
            Row r = flatten(all);
            rows.push_back(r);
        }

        // create headerKeysMain
        for (const auto& key : headerKeysSummary) {
            headerKeysMain.push_back(key);
        }
        std::sort(headerKeysLevel.begin(), headerKeysLevel.end());
        for (const auto& key : headerKeysLevel) {
            headerKeysMain.push_back(key);
        }

        // compute widths
        std::vector<std::size_t> colw(headerKeysMain.size());
        for (std::size_t i = 0; i < headerKeysMain.size(); ++i) {
            colw[i] = headerKeysMain[i].size();
        }

        for (auto&& r : rows) {
            for (std::size_t i = 0; i < headerKeysMain.size(); ++i) {
                colw[i] = std::max(colw[i], r[headerKeysMain[i]].size());
                colw[i] = std::max(colw[i], headerTypes[headerKeysMain[i]].size());
            }
        }

        auto rule = [&]() {
            out << "┼";
            for (std::size_t i = 0; i < headerKeysMain.size(); ++i) {
                const std::string wide = "─";
                std::string line;
                for (std::size_t j = 0; j < colw[i] + 2; ++j)
                    line += wide;
                out << line << "┼";
            }
            out << '\n';
        };

        // header
        rule();
        out << "│";
        for (std::size_t i = 0; i < headerKeysMain.size(); ++i) {
            out << ' ' << std::setw((int)colw[i]) << std::right << headerKeysMain[i] << " │";
        }
        out << '\n';

        // type
        out << "│";
        for (std::size_t i = 0; i < headerKeysMain.size(); ++i) {
            const auto& key = headerKeysMain[i];
            auto it =  headerTypes.find(key);
            auto type = it != headerTypes.end() ? it->second : "";
            out << ' ' << std::setw((int)colw[i]) << std::right << type << " │";
        }
        out << '\n';
        rule();

        // rows
        for (auto & row : rows) {
            out << "│";
            for (std::size_t c = 0; c < headerKeysMain.size(); ++c) {
                const auto& key = headerKeysMain[c];
                auto it = row.find(key);
                const std::string& cell = (it != row.end()) ? it->second : "";
                out << ' ' << std::setw((int)colw[c]) << std::right << cell << " │";
            }
            out << '\n';
        }
        rule();
    }
}