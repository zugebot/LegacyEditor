#include "LCEFile.hpp"

#include "common/nbt.hpp"
#include "common/utils.hpp"


namespace editor {


    void LCEFile::initialize(const std::string& fileNameIn) {
        if (m_timestamp == 0) {
            m_timestamp = std::time(nullptr);
        }

        if (fileNameIn.ends_with(".mcr")) {
            if (fileNameIn.starts_with("DIM-1")) {
                setType(lce::FILETYPE::OLD_REGION_NETHER);
            } else if (fileNameIn.starts_with("DIM1")) {
                setType(lce::FILETYPE::OLD_REGION_END);
            } else if (fileNameIn.starts_with("r")) {
                setType(lce::FILETYPE::OLD_REGION_OVERWORLD);
            }
            c_auto [fst, snd] = extractRegionCoords(fileNameIn);
            setRegionX(static_cast<i16>(fst));
            setRegionZ(static_cast<i16>(snd));

        } else if (fileNameIn.starts_with("GAMEDATA_000")) {
            static constexpr lce::FILETYPE REGION_DIMENSIONS[3] = {
                    lce::FILETYPE::NEW_REGION_OVERWORLD,
                    lce::FILETYPE::NEW_REGION_NETHER,
                    lce::FILETYPE::NEW_REGION_END,
            };
            if (c_auto dimChar = static_cast<char>(static_cast<int>(fileNameIn.at(12)) - 48);
                dimChar < 0 || dimChar > 2) {
                m_fileType = lce::FILETYPE::NONE;
            } else {
                m_fileType = REGION_DIMENSIONS[static_cast<int>(static_cast<u8>(dimChar))];
            }
            c_i16 rX = static_cast<i8>(strtol(fileNameIn.substr(13, 2).c_str(), nullptr, 16));
            c_i16 rZ = static_cast<i8>(strtol(fileNameIn.substr(15, 2).c_str(), nullptr, 16));
            setRegionX(rX);
            setRegionZ(rZ);

        } else if (fileNameIn == "entities.dat") {
            setType(lce::FILETYPE::ENTITY_OVERWORLD);

        } else if (fileNameIn.ends_with("entities.dat")) {
            if (fileNameIn.starts_with("DIM-1")) {
                setType(lce::FILETYPE::ENTITY_NETHER);
            } else if (fileNameIn.starts_with("DIM1/")) {
                setType(lce::FILETYPE::ENTITY_END);
            }

        } else if (fileNameIn == "level.dat") {
            setType(lce::FILETYPE::LEVEL);

        } else if (fileNameIn.starts_with("data/map_")) {
            c_i16 mapNumber = extractMapNumber(fileNameIn);
            setMapNumber(mapNumber);
            setType(lce::FILETYPE::MAP);

        }else if (fileNameIn == "data/villages.dat") {
            setType(lce::FILETYPE::VILLAGE);

        } else if (fileNameIn == "data/largeMapDataMappings.dat") {
            setType(lce::FILETYPE::DATA_MAPPING);

        } else if (fileNameIn.starts_with("data/")) {
            setType(lce::FILETYPE::STRUCTURE);

        } else if (fileNameIn.ends_with(".grf")) {
            setType(lce::FILETYPE::GRF);

        } else if (fileNameIn.starts_with("players/") ||
                   fileNameIn.find('/') == -1LLU) {
            setType(lce::FILETYPE::PLAYER);

        } else {
            setType(lce::FILETYPE::NONE);
        }
    }


