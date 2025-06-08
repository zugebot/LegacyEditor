#pragma once

#include "common/nbt.hpp"
#include "lce/processor.hpp"


namespace editor {


    static auto parseLabel(std::string_view s) -> std::pair<int, int> {
        int cat = 2;    // default = “other”
        size_t pos = 0; // position where the digits start

        if (s.starts_with("preTU")) {
            cat = 0;
            pos = 5; // skip "preTU"
        } else if (s.starts_with("TU")) {
            cat = 1;
            pos = 2; // skip "TU"
        }

        int num = 0;
        while (pos < s.size() && std::isdigit(s[pos]))
            num = num * 10 + (s[pos++] - '0');

        return {cat, num};
    }


    class TU {
        std::string_view m_tu;

    public:
        constexpr TU(std::string_view tu) : m_tu(tu) {}

        ND const std::string_view& str() const { return m_tu; }

        auto operator<=>(const TU& other) const {
            return parseLabel(m_tu) <=> parseLabel(other.m_tu);
        }

        bool operator==(const TU& other) const = default;
    };


    MU static constexpr TU preTU0033 = TU("preTU0033");
    MU static constexpr TU preTU0035 = TU("preTU0035");
    MU static constexpr TU TU00 = TU("TU00");
    MU static constexpr TU TU01 = TU("TU01");
    MU static constexpr TU TU02 = TU("TU02");
    MU static constexpr TU TU03 = TU("TU03");
    MU static constexpr TU TU04 = TU("TU04");
    MU static constexpr TU TU05 = TU("TU05");
    MU static constexpr TU TU06 = TU("TU06");
    MU static constexpr TU TU07 = TU("TU07");
    MU static constexpr TU TU08 = TU("TU08");
    MU static constexpr TU TU09 = TU("TU09");
    MU static constexpr TU TU10 = TU("TU10");
    MU static constexpr TU TU11 = TU("TU11");
    MU static constexpr TU TU12 = TU("TU12");
    MU static constexpr TU TU13 = TU("TU13");
    MU static constexpr TU TU14 = TU("TU14");
    MU static constexpr TU TU15 = TU("TU15");
    MU static constexpr TU TU16 = TU("TU16");
    MU static constexpr TU TU17 = TU("TU17");
    MU static constexpr TU TU18 = TU("TU18");
    MU static constexpr TU TU19 = TU("TU19");
    MU static constexpr TU TU20 = TU("TU20");
    MU static constexpr TU TU21 = TU("TU21");
    MU static constexpr TU TU22 = TU("TU22");
    MU static constexpr TU TU23 = TU("TU23");
    MU static constexpr TU TU24 = TU("TU24");
    MU static constexpr TU TU25 = TU("TU25");
    MU static constexpr TU TU26 = TU("TU26");
    MU static constexpr TU TU27 = TU("TU27");
    MU static constexpr TU TU28 = TU("TU28");
    MU static constexpr TU TU29 = TU("TU29");
    MU static constexpr TU TU30 = TU("TU30");
    MU static constexpr TU TU31 = TU("TU31");
    MU static constexpr TU TU32 = TU("TU32");
    MU static constexpr TU TU33 = TU("TU33");
    MU static constexpr TU TU34 = TU("TU34");
    MU static constexpr TU TU35 = TU("TU35");
    MU static constexpr TU TU36 = TU("TU36");
    MU static constexpr TU TU37 = TU("TU37");
    MU static constexpr TU TU38 = TU("TU38");
    MU static constexpr TU TU39 = TU("TU39");
    MU static constexpr TU TU40 = TU("TU40");
    MU static constexpr TU TU41 = TU("TU41");
    MU static constexpr TU TU42 = TU("TU42");
    MU static constexpr TU TU43 = TU("TU43");
    MU static constexpr TU TU44 = TU("TU44");
    MU static constexpr TU TU45 = TU("TU45");
    MU static constexpr TU TU46 = TU("TU46");
    MU static constexpr TU TU47 = TU("TU47");
    MU static constexpr TU TU48 = TU("TU48");
    MU static constexpr TU TU49 = TU("TU49");
    MU static constexpr TU TU50 = TU("TU50");
    MU static constexpr TU TU51 = TU("TU51");
    MU static constexpr TU TU52 = TU("TU52");
    MU static constexpr TU TU53 = TU("TU53");
    MU static constexpr TU TU54 = TU("TU54");
    MU static constexpr TU TU55 = TU("TU55");
    MU static constexpr TU TU56 = TU("TU56");
    MU static constexpr TU TU57 = TU("TU57");
    MU static constexpr TU TU58 = TU("TU58");
    MU static constexpr TU TU59 = TU("TU59");
    MU static constexpr TU TU60 = TU("TU60");
    MU static constexpr TU TU61 = TU("TU61");
    MU static constexpr TU TU62 = TU("TU62");
    MU static constexpr TU TU63 = TU("TU63");
    MU static constexpr TU TU64 = TU("TU64");
    MU static constexpr TU TU65 = TU("TU65");
    MU static constexpr TU TU66 = TU("TU66");
    MU static constexpr TU TU67 = TU("TU67");
    MU static constexpr TU TU68 = TU("TU68");
    MU static constexpr TU TU69 = TU("TU69");


