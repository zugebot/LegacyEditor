#include "LCEFile.hpp"


#include "LegacyEditor/utils/NBT.hpp"

namespace editor {


    LCEFile::LCEFile(const lce::CONSOLE consoleIn) : console(consoleIn) {
        nbt = new NBTTagCompound();
    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn) : data(Data(sizeIn)), console(consoleIn) {
        nbt = new NBTTagCompound();
    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn, c_u64 timestampIn) : data(Data(sizeIn)), timestamp(timestampIn), console(consoleIn) {}


    LCEFile::LCEFile(const lce::CONSOLE consoleIn, u8* dataIn, c_u32 sizeIn, c_u64 timestampIn) : data(dataIn, sizeIn), timestamp(timestampIn), console(consoleIn) {
        nbt = new NBTTagCompound();
    }


    LCEFile::~LCEFile() {
        nbt->deleteAll();
        delete nbt;
        nbt = nullptr;
    }


    void LCEFile::deleteData() {
        delete[] data.data;
        data.data = nullptr;
        data.size = 0;
    }


    std::string LCEFile::constructFileName(MU lce::CONSOLE console, c_bool separateRegions = false) const {
        static std::unordered_map<LCEFileType, std::string> FileTypeNames{
                {LCEFileType::VILLAGE, "data/villages.dat"},
                {LCEFileType::DATA_MAPPING, "data/largeMapDataMappings.dat"},
                {LCEFileType::LEVEL, "level.dat"},
                {LCEFileType::GRF, "requiredGameRules.grf"},
                {LCEFileType::ENTITY_NETHER, "DIM-1entities.dat"},
                {LCEFileType::ENTITY_OVERWORLD, "entities.dat"},
                {LCEFileType::ENTITY_END, "DIM1/entities.dat"}};
        std::string name = "NULL";
        switch (fileType) {
            using std::to_string;
            case LCEFileType::VILLAGE:
            case LCEFileType::DATA_MAPPING:
            case LCEFileType::LEVEL:
            case LCEFileType::GRF:
            case LCEFileType::ENTITY_NETHER:
            case LCEFileType::ENTITY_OVERWORLD:
            case LCEFileType::ENTITY_END:
                name = FileTypeNames[fileType];
                break;
            case LCEFileType::NONE:
                name = "NONE";
                printf("file encountered with no type, possibly an error?");
                break;
            case LCEFileType::STRUCTURE:
                name = getFileName();
                break;
            case LCEFileType::MAP: {
                c_i16 mapNum = getMapNumber();
                name = "data/map_" + to_string(mapNum) + ".dat";
                break;
            }
            case LCEFileType::PLAYER:
                name = getFileName();
                break;
            // TODO: rewrite to put files in different location for switch / ps4
            case LCEFileType::REGION_NETHER:
            case LCEFileType::REGION_OVERWORLD:
            case LCEFileType::REGION_END: {
                c_i16 regionX = getRegionX();
                c_i16 regionZ = getRegionZ();
                if (fileType == LCEFileType::REGION_NETHER) {
                    name = "DIM-1";
                } else if (fileType == LCEFileType::REGION_OVERWORLD) {
                    name = "";
                } else {
                    name = "DIM1/";
                }
                name += "r." + to_string(regionX) + "." + to_string(regionZ) + ".mcr";
                break;
            }
        }
        return name;
    }

    std::string LCEFile::toString() const {
        std::string str;
        str .append("[")
            .append("type='")
            .append(fileTypeToString(fileType))
            .append("', name='")
            .append(constructFileName(lce::CONSOLE::WIIU))
            .append("']");
        return str;
    }

    MU ND i16 LCEFile::getRegionX() const {
        return nbt->getTag("x").toPrim<i16>();
    }

    MU ND i16 LCEFile::getRegionZ() const {
        return nbt->getTag("z").toPrim<i16>();
    }

    MU ND i16 LCEFile::getMapNumber() const {
        return nbt->getTag("#").toPrim<i16>();
    }

    MU ND std::string LCEFile::getFileName() const {
        return nbt->getString("filename");
    }
}