#include "NBT.hpp"

#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/dataManager.hpp"

#include "NBTBase.hpp"


void NBT::writeTag(NBTBase* tag, DataManager& output) {
    output.writeInt8(tag->getId());

    if (tag->getId() != NBTType::NBT_NONE) {
        output.writeUTF("");
        tag->write(output);
    }
}


NBTBase* NBT::readTag(DataManager& input) {
    NBTBase* returnValue = nullptr;
    int id = input.readInt8();
    if (id != 0) {
        std::string key = input.readUTF();
        returnValue = readNBT(static_cast<NBTType>(id), key, input);
    }
    return returnValue;
}


NBTBase* NBT::readNBT(NBTType id, const std::string& key, DataManager& input) {
    NBTBase* pNbtBase = createNewByType(id);
    pNbtBase->read(input);
    return pNbtBase;
}


/*
uint8_t* NBT::writeCompressedTag(NBTBase* rootData, i64* outSize) {
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


uint8_t* NBT::writeGZIPCompressedTag(NBTBase* rootData, i64* outSize) {
    DataOutputManager output;
    writeTag(rootData, output);
    int sizeOfUncompressed = (int) output.data.size();
    uLongf sizeOfCompressed = sizeOfUncompressed + 18;
    auto* compressedData = (uint8_t*) malloc(sizeOfCompressed);
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


BlockPos NBTUtil::getPosFromTag(NBTTagCompound* tag) {
    auto pos = BlockPos(tag->getPrimitive<int>("X"), tag->getPrimitive<int>("Y"), tag->getPrimitive<int>("Z"));
    return pos;
}


NBTTagCompound* NBTUtil::createPosTag(BlockPos pos) {
    auto* pNbtTagCompound = new NBTTagCompound();
    pNbtTagCompound->setInteger("X", pos.x);
    pNbtTagCompound->setInteger("Y", pos.y);
    pNbtTagCompound->setInteger("Z", pos.z);
    return pNbtTagCompound;
}
*/
