#pragma once

#include "nbt.hpp"
#include "zlib-1.2.12/zlib.h"


class NBTUtil {


    /*
    u8* writeCompressedTag(NBTBase& rootData, i64* outSize) {
        DataOutputManager output;
        writeTag(rootData, output);
        // compress the data
        uLong size = output.data.size();
        auto* compressedData = (Bytef*) malloc(size);
        uLongf compressedSize = size;
        compress(compressedData, &compressedSize, output.data.data(), size);
        *outSize = compressedSize;
        return compressedData;
    }


    u8* writeGZIPCompressedTag(NBTBase& rootData, i64* outSize) {
        DataOutputManager output;
        writeTag(rootData, output);
        int sizeOfUncompressed = (int) output.data.size();
        uLongf sizeOfCompressed = sizeOfUncompressed + 18;
        auto* compressedData = (u8*) malloc(sizeOfCompressed);
        def(output.data.data(), compressedData, sizeOfUncompressed, &sizeOfCompressed, 31);
        *outSize = sizeOfCompressed;
        return compressedData;
    }
     */


    /*
    UUIDJava NBTUtil::getUUIDFromTag(NBTTagCompound* tag) {
        return {tag->getPrimitive<i64>("M"), tag->getPrimitive<i64>("L")};
    }


    NBTTagCompound* NBTUtil::createUUIDTag(UUIDJava uuid) {
        auto* pNbtTagCompound = new NBTTagCompound();
        pNbtTagCompound->setLong("M", uuid.getMostSignificantBits());
        pNbtTagCompound->setLong("L", uuid.getLeastSignificantBits());
        return pNbtTagCompound;
    }
    */



    MU ND static std::tuple<int, int, int> getPosFromTag(NBTBase& nbt) {
        if (!nbt.hasKeys({"X", "Y", "Z"})) {
            return {0, 0, 0};
        }
        std::tuple<int, int, int> pos = std::make_tuple(
                nbt["X"]->get<i32>(),
                nbt["Y"]->get<i32>(),
                nbt["Z"]->get<i32>()
        );
        return pos;
    }

    MU ND static NBTBase createPosTag(int x, int y, int z) {
        return makeCompound( {
                {"X", makeInt(x)},
                {"Y", makeInt(y)},
                {"Z", makeInt(z)},
        });
    }


    MU ND static bool hasUniqueId(NBTBase& nbt, const std::string& key) {
        if (!nbt.is<NBTCompound>()) return false;

        return nbt.hasKey(key + "Most", eNBT::PRIMITIVE) &&
               nbt.hasKey(key + "Least", eNBT::PRIMITIVE);
    }






};
