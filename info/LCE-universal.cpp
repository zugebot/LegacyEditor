#include "LCE-universal.hpp"
#include "Converter/ConvertEntity.hpp"
#include "Converter/ConvertItem.hpp"
#include "Converter/ConvertSettings.hpp"
#include "Converter/ConvertStructures.hpp"
#include "Converter/ConvertTileEntity.hpp"
#include "PaletteChunkParser.hpp"


bool LCE_universal::willBeWater(uint16_t block1_13, uint8_t data) {
    if (block1_13 == 9 || block1_13 == 0x102 || block1_13 == 0x10e || block1_13 == 0x110) { return true; }
    if (block1_13 == 0x10f) {
        if (data & 8) {
            return false;//remove the sea pickle if no water is present (4th bit means it's dead (no water))
        } else {
            return true;//return water if there is water
        }
    }
    return false;//no other downgrade will result in water
}

uint8_t LCE_universal::convertTo1_12Blocks(uint16_t block1_13, uint8_t& data) {
    switch (block1_13) {
            //sea pickle
        case 0x10f:
            if (data & 8) {
                data = 0;
                return 0;//remove the sea pickle if no water is present (4th bit means dead)
            } else {
                data = 0;
                return 9;//return water if there is water
            }
            //kelp, seagrass and bubble columns can only be placed underwater, so convert them to water
        case 0x102:
        case 0x10e:
        case 0x110:
            data = 0;
            return 9;
            //convert aquatic specific blocks like type variants that can be converted to basic
            //wood blocks and stripped wood logs
        case 0x127:
        case 0x139:
            //keep the rotation? But I don't know what the rotation data is, if it's first or second, so I'm assuming it's second
            data = (data & 12) | 1;//spruce
            return 17;
        case 0x128:
        case 0x13a:
            data = (data & 12) | 2;//birch
            return 17;
        case 0x129:
        case 0x13b:
            data = (data & 12) | 3;//jungle
            return 17;
        case 0x12a:
        case 0x13c:
            data = (data & 12);//acacia
            return 162;
        case 0x12b:
        case 0x13d:
            data = (data & 12) | 1;//dark oak
            return 162;
        case 0x12c:
        case 0x13e:
            data = (data & 12);//oak
            return 17;
            //pressure plate variants
        case 0x12d:
        case 0x12e:
        case 0x12f:
        case 0x130:
        case 0x131:
            return 0x48;//return base pressure plate id
            //wood button variants
        case 0x132:
        case 0x133:
        case 0x134:
        case 0x135:
        case 0x136:
            return 0x8f;//return base wooden button id
            //trap door variants
        case 0x112:
        case 0x113:
        case 0x114:
        case 0x115:
        case 0x116:
            return 0x60;//return base trapdoor id

            //turn 2 half slab blocks to the base block and same with a single slab and stairs
        case 0x123:
            data = 0;
            return 0xa8;//return regular prismarine id
        case 0x124:
            data = 1;//return prismarine bricks
            return 0xa8;
        case 0x125:
            data = 2;//return dark prismarine
            return 0xa8;
            //prismarine double slab
        case 0x137:
        case 0x138:
            return 0xa8;//return regular prismarine id with any data it remained from either slab (the type: Prismarine, Dark Prismarine, Prismarine Bricks)
            //turn blue ice into packed ice as that is the second-best thing
        case 0x111:
            return 0xae;//return packed ice

        case 0x101://return carved pumpkin (only pumpkin before 1.13) if it is regular
            return 0x56;
        default:
            data = 0;
            return 0;
    }
}

/// check for surrounding water and the current block with water
/// if surrounded by them or is going to be replaced since it's a 1.13
uint8_t LCE_universal::convertBlockFrom1_13(LoadedChunks& adjacentChunks, BlockPos& blockPos) {
    int x = blockPos.x;
    int y = blockPos.y;
    int z = blockPos.z;
    Block east = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z);
    if (east.waterLogged || willBeWater(east.block, east.data)) { return 9; }

    Block west = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z);
    if (west.waterLogged || willBeWater(west.block, west.data)) { return 9; }

    Block south = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z + 1);
    if (south.waterLogged || willBeWater(south.block, south.data)) { return 9; }

    Block north = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z - 1);
    if (north.waterLogged || willBeWater(north.block, north.data)) { return 9; }

    if (y < 255) {
        Block up = adjacentChunks.getBlockWithDirectionPreCalculated(x, y + 1, z);
        if (up.waterLogged || willBeWater(up.block, up.data)) { return 9; }
    }

    if (y > 0) {
        Block down = adjacentChunks.getBlockWithDirectionPreCalculated(x, y - 1, z);
        if (down.waterLogged || willBeWater(down.block, down.data)) { return 9; }
    }
    return 0;
}

NBTTagList* LCE_universal::convertEntities(NBTTagList* entitiesNbt) {
    auto* convertedEntities = new NBTTagList();
    if (entitiesNbt) {
        int size = (int) entitiesNbt->tagList.size();
        for (int i = 0; i < size; i++) {
            NBTBase base = entitiesNbt->get(i);//.copy();
            auto* entity = static_cast<NBTTagCompound*>(base.data);
            NBTTagCompound* convertedEntity = EntitiesFix::convertEntity(entity);
            if (convertedEntity) { convertedEntities->appendTag(NBTBase(convertedEntity, TAG_COMPOUND)); }
        }
        NBTBase(entitiesNbt, TAG_LIST).NbtFree();
    }
    return convertedEntities;
}


NBTTagList* LCE_universal::convertTileEntities(NBTTagList* tileEntitiesNbt, UniversalChunkFormat* chunkData) {
    auto* convertedTileEntities = new NBTTagList();
    if (tileEntitiesNbt) {
        int size = (int) tileEntitiesNbt->tagList.size();
        for (int i = 0; i < size; i++) {
            NBTBase base = tileEntitiesNbt->get(i);//.copy();
            auto* tileEntity = static_cast<NBTTagCompound*>(base.data);
            NBTTagCompound* convertedTileEntity = TileEntitiesFix::convertTileEntity(tileEntity, chunkData);
            if (convertedTileEntity) { convertedTileEntities->appendTag(NBTBase(convertedTileEntity, TAG_COMPOUND)); }
        }
        NBTBase(tileEntitiesNbt, TAG_LIST).NbtFree();
    }
    return convertedTileEntities;
}

