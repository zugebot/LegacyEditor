#pragma once

#include "code/Region/Region.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/chunk/chunkHandle.hpp"


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


    MU SaveSummary createSummary(lce::CONSOLE console, const fs::path& savegamePath) {
        SaveSummary retSummary;
        retSummary.m_savegamePath = savegamePath;

        editor::SaveProject saveProject;
        if (saveProject.read(retSummary.m_savegamePath.string()) != 0) {
            return {};
        }

        editor::ChunkHandle* chunk = nullptr;
        editor::Region regionManager;


        auto findFirstValidChunk = [&](lce::FILETYPE regionType)
                -> editor::ChunkHandle* {
            for (auto& regionFile : saveProject.view_of(regionType)) {
                regionManager.read(&regionFile);
                for (auto& handle : regionManager.m_handles) {
                    if (handle.buffer.empty())
                        continue;
                    handle.decodeChunk(console);
                    if (handle.data->lastVersion != 0) {
                        return &handle;
                    }
                }
            }
            return nullptr;
        };

        chunk = findFirstValidChunk(lce::FILETYPE::OLD_REGION_ANY);
        if (chunk == nullptr) {
            chunk = findFirstValidChunk(lce::FILETYPE::NEW_REGION_ANY);
            if (chunk == nullptr) {
                return {};
            }
        }


        c_bool chunkIsRLE = chunk->header.isRLE;
        c_bool chunkIsNEW = chunk->header.isNewSave;

        // chunk->decodeChunk(lce::CONSOLE::SWITCH);
        auto* chunkData = chunk->data.get();

        // eBlockOrder blockOrder = guessOrder<u16, 16>(chunkData->blocks, chunkData->heightMap);

        if (auto levelDat = saveProject.findFile(lce::FILETYPE::LEVEL);
            levelDat) {
            Buffer buffer = levelDat->get().getBuffer();
            DataReader reader(buffer);

            NBTCompound summary;


            if (lce::is_xbox360_family(console)) {
                summary.insert("_TU", makeString(""));

            } else if (lce::is_wiiu_family(console)) {
                summary.insert("_BUILD", makeString(""));

            } else {
                throw std::runtime_error("createSummary was not designed to use" + lce::consoleToStr(console));
            }

            summary.insert("_SP_Oldest", makeInt(saveProject.oldestVersion()));
            summary.insert("_SP_Latest", makeInt(saveProject.latestVersion()));
            summary.insert("_SP_EXTRADATA", makeString(saveProject.m_displayMetadata.extraData));
            summary.insert("_C_Version", makeInt(chunkData->lastVersion));
            summary.insert("_C_Height", makeInt(chunkData->chunkHeight));
            summary.insert("_C_isRLE", makeByte(chunkIsRLE));
            summary.insert("_C_isNEW", makeByte(chunkIsNEW));

            if (chunk->data->intel.wasNBTChunk) {
                if (!chunk->data->intel.hasBiomes) {
                    summary.insert("_C_MissingBiomes", makeByte(1));
                }
                if (chunk->data->intel.hasTerraFlagVariant) {
                    summary.insert("_C_NBTTerraFlagVariant", makeByte(1));
                } else {
                    summary.insert("_C_NBTTerraFlagVariant", makeByte(0));
                }
            }
            retSummary.m_summary = std::move(summary);

            NBTBase nbt = NBTBase::read(reader);
            auto level = nbt[""]["Data"];
            retSummary.m_level = level.get<NBTCompound>();
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