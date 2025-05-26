#include "LCEFile.hpp"

#include "common/nbt.hpp"


namespace editor {


    LCEFile::LCEFile(const lce::CONSOLE consoleIn) : m_console(consoleIn) {

    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn) : m_console(consoleIn) {
        m_data = Buffer(sizeIn);
    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn, c_u64 timestampIn) : m_timestamp(timestampIn), m_console(consoleIn) {
        m_data = Buffer(sizeIn);
    }


    LCEFile::~LCEFile() {
        m_data.clear();
    }


    void LCEFile::clear() {
        m_data.clear();
    }


    std::string LCEFile::constructFileName(MU lce::CONSOLE theConsole) const {
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
                name = getFileName();
                break;
            case lce::FILETYPE::MAP: {
                c_i16 mapNum = getMapNumber();
                name = "data/map_" + to_string(mapNum) + ".dat";
                break;
            }
            case lce::FILETYPE::PLAYER:
                name = getFileName();
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
                char dimChar =
                        m_fileType == lce::FILETYPE::NEW_REGION_NETHER ? '1' :
                        m_fileType == lce::FILETYPE::NEW_REGION_END    ? '2' : '0';
                auto byteToHex = [](i32 v) -> std::string {
                    std::ostringstream os;
                    os << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                       << (v & 0xFF);
                    return os.str();
                };

                name = "GAMEDATA_000";
                name += dimChar;
                name += byteToHex(getRegionX());
                name += byteToHex(getRegionZ());
                break;
            }
        }
        return name;
    }

    MU std::string LCEFile::toString() const {
        std::string str;
        str .append("[")
            .append("type='")
            .append(fileTypeToString(m_fileType))
            .append("', name='")
            .append(constructFileName(lce::CONSOLE::WIIU))
            .append("']");
        return str;
    }



    MU void LCEFile::setRegionX(c_i16 regionX) { setTag("x", regionX); }

    MU ND i16 LCEFile::getRegionX() const { return getTag("x"); }

    MU void LCEFile::setRegionZ(c_i16 regionZ) { setTag("z", regionZ); }

    MU ND i16 LCEFile::getRegionZ() const { return getTag("z"); }

    MU void LCEFile::setMapNumber(c_i16 mapNumber) { setTag("#", mapNumber); }

    MU ND i16 LCEFile::getMapNumber() const { return getTag("#"); }

    MU void LCEFile::setFileName(const std::string& filename) {
        m_internalName = filename;
    }

    MU ND std::string LCEFile::getFileName() const {
        return m_internalName;
    }

    MU void LCEFile::setTag(const std::string& key, i16 value) {
        m_nbt.insert(key, makeShort(value));
    }

    MU i16 LCEFile::getTag(const std::string& key) const {
        return m_nbt.value<i16>(key).value_or(-1);
    }
}