#include "file.hpp"


#include "LegacyEditor/utils/NBT.hpp"

namespace editor {


    File::File(const CONSOLE consoleIn) : console(consoleIn) {
        nbt = new NBTTagCompound();
    }

    File::File(const CONSOLE consoleIn, const u32 sizeIn) : data(Data(sizeIn)), console(consoleIn) {
        nbt = new NBTTagCompound();
    }

    File::File(const CONSOLE consoleIn, const u32 sizeIn, const u64 timestampIn) : data(Data(sizeIn)), timestamp(timestampIn), console(consoleIn) {}


    File::File(const CONSOLE consoleIn, u8* dataIn, const u32 sizeIn, const u64 timestampIn) : data(dataIn, sizeIn), timestamp(timestampIn), console(consoleIn) {
        nbt = new NBTTagCompound();
    }


    File::~File() {
        nbt->deleteAll();
        delete nbt;
        nbt = nullptr;
    }


    void File::deleteData() {
        delete[] data.data;
        data.data = nullptr;
        data.size = 0;
    }


    std::string File::constructFileName(MU CONSOLE console, const bool separateRegions = false) const {
        static std::unordered_map<FileType, std::string> FileTypeNames{
                {FileType::VILLAGE, "data/villages.dat"},
                {FileType::DATA_MAPPING, "data/largeMapDataMappings.dat"},
                {FileType::LEVEL, "level.dat"},
                {FileType::GRF, "requiredGameRules.grf"},
                {FileType::ENTITY_NETHER, "DIM-1entities.dat"},
                {FileType::ENTITY_OVERWORLD, "entities.dat"},
                {FileType::ENTITY_END, "DIM1/entities.dat"}};
        std::string name = "NULL";
        switch (fileType) {
            using std::to_string;
            case FileType::VILLAGE:
            case FileType::DATA_MAPPING:
            case FileType::LEVEL:
            case FileType::GRF:
            case FileType::ENTITY_NETHER:
            case FileType::ENTITY_OVERWORLD:
            case FileType::ENTITY_END:
                name = FileTypeNames[fileType];
                break;
            case FileType::NONE:
                name = "NONE";
                printf("file encountered with no type, possibly an error?");
                break;
            case FileType::STRUCTURE:
                name = getFileName();
                break;
            case FileType::MAP: {
                const i16 mapNum = getMapNumber();
                name = "data/map_" + to_string(mapNum) + ".dat";
                break;
            }
            case FileType::PLAYER:
                name = getFileName();
                break;
            // TODO: rewrite to put files in different location for switch / ps4
            case FileType::REGION_NETHER:
            case FileType::REGION_OVERWORLD:
            case FileType::REGION_END: {
                const i16 regionX = getRegionX();
                const i16 regionZ = getRegionZ();
                if (fileType == FileType::REGION_NETHER) {
                    name = "DIM-1";
                } else if (fileType == FileType::REGION_OVERWORLD) {
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

    std::string File::toString() const {
        std::string str;
        str .append("[")
            .append("type='")
            .append(fileTypeToString(fileType))
            .append("', name='")
            .append(constructFileName(CONSOLE::WIIU))
            .append("']");
        return str;
    }

    MU ND i16 File::getRegionX() const {
        return nbt->getTag("x").toPrim<i16>();
    }

    MU ND i16 File::getRegionZ() const {
        return nbt->getTag("z").toPrim<i16>();
    }

    MU ND i16 File::getMapNumber() const {
        return nbt->getTag("#").toPrim<i16>();
    }

    MU ND std::string File::getFileName() const {
        return nbt->getString("filename");
    }
}