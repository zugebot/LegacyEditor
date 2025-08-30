#pragma once


#include <utility>

#include "lce/processor.hpp"

#include "code/Impl/levelFile.hpp"
#include "code/chunk/chunkData.hpp"

#include "code/convert/convertChunk.hpp"
#include "lce/titleUpdate.hpp"


namespace editor::sch {

    enum class eIsNewSave {
        False = 0,
        True = 1
    };

    enum class eIsTerrainFlagNormal {
        False = 0,
        True = 1
    };

    enum class eIsMissingBiomes {
        False = 0,
        True = 1,
        NON_APPLICABLE = 2
    };


    class Schematic {
    public:
        std::string_view           display_name;
        lce::CONSOLE               save_console;
        TU                         save_tu;
        const i32                  fileListing_oldestVersion;
        const i32                  fileListing_latestVersion;
        const i32                  chunk_yHeight;
        const eChunkVersion        chunk_lastVersion;
        const eIsNewSave           chunk_isNewSave;
        const eIsTerrainFlagNormal chunk_isTerrainFlagNormal;
        const eIsMissingBiomes     chunk_isMissingBiomes;

        const ChunkConverterFn     func_chunk_convert;


        constexpr Schematic(
                std::string_view display_name,
                lce::CONSOLE console,
                TU tu,
                i32 i1,
                i32 i2,
                i32 i3,
                eChunkVersion i4,
                eIsNewSave i5,
                eIsTerrainFlagNormal i6,
                eIsMissingBiomes i7,
                ChunkConverterFn i9)

            : display_name(std::move(display_name)),
              save_console(console),
              save_tu(tu),
              fileListing_oldestVersion(i1),
              fileListing_latestVersion(i2),
              chunk_yHeight(i3),
              chunk_lastVersion(i4),
              chunk_isNewSave(i5),
              chunk_isTerrainFlagNormal(i6),
              chunk_isMissingBiomes(i7),
              func_chunk_convert(i9) {

        }

        void setConsole(lce::CONSOLE consoleIn) {
            save_console = consoleIn;
        }

    };



    MU static constexpr Schematic Potions = Schematic(
            "Potions",
            lce::CONSOLE::NONE,
            TU12,
            6,
            6,
            128,
            eChunkVersion::V_NBT,
            eIsNewSave::False,
            eIsTerrainFlagNormal::True,
            eIsMissingBiomes::NON_APPLICABLE,
            convertReadChunkToPotions
    );



    MU static constexpr Schematic ElytraLatest = Schematic(
            "Elytra Latest",
            lce::CONSOLE::NONE,
            TU68,
            10,
            10,
            256,
            eChunkVersion::V_11,
            eIsNewSave::False,
            eIsTerrainFlagNormal::False,
            eIsMissingBiomes::NON_APPLICABLE,
            convertReadChunkToElytra
    );



    MU static constexpr Schematic AquaticTU69 = Schematic(
            "AquaticTU69",
            lce::CONSOLE::NONE,
            TU69,
            12,
            12,
            256,
            eChunkVersion::V_12,
            eIsNewSave::True,
            eIsTerrainFlagNormal::False,
            eIsMissingBiomes::NON_APPLICABLE,
            convertReadChunkToAquatic
    );


}