NBTTagList* LCE_universal::convertTileTicks(NBTTagList* tileTicksNbt) {
    auto* convertedTileTicks = new NBTTagList();
    if (tileTicksNbt) {
        int size = (int) tileTicksNbt->tagList.size();
        for (int i = 0; i < size; i++) {
            NBTBase base = tileTicksNbt->get(i);
            auto* tileTick = static_cast<NBTTagCompound*>(base.data);
            //if the basics don't exist then don't convert it
            if (tileTick->hasKey("x", 99) && tileTick->hasKey("y", 99) && tileTick->hasKey("z", 99) &&
                tileTick->hasKey("i", 99)) {
                auto* tileTick_out = new NBTTagCompound();
                tileTick_out->setInteger("x", tileTick->getPrimitive<int>("x"));
                tileTick_out->setInteger("y", tileTick->getPrimitive<int>("y"));
                tileTick_out->setInteger("z", tileTick->getPrimitive<int>("z"));

                tileTick_out->setInteger("p", tileTick->getPrimitive<int>("p"));
                tileTick_out->setInteger("t", tileTick->getPrimitive<int>("t"));
                int id = tileTick->getPrimitive<int>("i");
                if (ConvertSettings::flatten) {
                    PaletteBlock block = BlocksFix::getBlockFromID(id, 0);
                    if (block.properties) { NBTBase(block.properties, TAG_COMPOUND).NbtFree(); }
                    tileTick_out->setString(
                            "i", "minecraft:" + block.block);//it's just the block name we need, the data doesn't matter
                    convertedTileTicks->appendTag(NBTBase(tileTick_out, TAG_COMPOUND));
                } else if (id <= 0xff) {
                    tileTick_out->setString("i", "minecraft:" + ItemsFix::getItemFromID1_12(id).name);
                    convertedTileTicks->appendTag(NBTBase(tileTick_out, TAG_COMPOUND));
                } else {
                    NBTBase(tileTick_out, TAG_COMPOUND).NbtFree();
                }
            }
        }
        NBTBase(tileTicksNbt, TAG_LIST).NbtFree();
    }
    return convertedTileTicks;
}


void LCE_universal::convertPortal(uint8_t* dataOut, BlockWithPos& blockWithPos, LoadedChunks& chunks) {
    //0 is default meaning the game calculates the rotation
    int x = blockWithPos.pos.x;
    int y = blockWithPos.pos.y;
    int z = blockWithPos.pos.z;
    int index = blockWithPos.index;
    dataOut[index] = 2;//fix the rotation (0 is z in old versions and 1 is x and 2 is z in new versions)
    for (int amount = 1; amount < 3; amount++) {//if it should be facing the x then face it that way
        Block block = chunks.getBlockWithDirection4(x, y, z, 1, amount);//east
        if (block.block == 49) {                                        //obsidian id
            for (int amountOtherWay = 1; amountOtherWay < 3; amountOtherWay++) {
                Block block1 = chunks.getBlockWithDirection4(x, y, z, 3, amountOtherWay);// west
                if (block1.block == 49) {                                                //obsidian id
                    dataOut[index] = 1;
                    return;
                }
            }
        }
    }
}

/**
    New versions:

    Top half specifies:
    -hinge type (left/right) (1st bit)
    -powered (2nd bit) of the whole door.
    -top bit (4th bit) always on for top half

    Bottom half specifies:
    -direction (1st and 2nd bit)
    -open (3rd bit) of the whole door
    -top bit (4th bit) always off for bottom half


    Old versions:

    Top half specifies:
    -direction (1st and 2nd bit)
    -open (3rd bit) of the whole door
    -top bit (4th bit) always off for bottom half
    (the same as the bottom half)

    Bottom half specifies:
    -direction (1st and 2nd bit)
    -open (3rd bit) of the whole door
    -top bit (4th bit) always off for bottom half
*/
uint8_t LCE_universal::convertDoorNew(uint8_t data) {
    if (data & 8) {
        // if the door is open in old or powered in new because that likely won't
        // matter if the door is open in new (& 4, & 2)
        if (data & 6) {
            //then old door, because new ones don't have 3rd bit
            //if powered in new or facing west or north in old
            return 8;
        }
    }

    //the bottom half is the same for new and old
    return data;
}

uint8_t LCE_universal::convertDoorOld(uint8_t data) {
    if (data & 8) {
        return 8;// just return the top bit as powered and hinge wasn't added?
    } else {
        return data;
    }
}


void LCE_universal::convertFlowerPotData(uint8_t data, NBTTagCompound* tileEntity) {
    std::string item;
    uint8_t dataOut;
    switch (data) {
        case 1:
            item = "red_flower";
            dataOut = 0;
            break;
        case 2:
            item = "yellow_flower";
            dataOut = 0;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            item = "sapling";
            dataOut = data - 3;
            break;
        case 7:
            item = "red_mushroom";
            dataOut = 0;
            break;
        case 8:
            item = "brown_mushroom";
            dataOut = 0;
            break;
        case 9:
            item = "cactus";
            dataOut = 0;
            break;
        case 10:
            item = "deadbush";
            dataOut = 0;
            break;
        case 11:
            item = "fern";
            dataOut = 2;
            break;
        case 12:
        case 13:
            item = "sapling";
            dataOut = data - 8;
            break;
        case 0:
        default:
            item = "air";
            dataOut = 0;
            break;
    }
    tileEntity->setString("Item", "minecraft:" + item);
    tileEntity->setInteger("Data", dataOut);
}

