#pragma once

#include "lce/enums.hpp"
#include "lce/processor.hpp"

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/error_status.hpp"


class NBTTagCompound;

namespace editor {


    class LCEFile {
        NBTTagCompound* nbt = nullptr;
    public:
        Data data;
        u64 timestamp = 0;
        u32 additionalData = 0;
        lce::CONSOLE console = lce::CONSOLE::NONE;
        lce::FILETYPE fileType = lce::FILETYPE::NONE;

        explicit LCEFile(lce::CONSOLE consoleIn);
        explicit LCEFile(lce::CONSOLE consoleIn, u32 sizeIn);
        LCEFile(lce::CONSOLE consoleIn, u32 sizeIn, u64 timestampIn);
        LCEFile(lce::CONSOLE consoleIn, u8* dataIn, u32 sizeIn, u64 timestampIn);

        ~LCEFile();

        MU ND bool isRegionType() const {
            return fileType == lce::FILETYPE::REGION_NETHER ||
                   fileType == lce::FILETYPE::REGION_OVERWORLD ||
                   fileType == lce::FILETYPE::REGION_END;
        }

        MU ND bool isEntityType() const {
            return fileType == lce::FILETYPE::ENTITY_NETHER ||
                   fileType == lce::FILETYPE::ENTITY_OVERWORLD ||
                   fileType == lce::FILETYPE::ENTITY_END;
        }

        void deleteData();
        MU void steal(Data& other) { data.steal(other); }

        ND std::string constructFileName(lce::CONSOLE console, bool separateRegions) const;
        MU ND bool isEmpty() const { return data.size != 0; }
        MU ND std::string toString() const;

        MU void setRegionX(i16 regionX);
        MU ND i16 getRegionX() const;

        MU void setRegionZ(i16 regionZ);
        MU ND i16 getRegionZ() const;

        MU void setMapNumber(i16 mapNumber);
        MU ND i16 getMapNumber() const;

        MU void setFileName(const std::string& filename) const;
        MU ND std::string getFileName() const;

    private:
        MU void setTag(const std::string& key, i16 value) const;
        MU ND i16 getTag(const std::string& key) const;

    };

    using LCEFile_vec = std::vector<LCEFile>;
    // using LCEFilePtr_vec = std::vector<LCEFile *>;
}


