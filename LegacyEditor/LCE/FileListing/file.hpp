#pragma once

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/NBT.hpp"
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


    class File {
    public:
        NBTBase* nbt = nullptr;
        Data data;
        u64 timestamp = 0;
        u32 additionalData = 0;
        FileType fileType = FileType::NONE;

        ~File();
        File() = default;
        explicit File(const u32 sizeIn) : data(Data(sizeIn)) {}
        File(const u32 sizeIn, const u64 timestampIn) : data(Data(sizeIn)), timestamp(timestampIn) {}
        File(u8* dataIn, const u32 sizeIn, const u64 timestampIn) : data(dataIn, sizeIn), timestamp(timestampIn) {}

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
        ND std::string constructFileName(CONSOLE console) const;
        MU ND bool isEmpty() const { return data.size != 0; }
    };

    typedef std::vector<File> File_vec;
}