NBTTagCompound* LCE_universal::convertTileEntityFromData(UniversalChunkFormat* currentChunk, BlockWithPos& blockWithPos,
                                                         int xPos, int zPos, const std::string& id) {
    int x = xPos * 16 + blockWithPos.pos.x;
    int y = blockWithPos.pos.y;
    int z = zPos * 16 + blockWithPos.pos.z;
    NBTTagCompound* tileEntityMatching = nullptr;
    NBTTagList* chunkTileEntities = currentChunk->tileEntities;
    int tileEntitySize = chunkTileEntities->tagCount();
    for (int i = 0; i < tileEntitySize; i++) {
        NBTTagCompound* tileEntity = chunkTileEntities->getCompoundTagAt(i);
        //don't need to check id because there can't be more than one block at the same cords
        if (tileEntity->getPrimitive<int>("x") == x && tileEntity->getPrimitive<int>("y") == y &&
            tileEntity->getPrimitive<int>("z") == z) {
            tileEntityMatching = tileEntity;
            break;
        }
    }
    if (!tileEntityMatching) {
        auto* out = new NBTTagCompound();
        out->setString("id", id);
        out->setInteger("x", x);
        out->setInteger("y", y);
        out->setInteger("z", z);
        chunkTileEntities->appendTag(NBTBase(out, TAG_COMPOUND));
        return out;
    }
    return nullptr;
}

void LCE_universal::applyFixes(UniversalChunkFormat* chunkData, LoadedChunks& adjacentChunks, LCEFixes& fixes) {
    int downgradeBlocksSize = (int) fixes.downgradingBlocks.size();
    for (int i = 0; i < downgradeBlocksSize; i++) {
        BlockWithPos blockWithPos = fixes.downgradingBlocks[i];
        uint8_t block = convertTo1_12Blocks(blockWithPos.block.block, chunkData->data[blockWithPos.index]);

        if (block == 0) {
            block = blockWithPos.block.waterLogged ? 9 : 0;
            chunkData->data[blockWithPos.index] = 0;
            if (block == 0) { block = convertBlockFrom1_13(adjacentChunks, blockWithPos.pos); }
        }
        chunkData->blocks[blockWithPos.index] = block;
    }

    int cauldronsSize = (int) fixes.cauldrons.size();
    for (int i = 0; i < cauldronsSize; i++) {
        BlockWithPos blockWithPos = fixes.cauldrons[i];
        //cauldrons in LCE have 6 levels vs java has 3
        chunkData->data[blockWithPos.index] = blockWithPos.block.data >> 1;
    }

    int chestsFixSize = (int) fixes.chests.size();
    for (int i = 0; i < chestsFixSize; i++) {
        BlockWithPos blockWithPos = fixes.chests[i];
        chunkData->data[blockWithPos.index] = alignChest(blockWithPos.pos, adjacentChunks);
    }
    int doorsFixSize = (int) fixes.doors.size();
    for (int i = 0; i < doorsFixSize; i++) {
        BlockWithPos blockWithPos = fixes.doors[i];
        if (currentVersion < 6) {
            chunkData->data[blockWithPos.index] = convertDoorOld(blockWithPos.block.data);
        } else {
            chunkData->data[blockWithPos.index] = convertDoorNew(blockWithPos.block.data);
        }
    }

    int bedsFixSize = (int) fixes.beds.size();
    for (int i = 0; i < bedsFixSize; i++) {
        BlockWithPos blockWithPos = fixes.beds[i];
        NBTTagCompound* hasTileEntityBed =
                convertTileEntityFromData(chunkData, blockWithPos, chunkData->x, chunkData->z, "minecraft:bed");
        if (hasTileEntityBed) {                       //doesn't exist because variable colour wasn't set in the function
            hasTileEntityBed->setInteger("color", 14);//red bed
        }
    }

    int flowerPotFixSize = (int) fixes.flowerPots.size();
    for (int i = 0; i < flowerPotFixSize; i++) {
        BlockWithPos blockWithPos = fixes.flowerPots[i];
        if (!ConvertSettings::flatten) {
            NBTTagCompound* titleEntityFlowerPot = convertTileEntityFromData(chunkData, blockWithPos, chunkData->x,
                                                                             chunkData->z, "minecraft:flower_pot");
            if (titleEntityFlowerPot) {
                //convert the flower inside
                convertFlowerPotData(blockWithPos.block.data, titleEntityFlowerPot);
            }
        }
        chunkData->data[blockWithPos.index] = 0;
    }
    int portalFixSize = (int) fixes.portals.size();
    for (int i = 0; i < portalFixSize; i++) {
        BlockWithPos blockWithPos = fixes.portals[i];
        convertPortal(chunkData->data, blockWithPos, adjacentChunks);
    }

    //convert entities
    chunkData->entities = convertEntities(chunkData->entities);
    //convert title entities
    chunkData->tileEntities = convertTileEntities(chunkData->tileEntities, chunkData);
    //convert tile ticks
    chunkData->tileTicks = convertTileTicks(chunkData->tileTicks);
}


void LCE_universal::checkForFixBlock(uint16_t block, uint8_t data, bool waterLogged, int x, int y, int z,
                                     LCEFixes& fixes) {
    //fix old block data or downgrade 1.13 blocks
    switch (block) {
        case 26://bed (each version doesn't have a color until the color update)
            if (oldestVersion < 10) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.beds.push_back(blockWithPos);
            }
            return;
        case 54://chest has sometime data value of 0 in old versions so set the default in current version < 3
            if (data == 0) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.chests.push_back(blockWithPos);
            }
            return;
        case 64://fix door in old versions before there was version tag?
        case 71:
            if (oldestVersion < 6) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.doors.push_back(blockWithPos);
            }
            return;
        case 90://nether portals
            if (data == 0) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.portals.push_back(blockWithPos);
            }
            return;
        case 118://cauldrons
            if (oldestVersion > 9 && data > 0) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.cauldrons.push_back(blockWithPos);
            }
            return;
        case 140://flower_pot
            if (oldestVersion < 8) {
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.flowerPots.push_back(blockWithPos);
            }
            return;
        default:
            if (block > 0xFF && !ConvertSettings::flatten) {//downgrade 1_13 blocks
                BlockWithPos blockWithPos(Block(block, data, waterLogged), BlockPos(x, y, z));
                fixes.downgradingBlocks.push_back(blockWithPos);
            }
            return;
    }
}

