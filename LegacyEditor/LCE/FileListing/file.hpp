#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/processor.hpp"


class NBTBase;
class NBTTagCompound;

namespace editor {

    enum class FileType : u8 {
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

    static std::string fileTypeToString(const FileType type) {
        switch (type) {
            case FileType::STRUCTURE:
                return "STRUCTURE";
            case FileType::VILLAGE:
                return "VILLAGE";
            case FileType::DATA_MAPPING:
                return "DATA_MAPPING";
            case FileType::MAP:
                return "MAP";
            case FileType::REGION_NETHER:
                return "REGION_NETHER";
            case FileType::REGION_OVERWORLD:
                return "REGION_OVERWORLD";
            case FileType::REGION_END:
                return "REGION_END";
            case FileType::PLAYER:
                return "PLAYER";
            case FileType::LEVEL:
                return "LEVEL";
            case FileType::GRF:
                return "GRF";
            case FileType::ENTITY_NETHER:
                return "ENTITY_NETHER";
            case FileType::ENTITY_OVERWORLD:
                return "ENTITY_OVERWORLD";
            case FileType::ENTITY_END:
                return "ENTITY_END";
            case FileType::NONE:
            default:
                return "NONE";
        }
    }


    class File {
    public:
        NBTBase* nbt = nullptr;
        Data data;
        u64 timestamp = 0;
        u32 additionalData = 0;
        CONSOLE console = CONSOLE::NONE;
        FileType fileType = FileType::NONE;

        File();
        explicit File(CONSOLE consoleIn, u32 sizeIn);
        File(CONSOLE consoleIn, u32 sizeIn, u64 timestampIn);
        File(CONSOLE consoleIn, u8* dataIn, u32 sizeIn, u64 timestampIn);

        ~File();

        MU ND bool isRegionType() const {
            return fileType == FileType::REGION_NETHER ||
                   fileType == FileType::REGION_OVERWORLD ||
                   fileType == FileType::REGION_END;
        }

        MU ND bool isEntityType() const {
            return fileType == FileType::ENTITY_NETHER ||
                   fileType == FileType::ENTITY_OVERWORLD ||
                   fileType == FileType::ENTITY_END;
        }

        void deleteData();
        MU void steal(const Data& other) { data.steal(other); }

        ND NBTTagCompound* createNBTTagCompound();
        ND NBTTagCompound* getNBTCompound() const;
        void deleteNBTCompound();
        ND std::string constructFileName(CONSOLE console, bool separateRegions) const;
        MU ND bool isEmpty() const { return data.size != 0; }
        MU ND std::string toString() const;

    private:
        MU ND i16 getRegionX() const;
        MU ND i16 getRegionZ() const;
        MU ND i16 getMapNumber() const;
        MU ND std::string getFileName() const;

    };

    typedef std::vector<File> File_vec;
    typedef std::vector<File*> FilePtr_vec;
}


