#include "map.hpp"

#include "LegacyEditor/LCE/FileListing/file.hpp"
#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/picture.hpp"
#include "mapcolors.hpp"


namespace editor::map {
    MU void saveMapToPng(const File* map,
                         const std::string& path,
                         const std::string& filename) {
        static constexpr int MAP_BYTE_SIZE = 16384;
        DataManager mapManager(map->data);
        const auto *const data = NBT::readTag(mapManager);
        const auto* byteArray = NBTBase::toType<NBTTagCompound>(data)->getCompoundTag("data")->getByteArray("colors");

        const Picture picture(128, 128);
        int count = 0;
        for (int i = 0; i < MAP_BYTE_SIZE; i++) {
            const RGB rgb = getRGB(byteArray->array[i]);
            picture.data[count++] = rgb.r;
            picture.data[count++] = rgb.g;
            picture.data[count++] = rgb.b;
        }

        picture.saveWithName(filename, path);
    }
}