int LCE_universal::checkChestRotationViableBlock(uint8_t block) {
    switch (block) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 29://sticky piston (might not be valid)
        case 33://piston (might not be valid)
        case 35:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 52:
        case 53:
        case 56:
        case 57:
        case 58:
        case 61:
        case 62:
        case 67:
        case 73:
        case 74:
        case 82:
        case 84:
        case 86:
        case 87:
        case 88:
        case 89:
        case 91:
            return 1;
        case 54:
            return 2;
        default:
            return 0;
    }
}


/**
 *  -a chest placed against another one will always connect
 *
 *  if facing north-south chest (chests east/west of each other)
 *   -default face south
 *  -if one south, it will face north
 *  -if both 1>= south and north, then face south
 *
 *  if facing east-west chest (chests north/south of each other)
 *  -default face east
 *  -if one east, it will face west
 *  -if both 1>= east and west, then face west
 */
uint8_t LCE_universal::alignDoubleChest(BlockPos blockPos, LoadedChunks& adjacentChunks, int direction) {

    int x = blockPos.x;
    int y = blockPos.y;
    int z = blockPos.z;
    Block block;
    if (direction == 2 || direction == 3) {
        //facing east/west chest
        block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z);//test west block (-x)
        //test block
        int testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            return 5;// east
        }

        if (direction == 2) {                                                          //north
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z - 1);//test north west block (-x, -z)
        } else {                                                                       //south
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z + 1);//test south west block (-x, +z)
        }
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) { return 5; }

        //only face east if no blocks west
        block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z);//test east blocK (+x)
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            //face west (only if one side has blocks)
            return 4;
        }

        if (direction == 2) {                                                          // north
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z - 1);//test north-east block (+x, -z)
        } else {                                                                       // south
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z + 1);//test south-east block (+x, +z)
        }
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            //face west (only if one side has blocks)
            return 4;
        }
        return 5;//default east

    } else if (direction == 4 || direction == 5) {
        // facing north/south chest
        block = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z - 1);//test north block (-z)
        // test block
        int testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            return 3;// south
        }

        if (direction == 4) {                                                          // west
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z - 1);//test north-west block (-x, -z)
        } else {                                                                       // east
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z - 1);//test north-east block (+x, -z)
        }
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) { return 3; }

        block = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z + 1);//test south block (+z)
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            //face north (only if one side has blocks)
            return 2;
        }

        if (direction == 4) {                                                          //west
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z + 1);//test south west block (-x, +z)
        } else {                                                                       //east
            block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z + 1);//test south east block (+x, +z)
        }
        //test block
        testBlockNorth = checkChestRotationViableBlock(block.block);
        if (testBlockNorth == 1) {
            //face north (only if one side has blocks)
            return 2;
        }
        return 3;//default south
    }
    return 0;
}


uint8_t LCE_universal::alignChest(BlockPos blockPos, LoadedChunks& adjacentChunks) {
    int x = blockPos.x;
    int y = blockPos.y;
    int z = blockPos.z;
    Block block;
    uint8_t data = 3;//south
    //I think the easiest way, is defaulting to south
    //then checking the south block if there is then face north
    //then if there is one block on west/east then face the other way (not both)
    bool hasOneNorthSouthBlock = false;
    block = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z - 1);//test north block (-z)
    //test block
    int testBlockNorth = checkChestRotationViableBlock(block.block);
    if (testBlockNorth == 1) {
        hasOneNorthSouthBlock = true;
    } else if (testBlockNorth == 2) {
        return alignDoubleChest(blockPos, adjacentChunks, 2);
    }


    block = adjacentChunks.getBlockWithDirectionPreCalculated(x, y, z + 1);//test south block (+z)
    // test block
    int testBlockSouth = checkChestRotationViableBlock(block.block);
    if (testBlockSouth == 1) {
        if (!hasOneNorthSouthBlock) {
            data = 2;//face north (only if one side has blocks)
        }
    } else if (testBlockSouth == 2) {
        return alignDoubleChest(blockPos, adjacentChunks, 3);
    }


    bool hasOneEastWestBlock = false;
    uint8_t facingEastWest = 0;
    block = adjacentChunks.getBlockWithDirectionPreCalculated(x + 1, y, z);//test east block (+x)
    //test block
    int testBlockEast = checkChestRotationViableBlock(block.block);
    if (testBlockEast == 1) {
        facingEastWest = 4;// face west
        hasOneEastWestBlock = true;

    } else if (testBlockEast == 2) {
        return alignDoubleChest(blockPos, adjacentChunks, 5);
    }

    block = adjacentChunks.getBlockWithDirectionPreCalculated(x - 1, y, z);// test west block (-x)
    // test block
    int testBlockWest = checkChestRotationViableBlock(block.block);
    if (testBlockWest == 1) {
        if (!hasOneEastWestBlock) {
            facingEastWest = 5;//face east (only if one side has blocks)
            hasOneEastWestBlock = true;

        } else {
            hasOneEastWestBlock = false;
        }
    } else if (testBlockWest == 2) {
        return alignDoubleChest(blockPos, adjacentChunks, 4);
    }

    if (hasOneEastWestBlock) { data = facingEastWest; }
    return data;
}


