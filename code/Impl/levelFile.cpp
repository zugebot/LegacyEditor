#include "levelFile.hpp"

#include "common/nbt.hpp"
#include "code/LCEFile/LCEFile.hpp"


template<>
std::optional<editor::enums::GameType>
NBTBase::value<editor::enums::GameType>(const std::string& key) const {
    using GT = editor::enums::GameType;

    static const std::array<GT, 3> table = {
            GT::survival,
            GT::creative,
            GT::adventure,
            // GT::spectator
    };

    auto idxOpt = this->value<int>(key);
    if (!idxOpt) return std::nullopt;

    auto idx = static_cast<std::size_t>(idxOpt.value());
    if (idx >= table.size()) return std::nullopt;

    return table[idx];
}


template<>
std::optional<editor::enums::Difficulty>
NBTBase::value<editor::enums::Difficulty>(const std::string& key) const {
    using Diff = editor::enums::Difficulty;

    static std::array<Diff, 4> table = {
            Diff::peaceful,
            Diff::easy,
            Diff::medium,
            Diff::hard,
    };

    auto idxOpt = this->value<int>(key);
    if (!idxOpt) return std::nullopt;

    auto idx = static_cast<std::size_t>(idxOpt.value());
    if (idx >= table.size()) return std::nullopt;

    return table[idx];
}


template<>
std::optional<editor::enums::BiomeScale>
NBTBase::value<editor::enums::BiomeScale>(const std::string& key) const {
    using BS = editor::enums::BiomeScale;

    static const std::array<BS, 3> table = {
            BS::small,
            BS::medium,
            BS::large
    };

    auto idxOpt = this->value<int>(key);
    if (!idxOpt) return std::nullopt;

    auto idx = static_cast<std::size_t>(idxOpt.value());
    if (idx >= table.size()) return std::nullopt;

    return table[idx];
}


namespace editor {


    MU void LevelFile::resetSettings() {
        m_BiomeCentreXChunk = std::nullopt;
        m_BiomeCentreZChunk = std::nullopt;
        m_BiomeScale = std::nullopt;
        m_DataVersion = std::nullopt;
        m_DayTime = std::nullopt;
        m_Difficulty = std::nullopt;
        m_DifficultyLocked = std::nullopt;
        m_GameType = std::nullopt;
        m_HellScale = std::nullopt;
        m_LastPlayed = std::nullopt;
        m_LevelName = std::nullopt;
        m_MapFeatures = std::nullopt;
        m_ModernEnd = std::nullopt;
        m_RandomSeed = std::nullopt;
        m_SizeOnDisk = std::nullopt;
        m_SpawnX = std::nullopt;
        m_SpawnY = std::nullopt;
        m_SpawnZ = std::nullopt;
        m_StrongholdEndPortalX = std::nullopt;
        m_StrongholdEndPortalZ = std::nullopt;
        m_StrongholdX = std::nullopt;
        m_StrongholdY = std::nullopt;
        m_StrongholdZ = std::nullopt;
        m_Time = std::nullopt;
        m_allowCommands = std::nullopt;
        m_generatorName = std::nullopt;
        m_generatorVersion = std::nullopt;
        m_hardcore = std::nullopt;
        m_hasBeenInCreative = std::nullopt;
        m_hasStronghold = std::nullopt;
        m_hasStrongholdEndPortal = std::nullopt;
        m_initialized = std::nullopt;
        m_newSeaLevel = std::nullopt;
        m_rainTime = std::nullopt;
        m_raining = std::nullopt;
        m_spawnBonusChest = std::nullopt;
        m_thunderTime = std::nullopt;
        m_thundering = std::nullopt;
        m_version = std::nullopt;
        m_ClassicMoat = std::nullopt;
        m_SmallMoat = std::nullopt;
        m_MediumMoat = std::nullopt;
    }


    MU void LevelFile::defaultSettings() {
        m_BiomeCentreXChunk = 0;
        m_BiomeCentreZChunk = 0;
        m_BiomeScale = enums::BiomeScale::small;
        m_DataVersion = 0;
        m_DayTime = 0;
        m_Difficulty = enums::Difficulty::easy;
        m_DifficultyLocked = 0;
        m_GameType = enums::GameType::survival;
        m_HellScale = 3;
        m_LastPlayed = 0;
        m_LevelName = "world";
        m_MapFeatures = 1;
        m_ModernEnd = 0;
        m_RandomSeed = 12345;
        m_SizeOnDisk = 0;
        m_SpawnX = 0;
        m_SpawnY = 64;
        m_SpawnZ = 0;
        m_StrongholdEndPortalX = 0;
        m_StrongholdEndPortalZ = 0;
        m_StrongholdX = 0;
        m_StrongholdY = 0;
        m_StrongholdZ = 0;
        m_Time = 0;
        m_allowCommands = 0;
        m_generatorName = "default";
        m_generatorVersion = 1;
        m_hardcore = 0;
        m_hasBeenInCreative = 0;
        m_hasStronghold = 0;
        m_hasStrongholdEndPortal = 0;
        m_initialized = 1;
        m_newSeaLevel = 1;
        m_rainTime = 120000;
        m_raining = 0;
        m_spawnBonusChest = 0;
        m_thunderTime = 120000;
        m_thundering = 0;
        m_version = 19132;

        m_ClassicMoat = 0;
        m_SmallMoat = 0;
        m_MediumMoat = 0;
    }


