#include "map.hpp"
#include "mapcolors.hpp"

#include "lce/include/picture.hpp"

#include "LegacyEditor/LCE/FileListing/LCEFile.hpp"
#include "LegacyEditor/utils/NBT.hpp"


namespace editor::map {


    MU void saveMapToPng(const LCEFile* map,
                         const std::string& path,
                         const std::string& filename) {
        static constexpr int MAP_BYTE_SIZE = 16384;

        if (map->data.data == nullptr) {
            return;
        }

        DataManager mapManager(map->data);
        const auto *const data = NBT::readTag(mapManager);
        const auto* byteArray = NBTBase
                ::toType<NBTTagCompound>(data)
                ->getCompoundTag("data")
                ->getByteArray("colors");

        const Picture picture(128, 128);
        int count = 0;
        for (int i = 0; i < MAP_BYTE_SIZE; i++) {
            const RGB rgb = getRGB(byteArray->array[i]);
            picture.myData[count++] = rgb.r;
            picture.myData[count++] = rgb.g;
            picture.myData[count++] = rgb.b;
        }

        picture.saveWithName(filename, path);
    }
}