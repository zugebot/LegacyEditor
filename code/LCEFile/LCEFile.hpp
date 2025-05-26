#pragma once

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"

#include "common/error_status.hpp"
#include "common/nbt.hpp"
#include "common/Buffer.hpp"

namespace editor {


    class LCEFile {
        NBTCompound m_nbt;
    public:
        Buffer m_data;
        u64 m_timestamp = 0;
        std::string m_internalName;
        lce::CONSOLE m_console = lce::CONSOLE::NONE;
        lce::FILETYPE m_fileType = lce::FILETYPE::NONE;

        explicit LCEFile(lce::CONSOLE consoleIn);
        explicit LCEFile(lce::CONSOLE consoleIn, u32 sizeIn);
        LCEFile(lce::CONSOLE consoleIn, u32 sizeIn, u64 timestampIn);

        template<class B>
            requires std::same_as<std::remove_cvref_t<B>, Buffer>
        LCEFile(lce::CONSOLE consoleIn, B&& bufferIn, c_u64 timestampIn)
            : m_data(std::forward<B>(bufferIn)),
              m_timestamp(timestampIn),
              m_console(consoleIn) {}

        ~LCEFile();

        MU ND bool isRegionType() const {
            return m_fileType == lce::FILETYPE::OLD_REGION_NETHER ||
                   m_fileType == lce::FILETYPE::OLD_REGION_OVERWORLD ||
                   m_fileType == lce::FILETYPE::OLD_REGION_END;
        }

        MU ND bool isEntityType() const {
            return m_fileType == lce::FILETYPE::ENTITY_NETHER ||
                   m_fileType == lce::FILETYPE::ENTITY_OVERWORLD ||
                   m_fileType == lce::FILETYPE::ENTITY_END;
        }

        void clear();
        // MU void steal(Data& other) { m_data.steal(other); }

        ND std::string constructFileName(lce::CONSOLE console) const;
        MU ND bool isEmpty() const { return !m_data.empty(); }
        MU ND std::string toString() const;

        MU void setType(lce::FILETYPE fileTypeIn) {
            m_fileType = fileTypeIn;
        }
        MU ND lce::FILETYPE getType() const { return m_fileType; }
        MU void setFileName(const std::string& filename);
        MU ND std::string getFileName() const;


        MU void setRegionX(i16 regionX);
        MU ND i16 getRegionX() const;

        MU void setRegionZ(i16 regionZ);
        MU ND i16 getRegionZ() const;

        MU void setMapNumber(i16 mapNumber);
        MU ND i16 getMapNumber() const;



    private:
        MU void setTag(const std::string& key, i16 value);
        MU ND i16 getTag(const std::string& key) const;

    };

}


