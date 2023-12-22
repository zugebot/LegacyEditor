#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/processor.hpp"


/*
 * WiiU: %number%.dat
 * NS__: %number%.dat
 * Vita: THUMBDATA.dat
 * PS3_: THUMB
 * PS4_: THUMB
 * X360: ?
 * X1__: ?
 */


namespace editor {

    class FileInfo {
        inline static const u8 IEND_DAT[12] = {
            0x00, 0x00, 0x00, 0x00, // size = 0
            0x49, 0x45, 0x4E, 0x44, // "IEND"
            0xAE, 0x42, 0x60, 0x82  // crc
        };

    public:
        Data thumbnail;
        Data settings;
        i64 seed{};
        i64 loads{};
        i64 hostoptions{};
        i64 texturepack{};
        i64 extradata{};
        i64 exploredchunks{};
        std::wstring basesavename;
        bool isLoaded;

        void readFile(const std::string& inFileStr);
        ND int writeFile(const std::string& outFileStr, CONSOLE console) const;
    };

}
