#pragma once

#include <utility>

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"

#include "../../common/data/buffer.hpp"
#include "common/error_status.hpp"
#include "common/nbt.hpp"

namespace editor {


    class LCEFile {
        NBTCompound m_nbt;
        fs::path m_origFolderPath;
        fs::path m_tempFolderPath;
        fs::path m_fileName;
        bool m_tampered = false;

    public:
        lce::CONSOLE m_console = lce::CONSOLE::NONE;
        lce::FILETYPE m_fileType = lce::FILETYPE::NONE;
        u64 m_timestamp = 0;

        explicit
        LCEFile(const lce::CONSOLE consoleIn) : m_console(consoleIn) {}

        LCEFile(lce::CONSOLE consoleIn,
                c_u64 timestampIn,
                fs::path origFolderPathIn,
                fs::path tempFolderPathIn,
                const std::string& fileNameIn) :
              m_timestamp(timestampIn),
              m_console(consoleIn),
              m_origFolderPath(std::move(origFolderPathIn)),
              m_tempFolderPath(std::move(tempFolderPathIn)),
              m_fileName(fileNameIn) {
            initialize(fileNameIn);
        }

        ND fs::path path() const {
            if (m_tampered) {
                return m_tempFolderPath / m_fileName;
            } else {
                return m_origFolderPath / m_fileName;
            }
        }

        MU ND Buffer getBuffer() const {
            return DataReader::readFile(path());
        }

        MU void setBuffer(Buffer buffer) {
            m_tampered = true;
            DataWriter::writeFile(path(), buffer.span());
            m_timestamp = std::time(nullptr);
        }

        MU void setTempFolderPath(fs::path pathIn) {
            m_tempFolderPath = pathIn;
        }

        u64 detectSize() const {
            fs::path filePath = path();
            if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
                return fs::file_size(filePath);
            }
            return 0;
        }

        MU ND bool isRegionType() const {
            return m_fileType == lce::FILETYPE::OLD_REGION_NETHER ||
                   m_fileType == lce::FILETYPE::OLD_REGION_OVERWORLD ||
                   m_fileType == lce::FILETYPE::OLD_REGION_END;
        }

        MU ND bool isTinyRegionType() const {
            return m_fileType == lce::FILETYPE::NEW_REGION_NETHER ||
                   m_fileType == lce::FILETYPE::NEW_REGION_OVERWORLD ||
                   m_fileType == lce::FILETYPE::NEW_REGION_END;
        }

        MU ND bool isEntityType() const {
            return m_fileType == lce::FILETYPE::ENTITY_NETHER ||
                   m_fileType == lce::FILETYPE::ENTITY_OVERWORLD ||
                   m_fileType == lce::FILETYPE::ENTITY_END;
        }

        MU ND std::string toString() const;

        MU void setType(lce::FILETYPE fileTypeIn) { m_fileType = fileTypeIn; }
        MU ND lce::FILETYPE getType() const { return m_fileType; }

        MU void setFileName(const fs::path& filename);
        MU ND fs::path getFileName() const;

        void initialize(const std::string& fileNameIn);
        ND std::string constructFileName() const;

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


