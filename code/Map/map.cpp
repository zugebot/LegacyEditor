#include "map.hpp"

#include "include/lce/include/picture.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "code/Map/mapcolors.hpp"
#include "common/nbt.hpp"


namespace editor::map {


    MU void saveMapToPng(const LCEFile* map,
                         const fs::path& filename) {
        static constexpr int MAP_BYTE_SIZE = 16384;

        if (map->m_data.empty()) {
            return;
        }

        DataReader mapManager(map->m_data.data(), map->m_data.size());
        NBTBase data;
        data.read(mapManager);
        auto byteArray = data
                .value<NBTCompound>("data")
                .value_or(NBTCompound{})
                .value<NBTByteArray>("colors")
                .value_or(NBTByteArray(16384))
                ;

        const Picture picture(128, 128);
        int count = 0;
        for (int i = 0; i < MAP_BYTE_SIZE; i++) {
            const RGB rgb = getRGB(byteArray[i]);
            picture.m_data[count++] = rgb.r;
            picture.m_data[count++] = rgb.g;
            picture.m_data[count++] = rgb.b;
        }

        picture.saveWithName(filename.string());
    }
}