    class LCEFile;


    namespace enums {
        enum class GameType {
            survival,
            creative,
            adventure
        };

        enum class Difficulty {
            peaceful,
            easy,
            medium,
            hard
        };


    }



class Level {
public:
        /* TU36 */ std::optional<i32> m_BiomeCentreXChunk;
        /* TU36 */ std::optional<i32> m_BiomeCentreZChunk;
        /* TU36 */ std::optional<i32> m_BiomeScale;
        /* TU46 */ std::optional<i32> m_DataVersion;
        /* TU19 */ std::optional<i64> m_DayTime;
        /* TU25 */ std::optional<enums::Difficulty> m_Difficulty;
        /* TU31 */ std::optional<u8> m_DifficultyLocked;
        /* TU46 */ std::optional<NBTCompound> m_DimensionData;
        /* TU05 */ std::optional<enums::GameType> m_GameType;
        /* TU17 */ std::optional<i32> m_HellScale;
        /* ALL_ */ std::optional<i64> m_LastPlayed;
        /* ALL_ */ std::optional<std::string> m_LevelName;
        /* TU05 */ std::optional<u8> m_MapFeatures;
        /* TU46 */ std::optional<u8> m_ModernEnd;
        /* ALL_ */ std::optional<i64> m_RandomSeed;
        /* ALL_ */ std::optional<i64> m_SizeOnDisk;
        /* ALL_ */ std::optional<i32> m_SpawnX;
        /* ALL_ */ std::optional<i32> m_SpawnY;
        /* ALL_ */ std::optional<i32> m_SpawnZ;
        /* TU09 */ std::optional<i32> m_StrongholdEndPortalX;
        /* TU09 */ std::optional<i32> m_StrongholdEndPortalZ;
        /* TU07 */ std::optional<i32> m_StrongholdX;
        /* TU07 */ std::optional<i32> m_StrongholdY;
        /* TU07 */ std::optional<i32> m_StrongholdZ;
        /* ALL_ */ std::optional<i64> m_Time;
        /* TU17 */ std::optional<i32> m_XZSize;
        /* TU14 */ std::optional<u8> m_allowCommands;
        /* TU25 */ std::optional<i32> m_clearWeatherTime;
        /* TU05 */ std::optional<std::string> m_generatorName;
        /*19-23 */ std::optional<std::string> m_generatorOptions;
        /* TU05 */ std::optional<i32> m_generatorVersion;
        /* TU07 */ std::optional<u8> m_hardcore;
        /* TU05 */ std::optional<u8> m_hasBeenInCreative;
        /* TU07 */ std::optional<u8> m_hasStronghold;
        /* TU09 */ std::optional<u8> m_hasStrongholdEndPortal;
        /* TU14 */ std::optional<u8> m_initialized;
        /* TU05 */ std::optional<u8> m_newSeaLevel;
        /* ALL_ */ std::optional<i32> m_rainTime;
        /* ALL_ */ std::optional<u8> m_raining;
        /* TU05 */ std::optional<u8> m_spawnBonusChest;
        /* ALL_ */ std::optional<i32> m_thunderTime;
        /* ALL_ */ std::optional<u8> m_thundering;
        /* ALL_ */ std::optional<i32> m_version;

        MU void resetSettings();
        MU void defaultSettings();
        MU void read(const LCEFile& levelFile);
        MU void write(LCEFile& levelFile, const TU& tu);
    };


}



