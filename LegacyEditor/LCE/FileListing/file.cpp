#include "file.hpp"


#include "LegacyEditor/utils/NBT.hpp"

namespace editor {


    File::File() {}


    File::File(const u32 sizeIn) : data(Data(sizeIn)) {}


    File::File(const u32 sizeIn, const u64 timestampIn) : data(Data(sizeIn)), timestamp(timestampIn) {}


    File::File(u8* dataIn, const u32 sizeIn, const u64 timestampIn) : data(dataIn, sizeIn), timestamp(timestampIn) {}


    File::~File() {
        if (nbt == nullptr) { return; }
        if (nbt->data == nullptr) { return; }
        getNBTCompound()->deleteAll();
        nbt->data = nullptr;
        delete nbt;
        nbt = nullptr;
    }


    void File::deleteData() {
        delete[] data.data;
        data.data = nullptr;
        data.size = 0;
    }


    NBTTagCompound* File::createNBTTagCompound() {
        if (nbt != nullptr) { return nullptr; }
        nbt = new NBTBase(new NBTTagCompound(), TAG_COMPOUND);
        return static_cast<NBTTagCompound*>(nbt->data);
    }


    ND NBTTagCompound* File::getNBTCompound() const {
        if (nbt == nullptr) { return nullptr; }
        if (nbt->data == nullptr) { return nullptr; }
        return static_cast<NBTTagCompound*>(nbt->data);
    }


    void File::deleteNBTCompound() {
        if (nbt == nullptr) { return; }
        if (nbt->data == nullptr) { return; }
        getNBTCompound()->deleteAll();
        nbt->data = nullptr;
        delete nbt;
        nbt = nullptr;
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
            case FileType::NONE: {
                name = "NONE";
                printf("file encountered with no type, possibly an error?");
                break;
            }
            case FileType::STRUCTURE: {
                if (const auto* compound = getNBTCompound(); compound == nullptr) { break; }
                name = getFileName();
                break;
            }
            case FileType::MAP: {
                if (const auto* compound = getNBTCompound(); compound == nullptr) { break; }
                const i16 mapNum = getMapNumber();
                name = "data/map_" + to_string(mapNum) + ".dat";
                break;
            }
            case FileType::PLAYER:
                if (const auto* compound = getNBTCompound(); compound == nullptr) { break; }
                name = getFileName();
                break;
            case FileType::REGION_NETHER:
            case FileType::REGION_OVERWORLD:
            case FileType::REGION_END: {
                if (const auto* compound = getNBTCompound(); compound == nullptr) { break; }
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
        return getNBTCompound()->getTag("x").toPrimitiveType<i16>();
    }

    MU ND i16 File::getRegionZ() const {
        return getNBTCompound()->getTag("z").toPrimitiveType<i16>();
    }

    MU ND i16 File::getMapNumber() const {
        return getNBTCompound()->getTag("#").toPrimitiveType<i16>();
    }

    MU ND std::string File::getFileName() const {
        return getNBTCompound()->getString("filename");
    }
}