UniversalChunkFormat* LCE_universal::convertLCEChunkToUniversal(int xChunk, int zChunk, int realChunkX, int realChunkZ,
                                                                std::vector<RegionFile>& dimData, int currentRegion,
                                                                DIMENSION dimension, LCEFixes& fixes) {
    DataInputManager input = dimData[currentRegion].regionData->getChunkDataInputStream(xChunk, zChunk);

    UniversalChunkFormat* universalData = nullptr;
    if (input.dataSize) {
        bool isNBT = input.readByte() == 0xA;
        if (isNBT) {
            input.seekStart();
            universalData = LCE_universal::convertNBTChunkToUniversal(input, dimension, fixes);
        } else {
            int version = input.readByte();
            if (version == 12) {
                AquaticParser parser;
                universalData = parser.ParseChunk(input, dimension, fixes);
            } else if (version >= 8) {
                PaletteChunkParser parser;
                universalData = parser.ParseChunk(input, dimension, fixes, version);
            } else {
                printf("Paletted version %d is not valid\n", version);
            }
        }
    } else if (Structures::structureStarts.find(ChunkPos(realChunkX, realChunkZ)) !=
               Structures::structureStarts.end()) {
        //loop through each one to make sure that there is structure in that dimension
        std::pair<std::unordered_multimap<ChunkPos, Structure>::iterator,
                  std::unordered_multimap<ChunkPos, Structure>::iterator>
                structureAt = Structures::structureStarts.equal_range(ChunkPos(realChunkX, realChunkZ));
        for (auto it = structureAt.first; it != structureAt.second; it++) {
            Structure structure = it->second;
            if (structure.dimension == dimension) {
                universalData = new UniversalChunkFormat(realChunkX, realChunkZ, dimension);
            }
        }
    }
    input.shouldFree = true;
    return universalData;
}


UniversalChunkFormat* LCE_universal::convertLCE1_13RegionToUniversal(AquaticChunkData& chunkData,
                                                                     DIMENSION dimension, LCEFixes& fixes) {
    auto* anvil = new UniversalChunkFormat(chunkData.chunkX, chunkData.chunkZ, 256, dimension);

    //used copied data to put in the data otherwise they intersect
    if (!chunkData.NBTData) { chunkData.NBTData = new NBTBase(new NBTTagCompound(), TAG_COMPOUND); }
    auto* chunkRootNbtData = static_cast<NBTTagCompound*>(chunkData.NBTData->data);
    currentChunkVersion = 12;
    anvil->version = 12;
    if (chunkRootNbtData->hasKey("Entities")) {
        anvil->entities = static_cast<NBTTagList*>(chunkRootNbtData->getTag("Entities").copy().data);
    }
    if (chunkRootNbtData->hasKey("TileEntities")) {
        anvil->tileEntities = static_cast<NBTTagList*>(chunkRootNbtData->getTag("TileEntities").copy().data);
    }
    if (chunkRootNbtData->hasKey("TileTicks")) {
        anvil->tileTicks = static_cast<NBTTagList*>(chunkRootNbtData->getTag("TileTicks").copy().data);
    }
    for (int i = 0; i < 0x10000; i++) {
        int x = (i >> 12) & 15;
        int y = i & 255;
        int z = (i >> 8) & 15;
        int index = y << 8 | x << 4 | z;//original plan
        //12 bits is the block and 4 bits is the data
        uint16_t byte1 = chunkData.blocks[i * 2];
        uint16_t byte2 = chunkData.blocks[i * 2 + 1];

        uint16_t block = ((byte2 << 4) + ((byte1 & 0xF0) >> 4)) & 0x1FF;
        uint8_t data = byte1 & 0xf;
        uint16_t byte3 = chunkData.submerged[i * 2];
        uint16_t byte4 = chunkData.submerged[i * 2 + 1];
        uint16_t submergedBlock = ((byte4 << 4) + ((byte3 & 0xF0) >> 4));
        uint8_t subData = byte3 & 0xf;
        anvil->blocks[i] = block;
        //16 128 has a submerged tag might want to check that
        anvil->data[i] = data;

        if (ConvertSettings::flatten) {
            if EXPECT_TRUE (!submergedBlock) {
                //if the last bit of the second byte (height sig value for blocks) is on then it is waterlogged
                submergedBlock = byte2 & 128;
            }
            anvil->isWaterLogged[i] = submergedBlock;
            //anvil->submergedBlocks[i] = submergedBlock;
            //anvil->submergedData[i] = subData;
        } else {
            anvil->isWaterLogged[i] = false;
        }
        //4 bits of light per block
        int lightIndex = i >> 1;
        if ((i & 1) == 0) {
            anvil->skyLight[index] = (uint8_t) chunkData.skyLight[lightIndex] & 15;
            anvil->blockLight[index] = (uint8_t) chunkData.blockLight[lightIndex] & 15;
        } else {
            anvil->skyLight[index] = (uint8_t) (chunkData.skyLight[lightIndex] >> 4) & 15;
            anvil->blockLight[index] = (uint8_t) (chunkData.blockLight[lightIndex] >> 4) & 15;
        }
        checkForFixBlock(block, data, submergedBlock, x, y, z, fixes);
    }

    if EXPECT_TRUE (!chunkData.biomes.empty()) {
        std::memcpy(anvil->biomes, chunkData.biomes.data(), 256);
        std::memcpy(anvil->heightMap, chunkData.heightMap.data(), 256);
        /*
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                anvil->biomes[x * 16 + z] = chunkData.biomes[x * 16 + z];
                anvil->heightMap[x * 16 + z] = chunkData.heightMap[x * 16 + z];
            }
        }
         */
    } else {
        if (dimension == DIMENSION::NETHER) {
            memset(anvil->biomes, 8, 256);
        } else if (dimension == DIMENSION::END) {
            memset(anvil->biomes, 9, 256);
        } else {
            memset(anvil->biomes, 1, 256);
        }//TODO: generate biomes into the chunk if the biomes tag is missing according to the world seed and settings
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) { anvil->heightMap[x * 16 + z] = chunkData.heightMap[x * 16 + z]; }
        }
    }
    anvil->lastUpdate = chunkData.lastUpdate;
    anvil->inhabitedTime = chunkData.inhabitedTime;
    anvil->terrainPopulated = chunkData.terrainPopulated != 0;
    return anvil;
}


