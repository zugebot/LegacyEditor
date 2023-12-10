#pragma once

#include <iostream>

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/mapcolors.hpp"
#include "LegacyEditor/utils/picture.hpp"
#include "LegacyEditor/utils/file.hpp"


MU void saveMapToPng(File* map,
                     const std::string& path,
                     const std::string& filename = "map_0.png") {
    DataManager mapManager(map->data);
    auto data = NBT::readTag(mapManager);
    auto* byteArray = NBTBase::toType<NBTTagCompound>(data)->getCompoundTag("data")->getByteArray("colors");
    std::cout << byteArray->size << std::endl;

    Picture picture(128, 128);
    int count = 0;
    for (int i = 0; i < 16384; i++) {
        RGB rgb = getRGB(byteArray->array[i]);
        picture.data[count++] = rgb.r;
        picture.data[count++] = rgb.g;
        picture.data[count++] = rgb.b;
    }

    picture.saveWithName(filename, path);
}