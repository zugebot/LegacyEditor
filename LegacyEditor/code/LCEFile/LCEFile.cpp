#include "LCEFile.hpp"

#include "LegacyEditor/utils/NBT.hpp"


namespace editor {


    LCEFile::LCEFile(const lce::CONSOLE consoleIn) : console(consoleIn) {
        nbt = new NBTTagCompound();
    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn) :
        console(consoleIn) {
        nbt = new NBTTagCompound();
        data = Data();
        data.allocate(sizeIn);
    }

    LCEFile::LCEFile(const lce::CONSOLE consoleIn, c_u32 sizeIn, c_u64 timestampIn) :
        timestamp(timestampIn), console(consoleIn) {
        data = Data();
        data.allocate(sizeIn);
    }


    LCEFile::LCEFile(const lce::CONSOLE consoleIn, u8* dataIn, c_u32 sizeIn, c_u64 timestampIn) :
        data(dataIn, sizeIn), timestamp(timestampIn), console(consoleIn) {
        nbt = new NBTTagCompound();
    }


    LCEFile::~LCEFile() {
        if (nbt == nullptr) {
            return;
        }
        nbt->deleteAll();
        delete nbt;
        nbt = nullptr;
    }


    // TODO: why doesn't this delete NBT?
    void LCEFile::deleteData() {
        delete[] data.data;
        data.data = nullptr;
        data.size = 0;
    }


    std::string LCEFile::constructFileName(MU lce::CONSOLE theConsole, MU c_bool separateRegions = false) const {
        static std::unordered_map<lce::FILETYPE, std::string> FileTypeNames{
                {lce::FILETYPE::VILLAGE, "data/villages.dat"},
                {lce::FILETYPE::DATA_MAPPING, "data/largeMapDataMappings.dat"},
                {lce::FILETYPE::LEVEL, "level.dat"},
                {lce::FILETYPE::GRF, "requiredGameRules.grf"},
                {lce::FILETYPE::ENTITY_NETHER, "DIM-1entities.dat"},
                {lce::FILETYPE::ENTITY_OVERWORLD, "entities.dat"},
                {lce::FILETYPE::ENTITY_END, "DIM1/entities.dat"}};
        std::string name = "NULL";
        switch (fileType) {
            using std::to_string;
            case lce::FILETYPE::VILLAGE:
            case lce::FILETYPE::DATA_MAPPING:
            case lce::FILETYPE::LEVEL:
            case lce::FILETYPE::GRF:
            case lce::FILETYPE::ENTITY_NETHER:
            case lce::FILETYPE::ENTITY_OVERWORLD:
            case lce::FILETYPE::ENTITY_END:
                name = FileTypeNames[fileType];
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
            case lce::FILETYPE::REGION_NETHER:
            case lce::FILETYPE::REGION_OVERWORLD:
            case lce::FILETYPE::REGION_END: {
                c_i16 regionX = getRegionX();
                c_i16 regionZ = getRegionZ();
                if (fileType == lce::FILETYPE::REGION_NETHER) {
                    name = "DIM-1";
                } else if (fileType == lce::FILETYPE::REGION_OVERWORLD) {
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

    MU std::string LCEFile::toString() const {
        std::string str;
        str .append("[")
            .append("type='")
            .append(fileTypeToString(fileType))
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

    MU void LCEFile::setFileName(const std::string& filename) const {
        if (nbt == nullptr) return;
        nbt->setString("filename", filename); }

    MU ND std::string LCEFile::getFileName() const {
        if (nbt == nullptr) return "NULL";
        return nbt->getString("filename"); }

    MU void LCEFile::setTag(const std::string& key, i16 value) const {
        if (nbt == nullptr) return;
        nbt->setTag(key, createNBT_INT16(value)); }

    MU i16 LCEFile::getTag(const std::string& key) const {
        if (nbt == nullptr) return -1;
        return nbt->getTag(key).toPrim<i16>(); }
}