UniversalChunkFormat* LCE_universal::convertLCE1_12RegionToUniversal(LCEChunkData& chunkData, DIMENSION dimension,
                                                                     LCEFixes& fixes) {
    auto* anvil = new UniversalChunkFormat(chunkData.chunkX, chunkData.chunkZ, 256, dimension);
    //used copied data to put in the data otherwise they intersect
    auto* chunkRootNbtData = static_cast<NBTTagCompound*>(chunkData.NBTData->data);
    //anvil->entities = getListTag("Entities", 9, chunkNbtData);
    currentChunkVersion = 11;
    anvil->version = 11;
    if (chunkRootNbtData->hasKey("Entities")) {
        anvil->entities = static_cast<NBTTagList*>(chunkRootNbtData->getTag("Entities").copy().data);
    }
    if (chunkRootNbtData->hasKey("TileEntities")) {
        anvil->tileEntities = static_cast<NBTTagList*>(chunkRootNbtData->getTag("TileEntities").copy().data);
    }
    if (chunkRootNbtData->hasKey("TileTicks")) {
        anvil->tileTicks = static_cast<NBTTagList*>(chunkRootNbtData->getTag("TileTicks").copy().data);
    }
    for (int i = 0; i < 0x10000; i++) {
        int x = (i >> 8) & 15;
        int y = i & 255;
        int z = (i >> 12) & 15;
        int index = y << 8 | z << 4 | x;
        //12 bits is the block and 4 bits is the data
        uint8_t block = chunkData.blocks[i];
        uint8_t data;
        anvil->blocks[index] = block;
        //4 bits of light per block
        int lightIndex = i >> 1;
        if ((i & 1) == 0) {
            data = (uint8_t) chunkData.data[lightIndex] & 15;
            anvil->skyLight[index] = (uint8_t) chunkData.skyLight[lightIndex] & 15;
            anvil->blockLight[index] = (uint8_t) chunkData.blockLight[lightIndex] & 15;
        } else {
            data = (uint8_t) (chunkData.data[lightIndex] >> 4) & 15;
            anvil->skyLight[index] = (uint8_t) (chunkData.skyLight[lightIndex] >> 4) & 15;
            anvil->blockLight[index] = (uint8_t) (chunkData.blockLight[lightIndex] >> 4) & 15;
        }
        anvil->data[index] = data;
        checkForFixBlock(block, data, false, x, y, z, fixes);
    }

    if EXPECT_TRUE (!chunkData.biomes.empty()) {
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                anvil->biomes[x * 16 + z] = chunkData.biomes[x * 16 + z];
                anvil->heightMap[x * 16 + z] = chunkData.heightMap[x * 16 + z];
            }
        }
    } else {
        if (dimension == DIMENSION::NETHER) {
            memset(anvil->biomes, 8, 256);
        } else if (dimension == DIMENSION::END) {
            memset(anvil->biomes, 9, 256);
        } else {
            memset(anvil->biomes, 1, 256);
        }// TODO: generate biomes into the chunk if the biomes tag is missing according to the world seed and settings
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) { anvil->heightMap[x * 16 + z] = chunkData.heightMap[x * 16 + z]; }
        }
    }
    anvil->lastUpdate = chunkData.lastUpdate;
    anvil->inhabitedTime = chunkData.inhabitedTime;
    anvil->terrainPopulated = chunkData.terrainPopulated != 0;
    return anvil;
}

