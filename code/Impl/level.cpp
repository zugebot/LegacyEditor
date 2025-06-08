#include "level.hpp"

#include "common/nbt.hpp"
#include "code/LCEFile/LCEFile.hpp"


template<>
std::optional<editor::enums::GameType> NBTBase::value<editor::enums::GameType>(const std::string& key) const {
    if (!hasKey(key)) return std::nullopt;
    return static_cast<editor::enums::GameType>(value<int>(key).value());
}

template<>
std::optional<editor::enums::Difficulty> NBTBase::value<editor::enums::Difficulty>(const std::string& key) const {
    if (!hasKey(key)) return std::nullopt;
    return static_cast<editor::enums::Difficulty>(value<u8>(key).value());
}



namespace editor {


    MU void Level::resetSettings() {
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
    }


    MU void Level::defaultSettings() {
        m_BiomeCentreXChunk = 0;
        m_BiomeCentreZChunk = 0;
        m_BiomeScale = 0;
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
        m_RandomSeed = 69420;
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
        m_thunderTime = 0;
        m_thundering = 0;
        m_version = 19132;
    }


    MU void Level::read(const LCEFile& levelFile) {
        Buffer buffer = levelFile.getBuffer();
        if (buffer.empty()) {
            return;
        }

        DataReader reader(buffer.data(), buffer.size());
        NBTBase root = NBTBase::read(reader);

        const auto& data = root("")("Data");

        auto grab = [data](auto& member, const char* key) {
            using Val = typename std::decay_t<decltype(member)>::value_type;
            if (data.hasKey(key))
                member = data.template value<Val>(key);
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
    }


    MU void Level::write(LCEFile& levelFile, const TU& tu) {
        NBTCompound data;
        // data["cloudHeight"] = makeInt(65);

        if (tu >= TU36 && m_BiomeCentreXChunk.has_value())
            data["BiomeCentreXChunk"] = makeInt(m_BiomeCentreXChunk.value());
        if (tu >= TU36 && m_BiomeCentreZChunk.has_value())
            data["BiomeCentreZChunk"] = makeInt(m_BiomeCentreZChunk.value());
        if (tu >= TU36 && m_BiomeScale.has_value())
            data["BiomeScale"] = makeInt(m_BiomeScale.value());
        if (tu >= TU46 && tu <= TU53) {
            data["DataVersion"] = makeLong(510);
        } else if (tu > TU53) {
            data["DataVersion"] = makeLong(922);
        }
        if (tu >= TU19 && m_DayTime.has_value())
            data["DayTime"] = makeLong(m_DayTime.value());
        if (tu >= TU25 && m_Difficulty.has_value())
            data["Difficulty"] = makeByte(static_cast<u8>(m_Difficulty.value()));
        if (tu >= TU31 && m_DifficultyLocked.has_value())
            data["DifficultyLocked"] = makeByte(m_DifficultyLocked.value());
        if (tu >= TU46 && m_DimensionData.has_value())
            data["DimensionData"] = makeCompound(m_DimensionData.value());
        if (tu >= TU05 && m_GameType.has_value())
            data["GameType"] = makeInt(static_cast<u8>(m_GameType.value()));
        if (tu >= TU17 && m_HellScale.has_value())
            data["HellScale"] = makeInt(m_HellScale.value());
        if (m_LastPlayed.has_value())
            data["LastPlayed"] = makeLong(m_LastPlayed.value());
        if (m_LevelName.has_value())
            data["LevelName"] = makeString(m_LevelName.value());
        if (tu >= TU05)
            data["MapFeatures"] = makeByte(m_MapFeatures.value());
        if (tu >= TU46)
            data["ModernEnd"] = makeByte(1);
        if (m_RandomSeed.has_value())
            data["RandomSeed"] = makeLong(m_RandomSeed.value());
        if (m_SizeOnDisk.has_value())
            data["SizeOnDisk"] = makeLong(m_SizeOnDisk.value());
        if (m_SpawnX.has_value())
            data["SpawnX"] = makeInt(m_SpawnX.value());
        if (m_SpawnY.has_value())
            data["SpawnY"] = makeInt(m_SpawnY.value());
        if (m_SpawnZ.has_value())
            data["SpawnZ"] = makeInt(m_SpawnZ.value());
        if (tu >= TU09 && m_StrongholdEndPortalX.has_value())
            data["StrongholdEndPortalX"] = makeInt(m_StrongholdEndPortalX.value());
        if (tu >= TU09 && m_StrongholdEndPortalZ.has_value())
            data["StrongholdEndPortalZ"] = makeInt(m_StrongholdEndPortalZ.value());
        if (tu >= TU07 && m_StrongholdX.has_value())
            data["StrongholdX"] = makeInt(m_StrongholdX.value());
        if (tu >= TU07 && m_StrongholdY.has_value())
            data["StrongholdY"] = makeInt(m_StrongholdY.value());
        if (tu >= TU07 && m_StrongholdZ.has_value())
            data["StrongholdZ"] = makeInt(m_StrongholdZ.value());
        if (m_Time.has_value())
            data["Time"] = makeLong(m_Time.value());
        if (tu >= TU17 && m_XZSize.has_value())
            data["XZSize"] = makeInt(m_XZSize.value());
        if (tu >= TU14 && m_allowCommands.has_value())
            data["allowCommands"] = makeByte(m_allowCommands.value());
        if (tu >= TU25 && m_clearWeatherTime.has_value())
            data["clearWeatherTime"] = makeInt(m_clearWeatherTime.value());
        if (tu >= TU05 && m_generatorName.has_value())
            data["generatorName"] = makeString(m_generatorName.value());
        if (tu >= TU19 && tu <= TU23 && m_generatorOptions.has_value())
            data["generatorOptions"] = makeString(m_generatorOptions.value());
        if (tu >= TU05 && m_generatorVersion.has_value())
            data["generatorVersion"] = makeInt(m_generatorVersion.value());
        if (tu >= TU07 && m_hardcore.has_value())
            data["hardcore"] = makeByte(m_hardcore.value());
        if (tu >= TU05 && m_hasBeenInCreative.has_value())
            data["hasBeenInCreative"] = makeByte(m_hasBeenInCreative.value());
        if (tu >= TU07 && m_hasStronghold.has_value())
            data["hasStronghold"] = makeByte(m_hasStronghold.value());
        if (tu >= TU09 && m_hasStrongholdEndPortal.has_value())
            data["hasStrongholdEndPortal"] = makeByte(m_hasStrongholdEndPortal.value());
        if (tu >= TU14 && m_initialized.has_value())
            data["initialized"] = makeByte(m_initialized.value());
        if (tu >= TU05)
            data["newSeaLevel"] = makeByte(1);
        if (m_rainTime.has_value())
            data["rainTime"] = makeInt(m_rainTime.value());
        if (m_raining.has_value())
            data["raining"] = makeByte(m_raining.value());
        if (tu >= TU05 && m_spawnBonusChest.has_value())
            data["spawnBonusChest"] = makeByte(m_spawnBonusChest.value());
        if (m_thunderTime.has_value())
            data["thunderTime"] = makeInt(m_thunderTime.value());
        if (m_thundering.has_value())
            data["thundering"] = makeByte(m_thundering.value());
        if (m_version.has_value())
            data["version"] = makeInt(m_version.value());


        NBTCompound root;
        root[""]["Data"] = makeCompound(std::move(data));

        NBTBase finalNBT = makeCompound(std::move(root));

        DataWriter writer;
        finalNBT.write(writer);
        levelFile.setBuffer(std::move(writer.take()));
    }


}
