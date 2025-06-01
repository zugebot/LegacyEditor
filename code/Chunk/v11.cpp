#include "v11.hpp"

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/helpers.hpp"
#include "common/nbt.hpp"


namespace editor::chunk {


    void ChunkV11::allocChunk() const {
        chunkData->oldBlocks = u8_vec(65536);
        chunkData->blockData = u8_vec(32768);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);
        chunkData->heightMap = u8_vec(256);
        chunkData->biomes = u8_vec(256);
    }


    // #####################################################
    // #               Read Section
    // #####################################################


    void ChunkV11::readChunk(DataReader& reader) {
        allocChunk();

        chunkData->chunkX = reader.read<i32>();
        chunkData->chunkZ = reader.read<i32>();
        chunkData->lastUpdate = reader.read<i64>();

        chunkData->DataGroupCount = 0;
        if (chunkData->lastVersion > 8) {
            chunkData->inhabitedTime = reader.read<i64>();
        }

        c_auto dataArray = fetchSections<8, true>(chunkData, reader);
        readBlocks(dataArray[0], &chunkData->oldBlocks[0]);
        readBlocks(dataArray[1], &chunkData->oldBlocks[32768]);
        readSection(dataArray[2], &chunkData->blockData[0]);
        readSection(dataArray[3], &chunkData->blockData[16384]);
        readSection(dataArray[4], &chunkData->skyLight[0]);
        readSection(dataArray[5], &chunkData->skyLight[16384]);
        readSection(dataArray[6], &chunkData->blockLight[0]);
        readSection(dataArray[7], &chunkData->blockLight[16384]);

        reader.readBytes(256, chunkData->heightMap.data());
        chunkData->terrainPopulated = reader.read<i16>();
        reader.readBytes(256, chunkData->biomes.data());

        this->readNBT(reader);

        chunkData->validChunk = true;
    }


    void ChunkV11::readBlocks(std::span<const u8> dataIn, u8* oldBlockPtr) const {
        c_u32 blockLength = dataIn.size() - GRID_COUNT * 2;
        if (blockLength > 65536) { return; } // checks for underflow

        std::span<const u8> gridHeader = {dataIn.data(), GRID_COUNT * 2};
        std::span<const u8> gridDataPtr = {dataIn.data() + GRID_COUNT * 2, blockLength};

        // 64 grids of 4x4x4 blocks, stored in XZY order
        int gridIndex = 0;
        for (i32 gridZ = 0; gridZ < 4; gridZ++) {
        for (i32 gridX = 0; gridX < 4; gridX++) {
        for (i32 gridY = 0; gridY < 32; gridY++) {
            int gridOffset = toIndex<BLOCK_ORDER>(gridX * 4, gridY * 4, gridZ * 4);

            Grid grid(
                    gridHeader[gridIndex * 2],
                    gridHeader[gridIndex * 2 + 1]
            );
            u8 blockBuffer[MAX_BLOCKS_SIZE] = {};

            if (grid.isSingleBlock()) {
                if (grid.getSingleBlock() != 0)
                    for (u8& gridIter: blockBuffer)
                        gridIter = grid.getSingleBlock();
            } else {
                // find the location of the grid's data
                c_u8* const gridOffsetPtr = gridDataPtr.data() + grid.getOffset();
                // switch over format
                switch (grid.getFormat()) {
                    case 0: readGrid<1>(gridOffsetPtr, blockBuffer); break;
                    case 1: readGrid<2>(gridOffsetPtr, blockBuffer); break;
                    case 2: readGrid<4>(gridOffsetPtr, blockBuffer); break;
                    case 3: fillAllBlocks<MAX_BLOCKS_SIZE>(gridOffsetPtr, blockBuffer); break;
                    default: return;
                }
            }
            // place the grid blocks into the chunkData
            int readOffset = 0;
            for (int z = 0; z < 4; z++) {
                for (int x = 0; x < 4; x++) {
                    for (int y = 0; y < 4; y++) {
                        c_int blockOffset = gridOffset + toIndex<BLOCK_ORDER>(x, y, z);
                        oldBlockPtr[blockOffset] = blockBuffer[readOffset++];
                    }
                }
            }
            // increment to next grid
            gridIndex++;
        }}}
    }



    template<size_t BitsPerBlock>
    bool ChunkV11::readGrid(u8 const* gridDataPtr, u8 blockBuffer[MAX_BLOCKS_SIZE]) {
        constexpr int size = 1 << BitsPerBlock;
        constexpr int blocks_per_byte = 8 / BitsPerBlock;

        u8_vec palette(size);
        std::copy_n(gridDataPtr, size, palette.begin());

        int gridIndex = 0;
        // iterates over all bytes
        for (size_t byteOffset = 0; byteOffset < 8 * BitsPerBlock; byteOffset++) {
            u16 currentByte = gridDataPtr[size + byteOffset];
            // iterates over each block in a byte
            for (u32 j = 0; j < blocks_per_byte; j++) {
                u16 paletteIndex = 0;
                // iterates over each bit in the byte, could be made faster?
                for (u32 bitPerBlock = 0; bitPerBlock < BitsPerBlock; bitPerBlock++) {
                    paletteIndex |= (currentByte & 1) << bitPerBlock;
                    currentByte >>= 1;
                }
                blockBuffer[gridIndex++] = palette[paletteIndex];
            }
        }
        return true;
    }

    // #####################################################
    // #               Write Section
    // #####################################################


    void ChunkV11::writeChunkInternal(DataWriter& writer, bool fastMode) {
        writer.write<i32>(chunkData->chunkX);
        writer.write<i32>(chunkData->chunkZ);
        writer.write<i64>(chunkData->lastUpdate);

        if (chunkData->lastVersion > 8) {
            writer.write<i64>(chunkData->inhabitedTime);
        }

        writeBlocks(writer, &chunkData->oldBlocks[0]);
        writeBlocks(writer, &chunkData->oldBlocks[32768]);

        writeSection(writer, &chunkData->blockData[0], !fastMode, false);
        writeSection(writer, &chunkData->blockData[16384], true, false);
        writeSection(writer, &chunkData->skyLight[0], !fastMode, true);
        writeSection(writer, &chunkData->skyLight[16384], true, true);
        writeSection(writer, &chunkData->blockLight[0], !fastMode, true);
        writeSection(writer, &chunkData->blockLight[16384], true, true);

        writer.writeBytes(chunkData->heightMap.data(), 256);
        writer.write<i16>(chunkData->terrainPopulated);
        writer.writeBytes(chunkData->biomes.data(), 256);

        NBTBase nbt = makeCompound();
        if (!chunkData->entities.get<NBTList>().empty()) {
            nbt["Entities"] = chunkData->entities;
        }
        nbt["TileEntities"] = chunkData->tileEntities;
        nbt["TileTicks"] = chunkData->tileTicks;

        NBTBase nbtRoot = makeCompound({
                {"", nbt}
        });
        nbtRoot.write(writer, true);
    }


    MU void ChunkV11::writeBlocks(DataWriter& writer, u8 const* oldBlockPtr) const {
        static constexpr u32 GRID_HEADER_SIZE = 2 * GRID_COUNT;

        u32 H_BEGIN = writer.tell();
        u32 H_GRID_DATA   = H_BEGIN + 4 + GRID_HEADER_SIZE;
        u32 h_grid_offset = 0;

        writer.seek(H_GRID_DATA);

        u8 blockMap[MAP_SIZE] = {};
        u8 gridHeader[GRID_HEADER_SIZE];
        i32 gridIndex = 0;

        u8FixVec_t blockVec;
        u8FixVec_t blockLoc;
        for (i32 gridZ = 0; gridZ < 4; gridZ++) {
        for (i32 gridX = 0; gridX < 4; gridX++) {
        for (i32 gridY = 0; gridY < 32; gridY++) {
            c_u32 blockOffset = toIndex<BLOCK_ORDER>(gridX * 4, gridY * 4, gridZ * 4);

            blockVec.set_size(0);
            blockLoc.set_size(0);

            for (i32 blockZ = 0; blockZ < 4; blockZ++) {
            for (i32 blockX = 0; blockX < 4; blockX++) {
            for (i32 blockY = 0; blockY < 4; blockY++) {
                c_u32 blockIndex = blockOffset + toIndex<BLOCK_ORDER>(blockX, blockY, blockZ);
                c_u8 block = oldBlockPtr[blockIndex];
                if (blockMap[block]) {
                    blockLoc.push_back(blockMap[block] - 1);
                } else {
                    blockMap[block] = blockVec.current_size() + 1;
                    u8 location = blockVec.current_size();
                    blockVec.push_back(block);
                    blockLoc.push_back(location);
                }
            }}}


            const size_t n = blockVec.current_size();
            MU V11GridFormat gridFormat;
            if (oldBlockPtr == this->chunkData->oldBlocks.data()) {
                gridFormat = V11_4_BIT;
            } else {
                gridFormat = static_cast<V11GridFormat>(n == 1 ? V11_0_BIT
                 : 32 - __builtin_clz(static_cast<unsigned>(n - 1)) - 1);
            }

            switch (gridFormat) {
                case V11_0_BIT:
                    gridHeader[gridIndex++] = Grid::IS_SINGLE_BLOCK_FLAG;
                    gridHeader[gridIndex++] = blockVec[0];
                    blockMap[blockVec[0]] = 0;
                    continue;
                case V11_1_BIT: writeGrid<1>(writer, blockVec, blockLoc, blockMap); break;
                case V11_2_BIT: writeGrid<2>(writer, blockVec, blockLoc, blockMap); break;
                case V11_3_BIT: writeGrid<3>(writer, blockVec, blockLoc, blockMap); break;
                case V11_4_BIT: {
                    for (size_t i = 0; i < GRID_COUNT; i++)
                        writer.write<u8>(blockVec[blockLoc[i]]);
                    for (c_u16 block : blockVec)
                        blockMap[block] = 0;
                    break;
                }
            }
            Grid grid;
            grid.setFormatOffset(h_grid_offset, gridFormat);
            gridHeader[gridIndex++] = static_cast<u8>(grid.data & 0xFF);
            gridHeader[gridIndex++] = static_cast<u8>((grid.data >> 8) & 0xFF);
            h_grid_offset += V11_GRID_SIZES[gridFormat];
        }}}


        u32 finalSize = GRID_HEADER_SIZE + h_grid_offset;
        writer.seek(H_BEGIN);
        writer.write<u32>(finalSize);                    //< write section size
        writer.writeBytes(gridHeader, GRID_HEADER_SIZE); //< write grid header
        writer.seek(H_GRID_DATA + h_grid_offset);        //< seek to end of data
    }


    template<size_t BitsPerBlock>
    void ChunkV11::writeGrid(DataWriter& writer,
                             u8FixVec_t& blockVector,
                             u8FixVec_t& blockLocations,
                             u8 blockMap[MAP_SIZE]) const {
        size_t total = 1 << BitsPerBlock;
        size_t count = blockVector.current_size();

        // write the palette data
        for (size_t blockIndex = 0; blockIndex < count; blockIndex++) {
            writer.write<u8>(blockVector[blockIndex]);
        }
        // fill remaining palette spots with 0xFF
        size_t remaining = total - count;
        for (size_t rest = 0; rest < remaining; rest++) {
            writer.write<u8>(0xFF);
        }

        //  write the position data
        //  so, write the first bit of each position, as a single u64,
        //  then the second, third etc. N times, where N is BitsPerBlock
        for (size_t bitIndex = 0; bitIndex < BitsPerBlock; bitIndex++) {
            u64 position = 0;
            for (size_t locIndex = 0; locIndex < GRID_COUNT; locIndex++) {
                c_u64 pos = blockLocations[locIndex];
                position |= (pos >> bitIndex & 1U) << (GRID_COUNT - locIndex - 1);
            }
            writer.write<u64>(position);
        }

        // clear the table
        for (size_t i = 0; i < count; ++i) {
            blockMap[blockVector[i]] = 0;
        }
    }

} // namespace editor::chunk