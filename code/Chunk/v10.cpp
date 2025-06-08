#include "v10.hpp"

#include "code/Chunk/helpers/helpers.hpp"
#include "common/nbt.hpp"


namespace editor::chunk {


    void ChunkVNBT::readChunk(DataReader& reader) {
        chunkData->intel.wasNBTChunk = true;

        NBTBase root = NBTBase::read(reader);

        auto& level = root[""]["Level"];
        if (!level.is<NBTCompound>()) return;

        chunkData->chunkX = level.getOr<i32>("xPos", 0);
        chunkData->chunkZ = level.getOr<i32>("zPos", 0);
        chunkData->lastUpdate = level.getOr<i64>("LastUpdate", 0);


        chunkData->intel.hasTerraFlagVariant = level.hasKey("TerrainPopulatedFlags");
        chunkData->terrainPopulatedFlags = level.getOr<u8>(
                "TerrainPopulated", "TerrainPopulatedFlags", 0);


        auto blocks = level.extract("Blocks");
        auto data = level.extract("Data");

        if (!blocks || !data) {
            std::cerr << "issue reading NBT Chunk\n";
            exit(-1);
        }

        u8_vec oldBlocks = std::move(blocks->get<NBTByteArray>());
        u8_vec blockData = std::move(data->get<NBTByteArray>());
        if (oldBlocks.size() == 32768) {
            chunkData->chunkHeight = 128;
        }
        chunkData->blocks = u16_vec(65536);
        for (i32 z = 0; z < 16; z++) {
            for (i32 x = 0; x < 16; x++) {
                for (i32 y = 0; y < chunkData->chunkHeight; y++) {
                    int in = toIndex<yXZy>(x, y, z);
                    int out = toIndex<CANONICAL_BLOCK_ORDER>(x, y, z);
                    chunkData->blocks[out] = oldBlocks[in] << 4 | getNibble(blockData, in);
                }
            }
        }


        chunkData->heightMap = u8_vec(256);
        if (auto heightMap = level.extract("HeightMap")) {
            chunkData->heightMap = std::move(heightMap->get<NBTByteArray>());
        }

        chunkData->biomes = u8_vec(256);
        if (auto biomes = level.extract("Biomes")) {
            chunkData->intel.hasBiomes = true;
            chunkData->biomes = std::move(biomes->get<NBTByteArray>());
        } else {
            chunkData->intel.hasBiomes = false;
        }


        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        auto skyLight = level.extract("SkyLight")->get<NBTByteArray>();
        auto blockLight = level.extract("BlockLight")->get<NBTByteArray>();
        for (i32 z = 0; z < 16; z++) {
            for (i32 x = 0; x < 16; x++) {
                for (i32 y = 0; y < chunkData->chunkHeight; y++) {
                    int in = toIndex<yXZy>(x, y, z);
                    int out = toIndex<CANONICAL_LIGHT_ORDER>(x, y, z);
                    setNibble(chunkData->skyLight, out, getNibble(skyLight, in));
                    setNibble(chunkData->blockLight, out, getNibble(blockLight, in));
                }
            }
        }
        if (chunkData->chunkHeight == 128) {
            memset(chunkData->skyLight.data() + 16384, 0xFF, 16384);
        }


        chunkData->entities = level.extract("Entities").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();
        chunkData->tileEntities = level.extract("TileEntities").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();
        chunkData->tileTicks = level.extract("TileTicks").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();

        chunkData->validChunk = true;

    }


    void ChunkVNBT::writeChunkInternal(DataWriter& writer, bool fastMode) {
        NBTCompound level;

        level["xPos"] = makeInt(chunkData->chunkX);
        level["zPos"] = makeInt(chunkData->chunkZ);
        level["LastUpdate"] = makeLong(chunkData->lastUpdate);
        level["TerrainPopulated"] = makeByte(chunkData->terrainPopulatedFlags);

        // convert blocks
        auto oldBlocks = u8_vec(65536);
        auto blockData = u8_vec(32768);
        for (i32 z = 0; z < 16; z++) {
            for (i32 x = 0; x < 16; x++) {
                for (i32 y = 0; y < chunkData->chunkHeight; y++) {
                    int in = toIndex<CANONICAL_BLOCK_ORDER>(x, y, z);
                    int out = toIndex<yXZy>(x, y, z);
                    u16 block = chunkData->blocks[in];
                    oldBlocks[out] = (block >> 4) & 255;
                    setNibble(blockData, out, block & 15);
                }
            }
        }
        level["Blocks"] = makeByteArray(NBTByteArray(
                oldBlocks.begin(),
                oldBlocks.begin() + 16 * chunkData->chunkHeight * 16));
        level["Data"] = makeByteArray(blockData);



        if (!chunkData->heightMap.empty()) {
            level["HeightMap"] = makeByteArray(chunkData->heightMap);
        }

        if (!chunkData->biomes.empty()) {
            level["Biomes"] = makeByteArray(chunkData->biomes);
        }



        auto skyLight = u8_vec(32768);
        auto blockLight = u8_vec(32768);
        for (i32 z = 0; z < 16; z++) {
            for (i32 x = 0; x < 16; x++) {
                for (i32 y = 0; y < chunkData->chunkHeight; y++) {
                    int in = toIndex<CANONICAL_LIGHT_ORDER>(x, y, z);
                    int out = toIndex<yXZy>(x, y, z);
                    setNibble(skyLight, out, getNibble(chunkData->skyLight, in));
                    setNibble(blockLight, out, getNibble(chunkData->blockLight, in));
                }
            }
        }
        if (!skyLight.empty()) {
            int size = (chunkData->chunkHeight * 16 * 16 / 2);
            level["SkyLight"] = makeByteArray(NBTByteArray(
                    skyLight.begin(), skyLight.begin() + size));
        }

        if (!blockLight.empty()) {
            int size = (chunkData->chunkHeight * 16 * 16 / 2);
            level["BlockLight"] = makeByteArray(NBTByteArray(
                    blockLight.begin(), blockLight.begin() + size));
        }



        level["Entities"] = makeList(chunkData->entities);
        level["TileEntities"] = makeList(chunkData->tileEntities);
        level["TileTicks"] = makeList(chunkData->tileTicks);


        NBTCompound root;
        root[""]["Level"] = makeCompound(std::move(level));

        NBTBase finalNBT = makeCompound(std::move(root));
        finalNBT.write(writer);
    }

}