UniversalChunkFormat* LCE_universal::convertNBTChunkToUniversal(DataInputManager& input, DIMENSION dimension,
                                                                LCEFixes& fixes) {
    NBTBase* chunkData = NBT::readTag(input);
    if (chunkData) {
        auto* rootTag = static_cast<NBTTagCompound*>(chunkData->data);
        NBTTagCompound* levelTag = rootTag->getCompoundTag("Level");
        if (levelTag && levelTag->hasKey("Blocks", 7) && levelTag->hasKey("Data", 7) &&
            levelTag->hasKey("BlockLight", 7) && levelTag->hasKey("SkyLight", 7) && levelTag->hasKey("HeightMap", 7)) {
            auto version = levelTag->getPrimitive<short>("version");
            int xPos = levelTag->getPrimitive<int>("xPos");
            int zPos = levelTag->getPrimitive<int>("zPos");
            auto* anvil = new UniversalChunkFormat(xPos, zPos, 256, dimension);
            NBTTagByteArray* blockTag = levelTag->getByteArray("Blocks");

            int blocksSize = blockTag->sizeOfData;
            if (!version) {
                if (levelTag->hasKey("Biomes")) {
                    version = 6;
                } else {
                    version = 1;//original
                }
            }
            currentChunkVersion = version;
            anvil->version = version;
            uint8_t* blocks = blockTag->getByteArray();
            uint8_t* data = levelTag->getByteArray("Data")->getByteArray();
            uint8_t* blockLight = levelTag->getByteArray("BlockLight")->getByteArray();
            uint8_t* skyLight = levelTag->getByteArray("SkyLight")->getByteArray();
            uint8_t* heightMap = levelTag->getByteArray("HeightMap")->getByteArray();

            if (levelTag->hasKey("Entities")) {
                anvil->entities = static_cast<NBTTagList*>(levelTag->getTag("Entities").copy().data);
            }
            if (levelTag->hasKey("TileEntities")) {
                anvil->tileEntities = static_cast<NBTTagList*>(levelTag->getTag("TileEntities").copy().data);
            }
            if (levelTag->hasKey("TileTicks")) {
                anvil->tileTicks = static_cast<NBTTagList*>(levelTag->getTag("TileTicks").copy().data);
            }
            // TODO: build height map if missing
            if (blocks && data && blockLight && skyLight && heightMap) {
                memset(anvil->isWaterLogged, 0, anvil->numBlocksInChunk());
                //the data is in the same format as 1.13 except past y128 the data is stored after 0x8000 thus it goes to y128 for both chunks of data
                for (int i = 0; i < 0x8000; i++) {
                    int x = (i >> 11) & 15;
                    int y = i & 127;
                    int z = (i >> 7) & 15;
                    int index = x << 12 | z << 8 | y;
                    //4 bits of light per block
                    int nibbleIndex = i >> 1;
                    uint8_t block = blocks[i];
                    anvil->blocks[index] = block;

                    if ((i & 1) == 0) {
                        anvil->data[index] = (uint8_t) data[nibbleIndex] & 15;
                        anvil->skyLight[index] = (uint8_t) skyLight[nibbleIndex] & 15;
                        anvil->blockLight[index] = (uint8_t) blockLight[nibbleIndex] & 15;
                    } else {
                        anvil->data[index] = (uint8_t) (data[nibbleIndex] >> 4) & 15;
                        anvil->skyLight[index] = (uint8_t) (skyLight[nibbleIndex] >> 4) & 15;
                        anvil->blockLight[index] = (uint8_t) (blockLight[nibbleIndex] >> 4) & 15;
                    }
                    checkForFixBlock(block, anvil->data[index], false, x, y, z, fixes);
                }
                if (blocksSize == 65536) {
                    for (int i = 0x8000; i < 0x10000; i++) {
                        int x = (i >> 11) & 15;
                        int y = (i & 127) + 128;
                        int z = (i >> 7) & 15;
                        int index = x << 12 | z << 8 | y;
                        //4 bits of light per block
                        int nibbleIndex = i >> 1;
                        uint8_t block = blocks[i];
                        anvil->blocks[index] = block;
                        if ((i & 1) == 0) {
                            anvil->data[index] = (uint8_t) data[nibbleIndex] & 15;
                            anvil->skyLight[index] = (uint8_t) skyLight[nibbleIndex] & 15;
                            anvil->blockLight[index] = (uint8_t) blockLight[nibbleIndex] & 15;
                        } else {
                            anvil->data[index] = (uint8_t) (data[nibbleIndex] >> 4) & 15;
                            anvil->skyLight[index] = (uint8_t) (skyLight[nibbleIndex] >> 4) & 15;
                            anvil->blockLight[index] = (uint8_t) (blockLight[nibbleIndex] >> 4) & 15;
                        }
                        checkForFixBlock(block, anvil->data[index], false, x, y, z, fixes);
                    }
                } else {
                    for (int i = 0x8000; i < 0x10000; i++) {
                        int x = (i >> 11) & 15;
                        int y = (i & 127) + 128;
                        int z = (i >> 7) & 15;
                        int index = x << 12 | z << 8 | y;
                        anvil->blocks[index] = 0;
                        anvil->data[index] = 0;
                        anvil->skyLight[index] = 0;
                        anvil->blockLight[index] = 0;
                    }
                }
                NBTTagByteArray* biomes = levelTag->getByteArray("Biomes");
                if (biomes) {
                    //dimension
                    uint8_t* biomeArray = biomes->getByteArray();
                    for (int x = 0; x < 16; x++) {
                        for (int z = 0; z < 16; z++) {
                            anvil->biomes[x * 16 + z] = biomeArray[x * 16 + z];
                            anvil->heightMap[x * 16 + z] = heightMap[x * 16 + z];
                        }
                    }
                } else {
                    if (dimension == DIMENSION::NETHER) {
                        memset(anvil->biomes, 8, 256);
                    } else if (dimension == DIMENSION::END) {
                        memset(anvil->biomes, 9, 256);

                        // TODO: generate biomes into the chunk if the biomes tag is missing
                        //  according to the world seed and settings, very later on though
                    } else {
                        memset(anvil->biomes, 1, 256);
                        //memset(anvil->biomes, 0xFF, 256);
                    }
                    for (int x = 0; x < 16; x++) {
                        for (int z = 0; z < 16; z++) { anvil->heightMap[x * 16 + z] = heightMap[x * 16 + z]; }
                    }
                }
                anvil->lastUpdate = levelTag->getPrimitive<int64_t>("LastUpdate");
                anvil->inhabitedTime = levelTag->getPrimitive<int64_t>("InhabitedTime");
                anvil->terrainPopulated = levelTag->getBool("TerrainPopulated");
                if (levelTag->hasKey("TerrainPopulatedFlags", 99)) {//not all version have "TerrainPopulated"
                    anvil->terrainPopulated = levelTag->getBool("TerrainPopulatedFlags");
                }
                chunkData->NbtFree();
                delete chunkData;
                return anvil;
            }
            //printf("Chunk at %d %d is version %d (and not programmed yet)\n", xPos, zPos, version);
        }
        chunkData->NbtFree();
        delete chunkData;
        return nullptr;
    }
    return nullptr;
}


UniversalChunkFormat* LCE_universal::convertLCEChunkToUniversalForAccess(int xChunk, int zChunk,
                                                                         std::vector<RegionFile>& dimData,
                                                                         int currentRegion, DIMENSION dimension,
                                                                         LCEFixes& fixes) {
    DataInputManager input = dimData[currentRegion].regionData->getChunkDataInputStream(xChunk, zChunk);

    UniversalChunkFormat* universalData = nullptr;
    if (input.dataSize) {
        bool isNBT = input.readByte() == 0xA;
        if (isNBT) {//quick way to check if it's nbt, which it might be for wii u before 1.12, maybe 1.12
            input.seekStart();
            universalData = LCE_universal::convertNBTChunkToUniversalForAccess(input, dimension, fixes);
        } else {
            int version = input.readByte();
            if (version == 12) {
                AquaticParser parser;
                universalData = parser.ParseChunkForAccess(input, dimension, fixes);
            } else if (version >= 8) {
                PaletteChunkParser parser;
                parser.LCE_ChunkData.version = version;
                universalData = parser.ParseChunkForAccess(input, dimension, fixes);
            } else {
                printf("Paletted version %d is not valid\n", version);
            }
        }
    }
    input.shouldFree = true;
    return universalData;
}


