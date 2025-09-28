#pragma once

#include "common/nbt.hpp"
#include "lce/processor.hpp"
#include "lce/titleUpdate.hpp"


namespace editor {


    class LCEFile;


    namespace enums {
        enum class GameType : i32 {
            survival,
            creative,
            adventure
        };

        enum class Difficulty : u8 {
            peaceful,
            easy,
            medium,
            hard
        };

        enum class BiomeScale : i32 {
            small,
            medium,
            large
        };


    }

    class LevelFile {
    public:
        /* ALL  */ std::optional<i64> m_LastPlayed;
        /* ALL  */ std::optional<std::string> m_LevelName;
        /* ALL  */ std::optional<i64> m_RandomSeed;
        /* ALL  */ std::optional<i64> m_SizeOnDisk;
        /* ALL  */ std::optional<i32> m_SpawnX;
        /* ALL  */ std::optional<i32> m_SpawnY;
        /* ALL  */ std::optional<i32> m_SpawnZ;
        /* ALL  */ std::optional<i64> m_Time;
        /* ALL  */ std::optional<i32> m_rainTime;
        /* ALL  */ std::optional<u8> m_raining;
        /* ALL  */ std::optional<i32> m_thunderTime;
        /* ALL  */ std::optional<u8> m_thundering;
        /* ALL  */ std::optional<i32> m_version;
        /* TU05 */ std::optional<u8> m_MapFeatures;
        /* TU05 */ std::optional<std::string> m_generatorName;
        /* TU05 */ std::optional<i32> m_generatorVersion;
        /* TU05 */ std::optional<u8> m_hasBeenInCreative;
        /* TU05 */ std::optional<u8> m_newSeaLevel;
        /* TU05 */ std::optional<u8> m_spawnBonusChest;
        /* TU05 */ std::optional<enums::GameType> m_GameType;
        /* TU07 */ std::optional<i32> m_StrongholdX;
        /* TU07 */ std::optional<i32> m_StrongholdY;
        /* TU07 */ std::optional<i32> m_StrongholdZ;
        /* TU17 */ std::optional<i32> m_XZSize;
        /* TU07 */ std::optional<u8> m_hardcore;
        /* TU07 */ std::optional<u8> m_hasStronghold;
        /* TU09 */ std::optional<u8> m_hasStrongholdEndPortal;
        /* TU09 */ std::optional<i32> m_StrongholdEndPortalX;
        /* TU09 */ std::optional<i32> m_StrongholdEndPortalZ;
        /* TU14 */ std::optional<u8> m_allowCommands;
        /* TU14 */ std::optional<u8> m_initialized;
        /* TU17 */ std::optional<i32> m_HellScale;
        /* TU19 */ std::optional<i64> m_DayTime;
        // TODO: used to be "std::optional<std::string>" (?) but latest versions use a std::vector<u8>
        /*19-23 */ std::optional<NBTByteArray> m_generatorOptions;
        /* TU25 */ std::optional<enums::Difficulty> m_Difficulty;
        /* TU25 */ std::optional<i32> m_clearWeatherTime;
        /* TU31 */ std::optional<u8> m_DifficultyLocked;
        /* TU36 */ std::optional<i32> m_BiomeCentreXChunk;
        /* TU36 */ std::optional<i32> m_BiomeCentreZChunk;
        /* TU36 */ std::optional<enums::BiomeScale> m_BiomeScale;
        /* TU46 */ std::optional<i32> m_DataVersion;
        /* TU46 */ std::optional<NBTCompound> m_DimensionData;
        /* TU46 */ std::optional<u8> m_ModernEnd;
        /* NEWG */ std::optional<i32> m_ClassicMoat;
        /* NEWG */ std::optional<i32> m_SmallMoat;
        /* NEWG */ std::optional<i32> m_MediumMoat;

        MU void resetSettings();
        MU void defaultSettings();
        MU void read(const LCEFile& levelFile);
        MU void write(LCEFile& levelFile, lce::CONSOLE console, const TU& tu);
    };


}



