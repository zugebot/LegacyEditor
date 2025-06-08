#include "map.hpp"

#include "include/lce/include/picture.hpp"

#include "common/nbt.hpp"
#include "code/LCEFile/LCEFile.hpp"
#include "code/Impl/mapcolors.hpp"


namespace editor::map {


    MU void saveMapToPng(const LCEFile* map,
                         const fs::path& filename) {
        static constexpr int MAP_BYTE_SIZE = 16384;

        Buffer buffer = map->getBuffer();
        if (buffer.empty()) {
            return;
        }

        DataReader reader(buffer.data(), buffer.size());
        NBTBase data = NBTBase::read(reader);
        auto byteArray = data["data"].getOr("colors", NBTByteArray(16384));

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