    std::string LCEFile::constructFileName() const {
        static std::unordered_map<lce::FILETYPE, std::string> FileTypeNames{
                {lce::FILETYPE::VILLAGE, "data/villages.dat"},
                {lce::FILETYPE::DATA_MAPPING, "data/largeMapDataMappings.dat"},
                {lce::FILETYPE::LEVEL, "level.dat"},
                {lce::FILETYPE::GRF, "requiredGameRules.grf"},
                {lce::FILETYPE::ENTITY_NETHER, "DIM-1entities.dat"},
                {lce::FILETYPE::ENTITY_OVERWORLD, "entities.dat"},
                {lce::FILETYPE::ENTITY_END, "DIM1/entities.dat"}};

        std::string name = "NULL";
        switch (m_fileType) {
            using std::to_string;
            case lce::FILETYPE::VILLAGE:
            case lce::FILETYPE::DATA_MAPPING:
            case lce::FILETYPE::LEVEL:
            case lce::FILETYPE::GRF:
            case lce::FILETYPE::ENTITY_NETHER:
            case lce::FILETYPE::ENTITY_OVERWORLD:
            case lce::FILETYPE::ENTITY_END:
                name = FileTypeNames[m_fileType];
                break;
            case lce::FILETYPE::NONE:
                name = "NONE";
                printf("file encountered with no type, possibly an error?");
                break;
            case lce::FILETYPE::STRUCTURE:
                name = getFileName().string();
                break;
            case lce::FILETYPE::MAP: {
                c_i16 mapNum = getMapNumber();
                name = "data/map_" + to_string(mapNum) + ".dat";
                break;
            }
            case lce::FILETYPE::PLAYER:
                name = getFileName().string();
                break;
            // TODO: rewrite to put files in different location for switch / ps4
            case lce::FILETYPE::OLD_REGION_NETHER:
            case lce::FILETYPE::OLD_REGION_OVERWORLD:
            case lce::FILETYPE::OLD_REGION_END: {
                c_i16 regionX = getRegionX();
                c_i16 regionZ = getRegionZ();
                if (m_fileType == lce::FILETYPE::OLD_REGION_NETHER) {
                    name = "DIM-1";
                } else if (m_fileType == lce::FILETYPE::OLD_REGION_OVERWORLD) {
                    name = "";
                } else {
                    name = "DIM1/";
                }
                name += "r." + to_string(regionX) + "." + to_string(regionZ) + ".mcr";
                break;
            }
            case lce::FILETYPE::NEW_REGION_NETHER:
            case lce::FILETYPE::NEW_REGION_OVERWORLD:
            case lce::FILETYPE::NEW_REGION_END: {
                char dimChar;
                switch (m_fileType) {
                    default: // will never default
                    case lce::FILETYPE::NEW_REGION_OVERWORLD:
                        dimChar = '0';
                        break;
                    case lce::FILETYPE::NEW_REGION_NETHER:
                        dimChar = '1';
                        break;
                    case lce::FILETYPE::NEW_REGION_END:
                        dimChar = '2';
                        break;
                }

                auto byteToHex = [](i32 v) -> std::string {
                    std::ostringstream os;
                    os << std::hex
                       << std::nouppercase
                       << std::setw(2)
                       << std::setfill('0')
                       << (v & 0xFF);
                    return os.str();
                };

                name = "GAMEDATA_000";
                name += dimChar;
                name += byteToHex(getRegionX());
                name += byteToHex(getRegionZ());
                break;
            }
            case lce::FILETYPE::OLD_REGION_ANY:
            case lce::FILETYPE::NEW_REGION_ANY:
            case lce::FILETYPE::ENTITY_ANY:
                break;
        }
        return name;
    }

    MU std::string LCEFile::toString() const {
        std::string str;
        str .append("[")
            .append("type='")
            .append(fileTypeToString(m_fileType))
            .append("', name='")
            .append(constructFileName())
            .append("']");
        return str;
    }



    MU void LCEFile::setRegionX(c_i16 regionX) { setTag("x", regionX); }

    MU ND i16 LCEFile::getRegionX() const { return getTag("x"); }

    MU void LCEFile::setRegionZ(c_i16 regionZ) { setTag("z", regionZ); }

    MU ND i16 LCEFile::getRegionZ() const { return getTag("z"); }

    MU void LCEFile::setMapNumber(c_i16 mapNumber) { setTag("#", mapNumber); }

    MU ND i16 LCEFile::getMapNumber() const { return getTag("#"); }

    MU void LCEFile::setFileName(const fs::path& path) {
        m_fileName = path;
    }

    MU ND fs::path LCEFile::getFileName() const {
        return m_fileName;
    }

    MU void LCEFile::setTag(const std::string& key, i16 value) {
        m_nbt.insert(key, makeShort(value));
    }

    MU i16 LCEFile::getTag(const std::string& key) const {
        return m_nbt.value<i16>(key).value_or(-1);
    }
}