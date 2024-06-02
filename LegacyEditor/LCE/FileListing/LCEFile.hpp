#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/error_status.hpp"
#include "LegacyEditor/utils/processor.hpp"


class NBTTagCompound;

namespace editor {

    enum class LCEFileType : u8 {
        NONE,             // NONE
        STRUCTURE,        // data/
        VILLAGE,          // data/
        DATA_MAPPING,     // data/
        MAP,              // data/
        REGION_NETHER,    // ...
        REGION_OVERWORLD, // ...
        REGION_END,       // ...
        PLAYER,           // ...
        LEVEL,            // ...
        GRF,              // ...
        ENTITY_NETHER,    // ...
        ENTITY_OVERWORLD, // ...
        ENTITY_END,       // ...
    };

    static std::string fileTypeToString(const LCEFileType type) {
        switch (type) {
            case LCEFileType::STRUCTURE:
                return "STRUCTURE";
            case LCEFileType::VILLAGE:
                return "VILLAGE";
            case LCEFileType::DATA_MAPPING:
                return "DATA_MAPPING";
            case LCEFileType::MAP:
                return "MAP";
            case LCEFileType::REGION_NETHER:
                return "REGION_NETHER";
            case LCEFileType::REGION_OVERWORLD:
                return "REGION_OVERWORLD";
            case LCEFileType::REGION_END:
                return "REGION_END";
            case LCEFileType::PLAYER:
                return "PLAYER";
            case LCEFileType::LEVEL:
                return "LEVEL";
            case LCEFileType::GRF:
                return "GRF";
            case LCEFileType::ENTITY_NETHER:
                return "ENTITY_NETHER";
            case LCEFileType::ENTITY_OVERWORLD:
                return "ENTITY_OVERWORLD";
            case LCEFileType::ENTITY_END:
                return "ENTITY_END";
            case LCEFileType::NONE:
            default:
                return "NONE";
        }
    }


    class LCEFile {
    public:
        NBTTagCompound* nbt = nullptr;
        Data data;
        u64 timestamp = 0;
        u32 additionalData = 0;
        lce::CONSOLE console = lce::CONSOLE::NONE;
        LCEFileType fileType = LCEFileType::NONE;

        explicit LCEFile(lce::CONSOLE consoleIn);
        explicit LCEFile(lce::CONSOLE consoleIn, u32 sizeIn);
        LCEFile(lce::CONSOLE consoleIn, u32 sizeIn, u64 timestampIn);
        LCEFile(lce::CONSOLE consoleIn, u8* dataIn, u32 sizeIn, u64 timestampIn);

        ~LCEFile();

        MU ND bool isRegionType() const {
            return fileType == LCEFileType::REGION_NETHER ||
                   fileType == LCEFileType::REGION_OVERWORLD ||
                   fileType == LCEFileType::REGION_END;
        }

        MU ND bool isEntityType() const {
            return fileType == LCEFileType::ENTITY_NETHER ||
                   fileType == LCEFileType::ENTITY_OVERWORLD ||
                   fileType == LCEFileType::ENTITY_END;
        }

        void deleteData();
        MU void steal(const Data& other) { data.steal(other); }

        ND std::string constructFileName(lce::CONSOLE console, bool separateRegions) const;
        MU ND bool isEmpty() const { return data.size != 0; }
        MU ND std::string toString() const;

    private:
        MU ND i16 getRegionX() const;
        MU ND i16 getRegionZ() const;
        MU ND i16 getMapNumber() const;
        MU ND std::string getFileName() const;

    };

    using LCEFile_vec = std::vector<LCEFile>;
    using LCEFilePtr_vec = std::vector<LCEFile *>;
}