    MU void LevelFile::read(const LCEFile& levelFile) {
        Buffer buffer = levelFile.getBuffer();
        if (buffer.empty()) {
            return;
        }

        DataReader reader(buffer.data(), buffer.size(), Endian::Big);
        NBTBase root = NBTBase::read(reader);

        const auto& data = root("")("Data");

        auto grab = [&data](auto& member, const char* key) {
            using Opt          = std::decay_t<decltype(member)>; // e.g. std::optional<int>
            using Val          = typename Opt::value_type;       // e.g. int
            auto ref           = data.template value<Val>(key);  // optional_ref<Val>

            if (!ref) return;

            if constexpr (std::is_same_v<Val, NBTBase>) {
                member.emplace(ref->get().copy());
            } else {
                member.emplace(*ref);
            }
        };

        grab(m_BiomeCentreXChunk, "BiomeCentreXChunk");
        grab(m_BiomeCentreZChunk, "BiomeCentreZChunk");
        grab(m_BiomeScale, "BiomeScale");
        grab(m_DataVersion, "DataVersion");
        grab(m_DayTime, "DayTime");
        grab(m_Difficulty, "Difficulty");
        grab(m_DifficultyLocked, "DifficultyLocked");
        grab(m_DimensionData, "DimensionData");
        grab(m_GameType, "GameType");
        grab(m_HellScale, "HellScale");
        grab(m_LastPlayed, "LastPlayed");
        grab(m_LevelName, "LevelName");
        grab(m_MapFeatures, "MapFeatures");
        grab(m_ModernEnd, "ModernEnd");
        grab(m_RandomSeed, "RandomSeed");
        grab(m_SizeOnDisk, "SizeOnDisk");
        grab(m_SpawnX, "SpawnX");
        grab(m_SpawnY, "SpawnY");
        grab(m_SpawnZ, "SpawnZ");
        grab(m_StrongholdEndPortalX, "StrongholdEndPortalX");
        grab(m_StrongholdEndPortalZ, "StrongholdEndPortalZ");
        grab(m_StrongholdX, "StrongholdX");
        grab(m_StrongholdY, "StrongholdY");
        grab(m_StrongholdZ, "StrongholdZ");
        grab(m_Time, "Time");
        grab(m_XZSize, "XZSize");
        grab(m_allowCommands, "allowCommands");
        grab(m_clearWeatherTime, "clearWeatherTime");
        grab(m_generatorName, "generatorName");
        grab(m_generatorOptions, "generatorOptions");
        grab(m_generatorVersion, "generatorVersion");
        grab(m_hardcore, "hardcore");
        grab(m_hasBeenInCreative, "hasBeenInCreative");
        grab(m_hasStronghold, "hasStronghold");
        grab(m_hasStrongholdEndPortal, "hasStrongholdEndPortal");
        grab(m_initialized, "initialized");
        grab(m_newSeaLevel, "newSeaLevel");
        grab(m_rainTime, "rainTime");
        grab(m_raining, "raining");
        grab(m_spawnBonusChest, "spawnBonusChest");
        grab(m_thunderTime, "thunderTime");
        grab(m_thundering, "thundering");
        grab(m_version, "version");
        grab(m_ClassicMoat, "ClassicMoat");
        grab(m_SmallMoat, "SmallMoat");
        grab(m_MediumMoat, "MediumMoat");
    }