UniversalChunkFormat* LCE_universal::convertLCE1_13RegionToUniversalForAccess(AquaticChunkData& chunkData,
                                                                              DIMENSION dimension, LCEFixes& fixes) {
    auto* anvil = new UniversalChunkFormat(chunkData.chunkX, chunkData.chunkZ, 256, dimension);

    //used copied data to put in the data otherwise they intersect
    for (int i = 0; i < 0x10000; i++) {
        int x = (i >> 12) & 15;
        int y = i & 255;
        int z = (i >> 8) & 15;
        int index = y << 8 | x << 4 | z;//original plan
        //12 bits is the block and 4 bits is the data
        uint16_t byte1 = chunkData.blocks[i * 2];
        uint16_t byte2 = chunkData.blocks[i * 2 + 1];

        uint16_t block = ((byte2 << 4) + ((byte1 & 0xF0) >> 4)) & 0x1FF;
        uint8_t data = byte1 & 0xf;
        uint16_t byte3 = chunkData.submerged[i * 2];
        uint16_t byte4 = chunkData.submerged[i * 2 + 1];
        uint16_t submergedBlock = ((byte4 << 4) + ((byte3 & 0xF0) >> 4)) & 0x1FF;
        uint8_t subData = byte3 & 0xf;
        anvil->blocks[i] = block;

        anvil->data[i] = data;

        if (ConvertSettings::flatten) {
            if EXPECT_TRUE (!submergedBlock) {
                // if the last bit of the second byte (the highest bit value for blocks) is on then it is waterlogged
                submergedBlock = byte2 & 128;
            }
            anvil->isWaterLogged[i] = submergedBlock;
            //anvil->submergedBlocks[i] = submergedBlock;
            //anvil->submergedData[i] = subData;
        } else {
            anvil->isWaterLogged[i] = false;
        }
    }
    return anvil;
}

UniversalChunkFormat* LCE_universal::convertLCE1_12RegionToUniversalForAccess(LCEChunkData& chunkData,
                                                                              DIMENSION dimension, LCEFixes& fixes) {
    auto* anvil = new UniversalChunkFormat(chunkData.chunkX, chunkData.chunkZ, 256, dimension);
    for (int i = 0; i < 0x10000; i++) {
        int x = (i >> 8) & 15;
        int y = i & 255;
        int z = (i >> 12) & 15;
        int index = y << 8 | z << 4 | x;
        //12 bits is the block and 4 bits is the data
        uint8_t block = chunkData.blocks[i];
        uint8_t data;
        anvil->blocks[index] = block;
        //4 bits of light per block
        int lightIndex = i >> 1;
        if ((i & 1) == 0) {
            data = (uint8_t) chunkData.data[lightIndex] & 15;
        } else {
            data = (uint8_t) (chunkData.data[lightIndex] >> 4) & 15;
        }
        anvil->data[index] = data;
    }
    return anvil;
}


UniversalChunkFormat* LCE_universal::convertNBTChunkToUniversalForAccess(DataInputManager& input, DIMENSION dimension,
                                                                         LCEFixes& fixes) {
    NBTBase* chunkData = NBT::readTag(input);
    if (!chunkData) return nullptr;
    auto* rootTag = static_cast<NBTTagCompound*>(chunkData->data);
    NBTTagCompound* levelTag = rootTag->getCompoundTag("Level");
    if (levelTag && levelTag->hasKey("Blocks", 7) && levelTag->hasKey("Data", 7)) {
        int xPos = levelTag->getPrimitive<int>("xPos");
        int zPos = levelTag->getPrimitive<int>("zPos");
        auto* anvil = new UniversalChunkFormat(xPos, zPos, 256, dimension);
        NBTTagByteArray* blockTag = levelTag->getByteArray("Blocks");

        int blocksSize = blockTag->sizeOfData;
        uint8_t* blocks = blockTag->getByteArray();
        uint8_t* data = levelTag->getByteArray("Data")->getByteArray();
        // TODO: build height map if missing
        if (blocks && data) {
            memset(anvil->isWaterLogged, false, anvil->numBlocksInChunk());
            //the data is in the same format as 1.13 except past y128 the data is stored after 0x8000 thus it goes to y128 for both chunks of data
            for (int i = 0; i < 0x8000; i++) {
                int x = (i >> 11) & 15;
                int y = i & 127;
                int z = (i >> 7) & 15;
                int index = x << 12 | z << 8 | y;
                //4 bits of light per block
                int nibbleIndex = i >> 1;
                uint8_t block = blocks[i];
                anvil->blocks[index] = block;

                if ((i & 1) == 0) {
                    anvil->data[index] = (uint8_t) data[nibbleIndex] & 15;
                } else {
                    anvil->data[index] = (uint8_t) (data[nibbleIndex] >> 4) & 15;
                }
            }
            if (blocksSize == 65536) {
                for (int i = 0x8000; i < 0x10000; i++) {
                    int x = (i >> 11) & 15;
                    int y = (i & 127) + 128;
                    int z = (i >> 7) & 15;
                    int index = x << 12 | z << 8 | y;
                    //4 bits of light per block
                    int nibbleIndex = i >> 1;
                    uint8_t block = blocks[i];
                    anvil->blocks[index] = block;
                    if ((i & 1) == 0) {
                        anvil->data[index] = (uint8_t) data[nibbleIndex] & 15;
                    } else {
                        anvil->data[index] = (uint8_t) (data[nibbleIndex] >> 4) & 15;
                    }
                }
            } else {
                for (int i = 0x8000; i < 0x10000; i++) {
                    int x = (i >> 11) & 15;
                    int y = (i & 127) + 128;
                    int z = (i >> 7) & 15;
                    int index = x << 12 | z << 8 | y;
                    anvil->blocks[index] = 0;
                    anvil->data[index] = 0;
                    anvil->skyLight[index] = 0;
                    anvil->blockLight[index] = 0;
                }
            }
            chunkData->NbtFree();
            delete chunkData;
            return anvil;
        }
    }
    chunkData->NbtFree();
    delete chunkData;
    return nullptr;
}