    MU void LevelFile::write(LCEFile& levelFile, const lce::CONSOLE console, const TU& tu) {
        NBTCompound data;
        // data["cloudHeight"] = makeInt(65);

        data["Time"] = makeLong(m_Time.value_or(0));
        data["LastPlayed"] = makeLong(m_LastPlayed.value_or(0));
        data["LevelName"] = makeString(m_LevelName.value_or("world"));
        data["RandomSeed"] = makeLong(m_RandomSeed.value_or(0));
        data["SizeOnDisk"] = makeLong(m_SizeOnDisk.value_or(100000));
        data["SpawnX"] = makeInt(m_SpawnX.value_or(0));
        data["SpawnY"] = makeInt(m_SpawnY.value_or(64));
        data["SpawnZ"] = makeInt(m_SpawnZ.value_or(0));
        data["rainTime"] = makeInt(m_rainTime.value_or(120000));
        data["raining"] = makeByte(m_raining.value_or(0));
        data["thunderTime"] = makeInt(m_thunderTime.value_or(120000));
        data["thundering"] = makeByte(m_thundering.value_or(0));
        data["version"] = makeInt(m_version.value_or(19132));

        if (lce::isConsoleNewGen(console)) {
            data["ClassicMoat"] = makeInt(m_ClassicMoat.value_or(0));
            data["SmallMoat"] = makeInt(m_SmallMoat.value_or(0));
            data["MediumMoat"] = makeInt(m_MediumMoat.value_or(0));
        }

        if (tu >= TU05) {
            data["GameType"] = makeInt(m_GameType.value_or(enums::GameType::survival));
            data["MapFeatures"] = makeByte(m_MapFeatures.value_or(1));
            data["newSeaLevel"] = makeByte(1);
            data["spawnBonusChest"] = makeByte(m_spawnBonusChest.value_or(0));
            data["hasBeenInCreative"] = makeByte(m_hasBeenInCreative.value_or(0));
            data["generatorName"] = makeString(m_generatorName.value_or("default"));
            data["generatorVersion"] = makeInt(m_generatorVersion.value_or(1));

            if (tu >= TU19 && tu <= TU23) {
                // these 4 updates write empty strings if it has no value, later on it doesn't.
                data["generatorOptions"] = makeString(m_generatorOptions.value_or(""));
            } else {
                if (m_generatorOptions.has_value()) {
                    data["generatorOptions"] = makeString(m_generatorOptions.value());
                }
            }
        }

        if (tu >= TU07) {
            data["hasStronghold"] = makeByte(m_hasStronghold.value_or(0));
            data["StrongholdX"] = makeInt(m_StrongholdX.value_or(0));
            data["StrongholdY"] = makeInt(m_StrongholdY.value_or(0));
            data["StrongholdZ"] = makeInt(m_StrongholdZ.value_or(0));
            data["hardcore"] = makeByte(m_hardcore.value_or(0));
        }

        if (tu >= TU09) {
            data["hasStrongholdEndPortal"] = makeByte(m_hasStrongholdEndPortal.value_or(0));
            data["StrongholdEndPortalX"] = makeInt(m_StrongholdEndPortalX.value_or(0));
            data["StrongholdEndPortalZ"] = makeInt(m_StrongholdEndPortalZ.value_or(0));
        }

        if (tu >= TU14) {
            data["allowCommands"] = makeByte(m_allowCommands.value_or(0));
            data["initialized"] = makeByte(1);
        }

        if (tu >= TU17) {
            data["XZSize"] = makeInt(m_XZSize.value_or(54));
            data["HellScale"] = makeInt(m_HellScale.value_or(3));
        }

        if (tu >= TU19) {
            data["DayTime"] = makeLong(m_DayTime.value_or(0));
        }

        if (tu >= TU25) {
            data["Difficulty"] = makeByte(m_Difficulty.value_or(enums::Difficulty::easy));
            data["clearWeatherTime"] = makeInt(m_clearWeatherTime.value_or(0));
        }

        if (tu >= TU31) {
            data["DifficultyLocked"] = makeByte(m_DifficultyLocked.value_or(0));
        }

        if (tu >= TU36) {
            data["BiomeCentreXChunk"] = makeInt(m_BiomeCentreXChunk.value_or(0));
            data["BiomeCentreZChunk"] = makeInt(m_BiomeCentreZChunk.value_or(0));
            data["BiomeScale"] = makeInt(m_BiomeScale.value_or(enums::BiomeScale::small));
        }

        if (tu >= TU46) {
            if (m_DimensionData.has_value()) {
                data["DimensionData"] = makeCompound(m_DimensionData.value().copy());
            }
            data["ModernEnd"] = makeByte(1);

            if (tu <= TU53) {
                data["DataVersion"] = makeInt(510);
            } else {
                data["DataVersion"] = makeInt(922);
            }
        }

        NBTCompound root;
        root[""]["Data"] = makeCompound(std::move(data));
        NBTBase finalNBT = makeCompound(std::move(root));

        DataWriter writer;
        finalNBT.write(writer);
        levelFile.setBuffer(std::move(writer.take()));
    }


}
