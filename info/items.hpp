#pragma once

#include "LegacyCubiomes/mc/item_types/Armor.hpp"
#include "LegacyCubiomes/mc/item_types/Item.hpp"
#include "LegacyCubiomes/mc/item_types/Skull.hpp"
#include "LegacyCubiomes/mc/item_types/Sword.hpp"
#include "LegacyCubiomes/mc/item_types/Tool.hpp"
#include "LegacyCubiomes/mc/item_types/Trident.hpp"


namespace Items {

    MU const Item AIR = Item(0, 0, "Air", "air");

    MU const Item STONE = Item(1, 0, "Stone", "stone");
    MU const Item GRANITE = Item(1, 1, "Granite", "stone");
    MU const Item POLISHED_GRANITE = Item(1, 2, "Polished Granite", "stone");
    MU const Item DIORITE = Item(1, 3, "Diorite", "stone");
    MU const Item POLISHED_DIORITE = Item(1, 4, "Polished Diorite", "stone");
    MU const Item ANDESITE = Item(1, 5, "Andesite", "stone");
    MU const Item POLISHED_ANDESITE = Item(1, 6, "Polished Andesite", "stone");

    MU const Item GRASS = Item(2, 0, "Grass", "grass");

    MU const Item DIRT = Item(3, 0, "Dirt", "dirt");
    MU const Item COARSE_DIRT = Item(3, 1, "Coarse Dirt", "dirt");
    MU const Item PODZOL = Item(3, 2, "Podzol", "dirt");

    MU const Item COBBLESTONE = Item(4, 0, "Cobblestone", "cobblestone");

    MU const Item OAK_WOOD_PLANK = Item(5, 0, "Oak Wood Plank", "planks");
    MU const Item SPRUCE_WOOD_PLANK = Item(5, 1, "Spruce Wood Plank", "planks");
    MU const Item BIRCH_WOOD_PLANK = Item(5, 2, "Birch Wood Plank", "planks");
    MU const Item JUNGLE_WOOD_PLANK = Item(5, 3, "Jungle Wood Plank", "planks");
    MU const Item ACACIA_WOOD_PLANK = Item(5, 4, "Acacia Wood Plank", "planks");
    MU const Item DARK_OAK_WOOD_PLANK = Item(5, 5, "Dark Oak Wood Plank", "planks");

    MU const Item OAK_SAPLING = Item(6, 0, "Oak Sapling", "sapling");
    MU const Item SPRUCE_SAPLING = Item(6, 1, "Spruce Sapling", "sapling");
    MU const Item BIRCH_SAPLING = Item(6, 2, "Birch Sapling", "sapling");
    MU const Item JUNGLE_SAPLING = Item(6, 3, "Jungle Sapling", "sapling");
    MU const Item ACACIA_SAPLING = Item(6, 4, "Acacia Sapling", "sapling");
    MU const Item DARK_OAK_SAPLING = Item(6, 5, "Dark Oak Sapling", "sapling");

    MU const Item BEDROCK = Item(7, 0, "Bedrock", "bedrock");
    MU const Item FLOWING_WATER = Item(8, 0, "Flowing Water", "flowing_water");
    MU const Item STILL_WATER = Item(9, 0, "Still Water", "water");
    MU const Item FLOWING_LAVA = Item(10, 0, "Flowing Lava", "flowing_lava");
    MU const Item STILL_LAVA = Item(11, 0, "Still Lava", "lava");

    MU const Item SAND = Item(12, 0, "Sand", "sand");
    MU const Item RED_SAND = Item(12, 1, "Red Sand", "sand");

    MU const Item GRAVEL = Item(13, 0, "Gravel", "gravel");
    MU const Item GOLD_ORE = Item(14, 0, "Gold Ore", "gold_ore");
    MU const Item IRON_ORE = Item(15, 0, "Iron Ore", "iron_ore");
    MU const Item COAL_ORE = Item(16, 0, "Coal Ore", "coal_ore");

    MU const Item OAK_WOOD = Item(17, 0, "Oak Wood", "log");
    MU const Item SPRUCE_WOOD = Item(17, 1, "Spruce Wood", "log");
    MU const Item BIRCH_WOOD = Item(17, 2, "Birch Wood", "log");
    MU const Item JUNGLE_WOOD = Item(17, 3, "Jungle Wood", "log");

    MU const Item OAK_LEAVES = Item(18, 0, "Oak Leaves", "leaves");
    MU const Item SPRUCE_LEAVES = Item(18, 1, "Spruce Leaves", "leaves");
    MU const Item BIRCH_LEAVES = Item(18, 2, "Birch Leaves", "leaves");
    MU const Item JUNGLE_LEAVES = Item(18, 3, "Jungle Leaves", "leaves");

    MU const Item SPONGE = Item(19, 0, "Sponge", "sponge");
    MU const Item WET_SPONGE = Item(19, 1, "Wet Sponge", "sponge");

    MU const Item GLASS = Item(20, 0, "Glass", "glass");
    MU const Item LAPIS_LAZULI_ORE = Item(21, 0, "Lapis Lazuli Ore", "lapis_ore");
    MU const Item LAPIS_LAZULI_BLOCK = Item(22, 0, "Lapis Lazuli Block", "lapis_block");
    MU const Item DISPENSER = Item(23, 0, "Dispenser", "dispenser");

    MU const Item SANDSTONE = Item(24, 0, "Sandstone", "sandstone");
    MU const Item CHISELED_SANDSTONE = Item(24, 1, "Chiseled Sandstone", "sandstone");
    MU const Item SMOOTH_SANDSTONE = Item(24, 2, "Smooth Sandstone", "sandstone");

    MU const Item NOTE_BLOCK = Item(25, 0, "Note Block", "noteblock");
    MU const Item BED_BLOCK = Item(26, 0, "Bed Block", "bed");
    MU const Item POWERED_RAIL = Item(27, 0, "Powered Rail", "golden_rail");
    MU const Item DETECTOR_RAIL = Item(28, 0, "Detector Rail", "detector_rail");
    MU const Item STICKY_PISTON = Item(29, 0, "Sticky Piston", "sticky_piston");
    MU const Item COBWEB = Item(30, 0, "Cobweb", "web");

    MU const Item TALL_GRASS_SHRUB = Item(31, 0, "Shrub", "tallgrass");
    MU const Item TALL_GRASS_GRASS = Item(31, 1, "Grass", "tallgrass");
    MU const Item TALL_GRASS_FERN = Item(31, 2, "Fern", "tallgrass");

    MU const Item DEAD_BUSH = Item(32, 0, "Dead Bush", "deadbush");

    MU const Item PISTON = Item(33, 0, "Piston", "piston");
    MU const Item PISTON_HEAD = Item(34, 0, "Piston Head", "piston_head");

    MU const Item WHITE_WOOL = Item(35, 0, "White Wool", "wool");
    MU const Item ORANGE_WOOL = Item(35, 1, "Orange Wool", "wool");
    MU const Item MAGENTA_WOOL = Item(35, 2, "Magenta Wool", "wool");
    MU const Item LIGHT_BLUE_WOOL = Item(35, 3, "Light Blue Wool", "wool");
    MU const Item YELLOW_WOOL = Item(35, 4, "Yellow Wool", "wool");
    MU const Item LIME_WOOL = Item(35, 5, "Lime Wool", "wool");
    MU const Item PINK_WOOL = Item(35, 6, "Pink Wool", "wool");
    MU const Item GRAY_WOOL = Item(35, 7, "Gray Wool", "wool");
    MU const Item LIGHT_GRAY_WOOL = Item(35, 8, "Light Gray Wool", "wool");
    MU const Item CYAN_WOOL = Item(35, 9, "Cyan Wool", "wool");
    MU const Item PURPLE_WOOL = Item(35, 10, "Purple Wool", "wool");
    MU const Item BLUE_WOOL = Item(35, 11, "Blue Wool", "wool");
    MU const Item BROWN_WOOL = Item(35, 12, "Brown Wool", "wool");
    MU const Item GREEN_WOOL = Item(35, 13, "Green Wool", "wool");
    MU const Item RED_WOOL = Item(35, 14, "Red Wool", "wool");
    MU const Item BLACK_WOOL = Item(35, 15, "Black Wool", "wool");

    MU const Item DANDELION = Item(37, 0, "Dandelion", "yellow_flower");

    MU const Item POPPY = Item(38, 0, "Poppy", "red_flower");
    MU const Item BLUE_ORCHID = Item(38, 1, "Blue Orchid", "red_flower");
    MU const Item ALLIUM = Item(38, 2, "Allium", "red_flower");
    MU const Item AZURE_BLUET = Item(38, 3, "Azure Bluet", "red_flower");
    MU const Item RED_TULIP = Item(38, 4, "Red Tulip", "red_flower");
    MU const Item ORANGE_TULIP = Item(38, 5, "Orange Tulip", "red_flower");
    MU const Item WHITE_TULIP = Item(38, 6, "White Tulip", "red_flower");
    MU const Item PINK_TULIP = Item(38, 7, "Pink Tulip", "red_flower");
    MU const Item OXEYE_DAISY = Item(38, 8, "Oxeye Daisy", "red_flower");

    MU const Item BROWN_MUSHROOM = Item(39, 0, "Brown Mushroom", "brown_mushroom");
    MU const Item RED_MUSHROOM = Item(40, 0, "Red Mushroom", "red_mushroom");
    MU const Item GOLD_BLOCK = Item(41, 0, "Gold Block", "gold_block");
    MU const Item IRON_BLOCK = Item(42, 0, "Iron Block", "iron_block");

    MU const Item DOUBLE_STONE_SLAB = Item(43, 0, "Double Stone Slab", "double_stone_slab");
    MU const Item DOUBLE_SANDSTONE_SLAB = Item(43, 1, "Double Sandstone Slab", "double_stone_slab");
    MU const Item DOUBLE_WOODEN_SLAB = Item(43, 2, "Double Wooden Slab", "double_stone_slab");
    MU const Item DOUBLE_COBBLESTONE_SLAB = Item(43, 3, "Double Cobblestone Slab", "double_stone_slab");
    MU const Item DOUBLE_BRICK_SLAB = Item(43, 4, "Double Brick Slab", "double_stone_slab");
    MU const Item DOUBLE_STONE_BRICK_SLAB = Item(43, 5, "Double Stone Brick Slab", "double_stone_slab");
    MU const Item DOUBLE_NETHER_BRICK_SLAB = Item(43, 6, "Double Nether Brick Slab", "double_stone_slab");
    MU const Item DOUBLE_QUARTZ_SLAB = Item(43, 7, "Double Quartz Slab", "double_stone_slab");
    MU const Item SMOOTH_DOUBLE_STONE_SLAB = Item(43, 8, "Smooth Double Stone Slab", "double_stone_slab");
    MU const Item SMOOTH_DOUBLE_SANDSTONE_SLAB = Item(43, 9, "Smooth Double Sandstone Slab", "double_stone_slab");
    MU const Item TILE_DOUBLE_QUARTZ_SLAB = Item(43, 15, "Tile Double Quartz Slab", "double_stone_slab");

    MU const Item STONE_SLAB = Item(44, 0, "Stone Slab", "stone_slab");
    MU const Item SANDSTONE_SLAB = Item(44, 1, "Sandstone Slab", "stone_slab");
    MU const Item WOODEN_SLAB = Item(44, 2, "Wooden Slab", "stone_slab");
    MU const Item COBBLESTONE_SLAB = Item(44, 3, "Cobblestone Slab", "stone_slab");
    MU const Item BRICK_SLAB = Item(44, 4, "Brick Slab", "stone_slab");
    MU const Item STONE_BRICK_SLAB = Item(44, 5, "Stone Brick Slab", "stone_slab");
    MU const Item NETHER_STONE_SLAB = Item(44, 6, "Nether Brick Slab", "stone_slab");
    MU const Item QUARTZ_SLAB = Item(44, 7, "Quartz Slab", "stone_slab");
    MU const Item UPPER_STONE_SLAB = Item(44, 8, "Upper Stone Slab", "stone_slab");
    MU const Item UPPER_SANDSTONE_SLAB = Item(44, 9, "Upper Sandstone Slab", "stone_slab");
    MU const Item UPPER_WOODEN_SLAB = Item(44, 10, "Upper Wooden Slab", "stone_slab");
    MU const Item UPPER_COBBLESTONE_SLAB = Item(44, 11, "Upper Cobblestone Slab", "stone_slab");
    MU const Item UPPER_BRICKS_SLAB = Item(44, 12, "Upper Bricks Slab", "stone_slab");
    MU const Item UPPER_STONE_BRICK_SLAB = Item(44, 13, "Upper Stone Brick Slab", "stone_slab");
    MU const Item UPPER_NETHER_BRICK_SLAB = Item(44, 14, "Upper Nether Brick Slab", "stone_slab");
    MU const Item UPPER_QUARTZ_SLAB = Item(44, 15, "Upper Quartz Slab", "stone_slab");

    MU const Item BRICKS = Item(45, 0, "Bricks", "brick_block");
    MU const Item TNT = Item(46, 0, "TNT", "tnt");
    MU const Item BOOKSHELF = Item(47, 0, "Bookshelf", "bookshelf");
    MU const Item MOSS_STONE = Item(48, 0, "Moss Stone", "mossy_cobblestone");
    MU const Item OBSIDIAN = Item(49, 0, "Obsidian", "obsidian");
    MU const Item TORCH = Item(50, 0, "Torch", "torch");
    MU const Item FIRE = Item(51, 0, "Fire", "fire");
    MU const Item MONSTER_SPAWNER = Item(52, 0, "Monster Spawner", "mob_spawner");
    MU const Item OAK_WOOD_STAIRS = Item(53, 0, "Oak Wood Stairs", "oak_stairs");
    MU const Item CHEST = Item(54, 0, "Chest", "chest");
    MU const Item REDSTONE_WIRE = Item(55, 0, "Redstone Wire", "redstone_wire");
    MU const Item DIAMOND_ORE = Item(56, 0, "Diamond Ore", "diamond_ore");
    MU const Item DIAMOND_BLOCK = Item(57, 0, "Diamond Block", "diamond_block");
    MU const Item CRAFTING_TABLE = Item(58, 0, "Crafting Table", "crafting_table");
    MU const Item WHEAT_CROPS = Item(59, 0, "Wheat Crops", "wheat");
    MU const Item FARMLAND = Item(60, 0, "Farmland", "farmland");
    MU const Item FURNACE = Item(61, 0, "Furnace", "furnace");
    MU const Item BURNING_FURNACE = Item(62, 0, "Burning Furnace", "lit_furnace");
    MU const Item STANDING_SIGN_BLOCK = Item(63, 0, "Standing Sign Block", "standing_sign");
    MU const Item OAK_DOOR_BLOCK = Item(64, 0, "Oak Door Block", "wooden_door");
    MU const Item LADDER = Item(65, 0, "Ladder", "ladder");
    MU const Item RAIL = Item(66, 0, "Rail", "rail");
    MU const Item COBBLESTONE_STAIRS = Item(67, 0, "Cobblestone Stairs", "stone_stairs");
    MU const Item WALL_MOUNTED_SIGN_BLOCK = Item(68, 0, "Wall-mounted Sign Block", "wall_sign");
    MU const Item LEVER = Item(69, 0, "Lever", "lever");
    MU const Item STONE_PRESSURE_PLATE = Item(70, 0, "Stone Pressure Plate", "stone_pressure_plate");
    MU const Item IRON_DOOR_BLOCK = Item(71, 0, "Iron Door Block", "iron_door");
    MU const Item WOODEN_PRESSURE_PLATE = Item(72, 0, "Wooden Pressure Plate", "wooden_pressure_plate");
    MU const Item REDSTONE_ORE = Item(73, 0, "Redstone Ore", "redstone_ore");
    MU const Item GLOWING_REDSTONE_ORE = Item(74, 0, "Glowing Redstone Ore", "lit_redstone_ore");
    MU const Item OFF_REDSTONE_TORCH = Item(75, 0, "Redstone Torch (off)", "unlit_redstone_torch");
    MU const Item ON_REDSTONE_TORCH = Item(76, 0, "Redstone Torch (on)", "redstone_torch");
    MU const Item STONE_BUTTON = Item(77, 0, "Stone Button", "stone_button");
    MU const Item SNOW = Item(78, 0, "Snow", "snow_layer");
    MU const Item ICE = Item(79, 0, "Ice", "ice");
    MU const Item SNOW_BLOCK = Item(80, 0, "Snow Block", "snow");
    MU const Item CACTUS = Item(81, 0, "Cactus", "cactus");
    MU const Item CLAY_BLOCK = Item(82, 0, "Clay", "clay");
    MU const Item SUGAR_CANES = Item(83, 0, "Sugar Canes", "reeds");
    MU const Item JUKEBOX = Item(84, 0, "Jukebox", "jukebox");
    MU const Item OAK_FENCE = Item(85, 0, "Oak Fence", "fence");
    MU const Item PUMPKIN = Item(86, 0, "Pumpkin", "pumpkin");
    MU const Item NETHERRACK = Item(87, 0, "Netherrack", "netherrack");
    MU const Item SOUL_SAND = Item(88, 0, "Soul Sand", "soul_sand");
    MU const Item GLOWSTONE = Item(89, 0, "Glowstone", "glowstone");
    MU const Item NETHER_PORTAL = Item(90, 0, "Nether Portal", "portal");
    MU const Item JACK_O_LANTERN = Item(91, 0, "Jack o’Lantern", "lit_pumpkin");
    MU const Item CAKE_BLOCK = Item(92, 0, "Cake Block", "cake");
    MU const Item OFF_REDSTONE_REPEATER_BLOCK = Item(93, 0, "Redstone Repeater Block (off)", "unpowered_repeater");
    MU const Item ON_REDSTONE_REPEATER_BLOCK = Item(94, 0, "Redstone Repeater Block (on)", "powered_repeater");

    MU const Item WHITE_STAINED_GLASS = Item(95, 0, "White Stained Glass", "stained_glass");
    MU const Item ORANGE_STAINED_GLASS = Item(95, 1, "Orange Stained Glass", "stained_glass");
    MU const Item MAGENTA_STAINED_GLASS = Item(95, 2, "Magenta Stained Glass", "stained_glass");
    MU const Item LIGHT_BLUE_STAINED_GLASS = Item(95, 3, "Light Blue Stained Glass", "stained_glass");
    MU const Item YELLOW_STAINED_GLASS = Item(95, 4, "Yellow Stained Glass", "stained_glass");
    MU const Item LIME_STAINED_GLASS = Item(95, 5, "Lime Stained Glass", "stained_glass");
    MU const Item PINK_STAINED_GLASS = Item(95, 6, "Pink Stained Glass", "stained_glass");
    MU const Item GRAY_STAINED_GLASS = Item(95, 7, "Gray Stained Glass", "stained_glass");
    MU const Item LIGHT_GRAY_STAINED_GLASS = Item(95, 8, "Light Gray Stained Glass", "stained_glass");
    MU const Item CYAN_STAINED_GLASS = Item(95, 9, "Cyan Stained Glass", "stained_glass");
    MU const Item PURPLE_STAINED_GLASS = Item(95, 10, "Purple Stained Glass", "stained_glass");
    MU const Item BLUE_STAINED_GLASS = Item(95, 11, "Blue Stained Glass", "stained_glass");
    MU const Item BROWN_STAINED_GLASS = Item(95, 12, "Brown Stained Glass", "stained_glass");
    MU const Item GREEN_STAINED_GLASS = Item(95, 13, "Green Stained Glass", "stained_glass");
    MU const Item RED_STAINED_GLASS = Item(95, 14, "Red Stained Glass", "stained_glass");
    MU const Item BLACK_STAINED_GLASS = Item(95, 15, "Black Stained Glass", "stained_glass");

    MU const Item WOODEN_TRAPDOOR = Item(96, 0, "Wooden Trapdoor", "trapdoor");

    MU const Item STONE_MONSTER_EGG = Item(97, 0, "Stone Monster Egg", "monster_egg");
    MU const Item COBBLESTONE_MONSTER_EGG = Item(97, 1, "Cobblestone Monster Egg", "monster_egg");
    MU const Item STONE_BRICK_MONSTER_EGG = Item(97, 2, "Stone Brick Monster Egg", "monster_egg");
    MU const Item MOSSY_STONE_MONSTER_EGG = Item(97, 3, "Mossy Stone Brick Monster Egg", "monster_egg");
    MU const Item CRACKED_STONE_BRICK_MONSTER_EGG = Item(97, 4, "Cracked Stone Brick Monster Egg", "monster_egg");
    MU const Item CHISELED_STONE_BRICK_MONSTER_EGG = Item(97, 5, "Chiseled Stone Brick Monster Egg", "monster_egg");

    MU const Item STONE_BRICKS = Item(98, 0, "Stone Bricks", "stonebrick");
    MU const Item MOSSY_STONE_BRICKS = Item(98, 1, "Mossy Stone Bricks", "stonebrick");
    MU const Item CRACKED_STONE_BRICKS = Item(98, 2, "Cracked Stone Bricks", "stonebrick");
    MU const Item CHISELED_STONE_BRICKS = Item(98, 3, "Chiseled Stone Bricks", "stonebrick");

    MU const Item BROWN_MUSHROOM_BLOCK = Item(99, 0, "Brown Mushroom Block", "brown_mushroom_block");
    MU const Item RED_MUSHROOM_BLOCK = Item(100, 0, "Red Mushroom Block", "red_mushroom_block");
    MU const Item IRON_BARS = Item(101, 0, "Iron Bars", "iron_bars");
    MU const Item GLASS_PANE = Item(102, 0, "Glass Pane", "glass_pane");
    MU const Item MELON_BLOCK = Item(103, 0, "Melon Block", "melon_block");
    MU const Item PUMPKIN_STEM = Item(104, 0, "Pumpkin Stem", "pumpkin_stem");
    MU const Item MELON_STEM = Item(105, 0, "Melon Stem", "melon_stem");
    MU const Item VINES = Item(106, 0, "Vines", "vine");
    MU const Item OAK_FENCE_GATE = Item(107, 0, "Oak Fence Gate", "fence_gate");
    MU const Item BRICK_STAIRS = Item(108, 0, "Brick Stairs", "brick_stairs");
    MU const Item STONE_BRICK_STAIRS = Item(109, 0, "Stone Brick Stairs", "stone_brick_stairs");
    MU const Item MYCELIUM = Item(110, 0, "Mycelium", "mycelium");
    MU const Item LILY_PAD = Item(111, 0, "Lily Pad", "waterlily");
    MU const Item NETHER_BRICK_BLOCK = Item(112, 0, "Nether Brick", "nether_brick");
    MU const Item NETHER_BRICK_FENCE = Item(113, 0, "Nether Brick Fence", "nether_brick_fence");
    MU const Item NETHER_BRICK_STAIRS = Item(114, 0, "Nether Brick Stairs", "nether_brick_stairs");
    MU const Item NETHER_WART = Item(115, 0, "Nether Wart", "nether_wart");
    MU const Item ENCHANTMENT_TABLE = Item(116, 0, "Enchantment Table", "enchanting_table");
    MU const Item BREWING_STAND = Item(117, 0, "Brewing Stand", "brewing_stand");
    MU const Item CAULDRON = Item(118, 0, "Cauldron", "cauldron");
    MU const Item END_PORTAL = Item(119, 0, "End Portal", "end_portal");
    MU const Item END_PORTAL_FRAME = Item(120, 0, "End Portal Frame", "end_portal_frame");
    MU const Item END_STONE = Item(121, 0, "End Stone", "end_stone");
    MU const Item DRAGON_EGG = Item(122, 0, "Dragon Egg", "dragon_egg");
    MU const Item INACTIVE_REDSTONE_LAMP = Item(123, 0, "Redstone Lamp (inactive)", "redstone_lamp");
    MU const Item ACTIVE_REDSTONE_LAMP = Item(124, 0, "Redstone Lamp (active)", "lit_redstone_lamp");

    MU const Item DOUBLE_OAK_WOOD_SLAB = Item(125, 0, "Double Oak Wood Slab", "double_wooden_slab");
    MU const Item DOUBLE_SPRUCE_WOOD_SLAB = Item(125, 1, "Double Spruce Wood Slab", "double_wooden_slab");
    MU const Item DOUBLE_BIRCH_WOOD_SLAB = Item(125, 2, "Double Birch Wood Slab", "double_wooden_slab");
    MU const Item DOUBLE_JUNGLE_WOOD_SLAB = Item(125, 3, "Double Jungle Wood Slab", "double_wooden_slab");
    MU const Item DOUBLE_ACACIA_WOOD_SLAB = Item(125, 4, "Double Acacia Wood Slab", "double_wooden_slab");
    MU const Item DOUBLE_DARK_OAK_WOOD_SLAB = Item(125, 5, "Double Dark Oak Wood Slab", "double_wooden_slab");

    MU const Item OAK_WOOD_SLAB = Item(126, 0, "Oak Wood Slab", "wooden_slab");
    MU const Item SPRUCE_WOOD_SLAB = Item(126, 1, "Spruce Wood Slab", "wooden_slab");
    MU const Item BIRCH_WOOD_SLAB = Item(126, 2, "Birch Wood Slab", "wooden_slab");
    MU const Item JUNGLE_WOOD_SLAB = Item(126, 3, "Jungle Wood Slab", "wooden_slab");
    MU const Item ACACIA_WOOD_SLAB = Item(126, 4, "Acacia Wood Slab", "wooden_slab");
    MU const Item DARK_OAK_WOOD_SLAB = Item(126, 5, "Dark Wood Slab", "wooden_slab");
    MU const Item UPPER_OAK_WOOD_SLAB = Item(126, 8, "Upper Oak Wood Slab", "wooden_slab");
    MU const Item UPPER_SPRUCE_WOOD_SLAB = Item(126, 9, "Upper Spruce Wood Slab", "wooden_slab");
    MU const Item UPPER_BIRCH_WOOD_SLAB = Item(126, 10, "Upper Birch Wood Slab", "wooden_slab");
    MU const Item UPPER_JUNGLE_WOOD_SLAB = Item(126, 11, "Upper Jungle Wood Slab", "wooden_slab");
    MU const Item UPPER_ACACIA_WOOD_SLAB = Item(126, 12, "Upper Acacia Wood Slab", "wooden_slab");
    MU const Item UPPER_DARK_OAK_WOOD_SLAB = Item(126, 13, "Upper Dark Wood Slab", "wooden_slab");

    MU const Item COCOA = Item(127, 0, "Cocoa", "cocoa");
    MU const Item SANDSTONE_STAIRS = Item(128, 0, "Sandstone Stairs", "sandstone_stairs");
    MU const Item EMERALD_ORE = Item(129, 0, "Emerald Ore", "emerald_ore");
    MU const Item ENDER_CHEST = Item(130, 0, "Ender Chest", "ender_chest");
    MU const Item TRIPWIRE_HOOK = Item(131, 0, "Tripwire Hook", "tripwire_hook");
    MU const Item TRIPWIRE = Item(132, 0, "Tripwire", "tripwire_hook");
    MU const Item EMERALD_BLOCK = Item(133, 0, "Emerald Block", "emerald_block");
    MU const Item SPRUCE_WOOD_STAIRS = Item(134, 0, "Spruce Wood Stairs", "spruce_stairs");
    MU const Item BIRCH_WOOD_STAIRS = Item(135, 0, "Birch Wood Stairs", "birch_stairs");
    MU const Item JUNGLE_WOOD_STAIRS = Item(136, 0, "Jungle Wood Stairs", "jungle_stairs");
    MU const Item COMMAND_BLOCK = Item(137, 0, "Command Block", "command_block");
    MU const Item BEACON = Item(138, 0, "Beacon", "beacon");

    MU const Item COBBLESTONE_WALL = Item(139, 0, "Cobblestone Wall", "cobblestone_wall");
    MU const Item MOSSY_COBBLESTONE_WALL = Item(139, 1, "Mossy Cobblestone Wall", "cobblestone_wall");

    MU const Item FLOWER_POT = Item(140, 0, "Flower Pot", "flower_pot");
    MU const Item CARROTS = Item(141, 0, "Carrots", "carrots");
    MU const Item POTATOES = Item(142, 0, "Potatoes", "potatoes");
    MU const Item WOODEN_BUTTON = Item(143, 0, "Wooden Button", "wooden_button");

    MU const Item MOB_HEAD = Item(144, 0, "Mob Head", "skull");

    MU const Item ANVIL = Item(145, 0, "Anvil", "anvil");
    MU const Item SLIGHTLY_DAMAGED_ANVIL = Item(145, 1, "Slightly Damaged Anvil", "anvil");
    MU const Item VERY_DAMAGED_ANVIL = Item(145, 2, "Very Damaged Anvil", "anvil");

    MU const Item TRAPPED_CHEST = Item(146, 0, "Trapped Chest", "trapped_chest");
    MU const Item LIGHT_WEIGHTED_PRESSURE_PLATE =
            Item(147, 0, "Weighted Pressure Plate (light)", "light_weighted_pressure_plate");
    MU const Item HEAVY_WEIGHTED_PRESSURE_PLATE =
            Item(148, 0, "Weighted Pressure Plate (heavy)", "heavy_weighted_pressure_plate");
    MU const Item INACTIVE_REDSTONE_COMPARATOR = Item(149, 0, "Redstone Comparator (inactive)", "unpowered_comparator");
    MU const Item ACTIVE_REDSTONE_COMPARATOR = Item(150, 0, "Redstone Comparator (active)", "powered_comparator");
    MU const Item DAYLIGHT_SENSOR = Item(151, 0, "Daylight Sensor", "daylight_detector");
    MU const Item REDSTONE_BLOCK = Item(152, 0, "Redstone Block", "redstone_block");
    MU const Item NETHER_QUARTZ_ORE = Item(153, 0, "Nether Quartz Ore", "quartz_ore");
    MU const Item HOPPER = Item(154, 0, "Hopper", "hopper");

    MU const Item QUARTZ_BLOCK = Item(155, 0, "Quartz Block", "quartz_block");
    MU const Item CHISELED_QUARTZ_BLOCK = Item(155, 1, "Chiseled Quartz Block", "quartz_block");
    MU const Item PILLAR_QUARTZ_BLOCK = Item(155, 2, "Pillar Quartz Block", "quartz_block");

    MU const Item QUARTZ_STAIRS = Item(156, 0, "Quartz Stairs", "quartz_stairs");
    MU const Item ACTIVATOR_RAIL = Item(157, 0, "Activator Rail", "activator_rail");
    MU const Item DROPPER = Item(158, 0, "Dropper", "dropper");

    MU const Item WHITE_HARDENED_CLAY = Item(159, 0, "White Hardened Clay", "stained_hardened_clay");
    MU const Item ORANGE_HARDENED_CLAY = Item(159, 1, "Orange Hardened Clay", "stained_hardened_clay");
    MU const Item MAGENTA_HARDENED_CLAY = Item(159, 2, "Magenta Hardened Clay", "stained_hardened_clay");
    MU const Item LIGHT_BLUE_HARDENED_CLAY = Item(159, 3, "Light Blue Hardened Clay", "stained_hardened_clay");
    MU const Item YELLOW_HARDENED_CLAY = Item(159, 4, "Yellow Hardened Clay", "stained_hardened_clay");
    MU const Item LIME_HARDENED_CLAY = Item(159, 5, "Lime Hardened Clay", "stained_hardened_clay");
    MU const Item PINK_HARDENED_CLAY = Item(159, 6, "Pink Hardened Clay", "stained_hardened_clay");
    MU const Item GRAY_HARDENED_CLAY = Item(159, 7, "Gray Hardened Clay", "stained_hardened_clay");
    MU const Item LIGHT_GRAY_HARDENED_CLAY = Item(159, 8, "Light Gray Hardened Clay", "stained_hardened_clay");
    MU const Item CYAN_HARDENED_CLAY = Item(159, 9, "Cyan Hardened Clay", "stained_hardened_clay");
    MU const Item PURPLE_HARDENED_CLAY = Item(159, 10, "Purple Hardened Clay", "stained_hardened_clay");
    MU const Item BLUE_HARDENED_CLAY = Item(159, 11, "Blue Hardened Clay", "stained_hardened_clay");
    MU const Item BROWN_HARDENED_CLAY = Item(159, 12, "Brown Hardened Clay", "stained_hardened_clay");
    MU const Item GREEN_HARDENED_CLAY = Item(159, 13, "Green Hardened Clay", "stained_hardened_clay");
    MU const Item RED_HARDENED_CLAY = Item(159, 14, "Red Hardened Clay", "stained_hardened_clay");
    MU const Item BLACK_HARDENED_CLAY = Item(159, 15, "Black Hardened Clay", "stained_hardened_clay");

    MU const Item WHITE_STAINED_GLASS_PANE = Item(160, 0, "White Stained Glass Pane", "stained_glass_pane");
    MU const Item ORANGE_STAINED_GLASS_PANE = Item(160, 1, "Orange Stained Glass Pane", "stained_glass_pane");
    MU const Item MAGENTA_STAINED_GLASS_PANE = Item(160, 2, "Magenta Stained Glass Pane", "stained_glass_pane");
    MU const Item LIGHT_BLUE_STAINED_GLASS_PANE = Item(160, 3, "Light Blue Stained Glass Pane", "stained_glass_pane");
    MU const Item YELLOW_STAINED_GLASS_PANE = Item(160, 4, "Yellow Stained Glass Pane", "stained_glass_pane");
    MU const Item LIME_STAINED_GLASS_PANE = Item(160, 5, "Lime Stained Glass Pane", "stained_glass_pane");
    MU const Item PINK_STAINED_GLASS_PANE = Item(160, 6, "Pink Stained Glass Pane", "stained_glass_pane");
    MU const Item GRAY_STAINED_GLASS_PANE = Item(160, 7, "Gray Stained Glass Pane", "stained_glass_pane");
    MU const Item LIGHT_GRAY_STAINED_GLASS_PANE = Item(160, 8, "Light Gray Stained Glass Pane", "stained_glass_pane");
    MU const Item CYAN_STAINED_GLASS_PANE = Item(160, 9, "Cyan Stained Glass Pane", "stained_glass_pane");
    MU const Item PURPLE_STAINED_GLASS_PANE = Item(160, 10, "Purple Stained Glass Pane", "stained_glass_pane");
    MU const Item BLUE_STAINED_GLASS_PANE = Item(160, 11, "Blue Stained Glass Pane", "stained_glass_pane");
    MU const Item BROWN_STAINED_GLASS_PANE = Item(160, 12, "Brown Stained Glass Pane", "stained_glass_pane");
    MU const Item GREEN_STAINED_GLASS_PANE = Item(160, 13, "Green Stained Glass Pane", "stained_glass_pane");
    MU const Item RED_STAINED_GLASS_PANE = Item(160, 14, "Red Stained Glass Pane", "stained_glass_pane");
    MU const Item BLACK_STAINED_GLASS_PANE = Item(160, 15, "Black Stained Glass Pane", "stained_glass_pane");

    MU const Item ACACIA_LEAVES = Item(161, 0, "Acacia Leaves", "leaves2");
    MU const Item DARK_OAK_LEAVES = Item(161, 1, "Dark Oak Leaves", "leaves2");

    MU const Item ACACIA_WOOD = Item(162, 0, "Acacia Wood", "log2");
    MU const Item DARK_OAK_WOOD = Item(162, 1, "Dark Oak Wood", "log2");

    MU const Item ACACIA_WOOD_STAIRS = Item(163, 0, "Acacia Wood Stairs", "acacia_stairs");
    MU const Item DARK_OAK_WOOD_STAIRS = Item(164, 0, "Dark Oak Wood Stairs", "dark_oak_stairs");
    MU const Item SLIME_BLOCK = Item(165, 0, "Slime Block", "slime");
    MU const Item BARRIER = Item(166, 0, "Barrier", "barrier");
    MU const Item IRON_TRAPDOOR = Item(167, 0, "Iron Trapdoor", "iron_trapdoor");

    MU const Item PRISMARINE = Item(168, 0, "Prismarine", "prismarine");
    MU const Item PRISMARINE_BRICKS = Item(168, 1, "Prismarine Bricks", "prismarine");
    MU const Item DARK_PRISMARINE = Item(168, 2, "Dark Prismarine", "prismarine");

    MU const Item SEA_LANTERN = Item(169, 0, "Sea Lantern", "sea_lantern");
    MU const Item HAY_BALE = Item(170, 0, "Hay Bale", "hay_block");

    MU const Item WHITE_CARPET = Item(171, 0, "White White Carpet", "carpet");
    MU const Item ORANGE_CARPET = Item(171, 1, "Orange White Carpet", "carpet");
    MU const Item MAGENTA_CARPET = Item(171, 2, "Magenta White Carpet", "carpet");
    MU const Item LIGHT_BLUE_CARPET = Item(171, 3, "Light Blue White Carpet", "carpet");
    MU const Item YELLOW_CARPET = Item(171, 4, "Yellow White Carpet", "carpet");
    MU const Item LIME_CARPET = Item(171, 5, "Lime White Carpet", "carpet");
    MU const Item PINK_CARPET = Item(171, 6, "Pink White Carpet", "carpet");
    MU const Item GRAY_CARPET = Item(171, 7, "Gray White Carpet", "carpet");
    MU const Item LIGHT_GRAY_CARPET = Item(171, 8, "Light Gray White Carpet", "carpet");
    MU const Item CYAN_CARPET = Item(171, 9, "Cyan White Carpet", "carpet");
    MU const Item PURPLE_CARPET = Item(171, 10, "Purple White Carpet", "carpet");
    MU const Item BLUE_CARPET = Item(171, 11, "Blue White Carpet", "carpet");
    MU const Item BROWN_CARPET = Item(171, 12, "Brown White Carpet", "carpet");
    MU const Item GREEN_CARPET = Item(171, 13, "Green White Carpet", "carpet");
    MU const Item RED_CARPET = Item(171, 14, "Red White Carpet", "carpet");
    MU const Item BLACK_CARPET = Item(171, 15, "Black White Carpet", "carpet");

    MU const Item HARDENED_CLAY = Item(172, 0, "Hardened Clay", "hardened_clay");
    MU const Item BLOCK_OF_COAL = Item(173, 0, "Block of Coal", "coal_block");
    MU const Item PACKED_ICE = Item(174, 0, "Packed Ice", "packed_ice");

    MU const Item SUNFLOWER = Item(175, 0, "Sunflower", "double_plant");
    MU const Item LILAC = Item(175, 1, "Lilac", "double_plant");
    MU const Item DOUBLE_TALLGRASS = Item(175, 2, "Double Tall Grass", "double_plant");
    MU const Item LARGE_FERN = Item(175, 3, "Large Fern", "double_plant");
    MU const Item ROSE_BUSH = Item(175, 4, "Rose Bush", "double_plant");
    MU const Item PEONY = Item(175, 5, "Peony", "double_plant");

    MU const Item FREE_STANDING_BANNER = Item(176, 0, "Free-standing Banner", "standing_banner");

    MU const Item WALL_MOUNTED_BANNER = Item(177, 0, "Wall-mounted Banner", "wall_banner");

    MU const Item INVERTED_DAYLIGHT_SENSOR = Item(178, 0, "Inverted Daylight Sensor", "daylight_detector_inverted");

    MU const Item RED_SANDSTONE = Item(179, 0, "Red Sandstone", "red_sandstone");
    MU const Item CHISELED_RED_SANDSTONE = Item(179, 1, "Chiseled Red Sandstone", "red_sandstone");
    MU const Item SMOOTH_RED_SANDSTONE = Item(179, 2, "Smooth Red Sandstone", "red_sandstone");

    MU const Item RED_SANDSTONE_STAIRS = Item(180, 0, "Red Sandstone Stairs", "red_sandstone_stairs");

    MU const Item DOUBLE_RED_SANDSTONE_SLAB = Item(181, 0, "Double Red Sandstone Slab", "double_stone_slab2");
    MU const Item SMOOTH_DOUBLE_RED_SANDSTONE_SLAB =
            Item(181, 8, "Smooth Double Red Sandstone Slab", "double_stone_slab2");

    MU const Item RED_SANDSTONE_SLAB = Item(182, 0, "Red Sandstone Slab", "stone_slab2");
    MU const Item UPPER_RED_SANDSTONE_SLAB = Item(182, 1, "Upper Red Sandstone Slab", "stone_slab2");

    MU const Item SPRUCE_FENCE_GATE = Item(183, 0, "Spruce Fence Gate", "spruce_fence_gate");
    MU const Item BIRCH_FENCE_GATE = Item(184, 0, "Birch Fence Gate", "birch_fence_gate");
    MU const Item JUNGLE_FENCE_GATE = Item(185, 0, "Jungle Fence Gate", "jungle_fence_gate");
    MU const Item DARK_OAK_FENCE_GATE = Item(186, 0, "Dark Oak Fence Gate", "dark_oak_fence_gate");
    MU const Item ACACIA_FENCE_GATE = Item(187, 0, "Acacia Fence Gate", "acacia_fence_gate");
    MU const Item SPRUCE_FENCE = Item(188, 0, "Spruce Fence", "spruce_fence");
    MU const Item BIRCH_FENCE = Item(189, 0, "Birch Fence", "birch_fence");
    MU const Item JUNGLE_FENCE = Item(190, 0, "Jungle Fence", "jungle_fence");
    MU const Item DARK_OAK_FENCE = Item(191, 0, "Dark Oak Fence", "dark_oak_fence");
    MU const Item ACACIA_FENCE = Item(192, 0, "Acacia Fence", "acacia_fence");
    MU const Item SPRUCE_DOOR_BLOCK = Item(193, 0, "Spruce Door Block", "spruce_door");
    MU const Item BIRCH_DOOR_BLOCK = Item(194, 0, "Birch Door Block", "birch_door");
    MU const Item JUNGLE_DOOR_BLOCK = Item(195, 0, "Jungle Door Block", "jungle_door");
    MU const Item ACACIA_DOOR_BLOCK = Item(196, 0, "Acacia Door Block", "acacia_door");
    MU const Item DARK_OAK_DOOR_BLOCK = Item(197, 0, "Dark Oak Door Block", "dark_oak_door");
    MU const Item END_ROD = Item(198, 0, "End Rod", "end_rod");
    MU const Item CHORUS_PLANT = Item(199, 0, "Chorus Plant", "chorus_plant");
    MU const Item CHORUS_FLOWER = Item(200, 0, "Chorus Flower", "chorus_flower");
    MU const Item PURPUR_BLOCK = Item(201, 0, "Purpur Block", "purpur_block");
    MU const Item PURPUR_PILLAR = Item(202, 0, "Purpur Pillar", "purpur_pillar");
    MU const Item PURPUR_STAIRS = Item(203, 0, "Purpur Stairs", "purpur_stairs");
    MU const Item PURPUR_DOUBLE_SLAB = Item(204, 0, "Purpur Double Slab", "purpur_double_slab");
    MU const Item PURPUR_SLAB = Item(205, 0, "Purpur Slab", "purpur_slab");
    MU const Item END_STONE_BRICKS = Item(206, 0, "End Stone Bricks", "end_bricks");
    MU const Item BEETROOT_BLOCK = Item(207, 0, "Beetroot Block", "beetroots");
    MU const Item GRASS_PATH = Item(208, 0, "Grass Path", "grass_path");
    MU const Item END_GATEWAY = Item(209, 0, "End Gateway", "end_gateway");
    MU const Item REPEATING_COMMAND_BLOCK = Item(210, 0, "Repeating Command Block", "repeating_command_block");
    MU const Item CHAIN_COMMAND_BLOCK = Item(211, 0, "Chain Command Block", "chain_command_block");
    MU const Item FROSTED_ICE = Item(212, 0, "Frosted Ice", "frosted_ice");
    MU const Item MAGMA_BLOCK = Item(213, 0, "Magma Block", "magma");
    MU const Item NETHER_WART_BLOCK = Item(214, 0, "Nether Wart Block", "nether_wart_block");
    MU const Item RED_NETHER_BRICK = Item(215, 0, "Red Nether Brick", "red_nether_brick");
    MU const Item BONE_BLOCK = Item(216, 0, "Bone Block", "bone_block");
    MU const Item STRUCTURE_VOID = Item(217, 0, "Structure Void", "structure_void");
    MU const Item OBSERVER = Item(218, 0, "Observer", "observer");
    MU const Item WHITE_SHULKER_BOX = Item(219, 0, "White Shulker Box", "white_shulker_box");
    MU const Item ORANGE_SHULKER_BOX = Item(220, 0, "Orange Shulker Box", "orange_shulker_box");
    MU const Item MAGENTA_SHULKER_BOX = Item(221, 0, "Magenta Shulker Box", "magenta_shulker_box");
    MU const Item LIGHT_BLUE_SHULKER_BOX = Item(222, 0, "Light Blue Shulker Box", "light_blue_shulker_box");
    MU const Item YELLOW_SHULKER_BOX = Item(223, 0, "Yellow Shulker Box", "yellow_shulker_box");
    MU const Item LIME_SHULKER_BOX = Item(224, 0, "Lime Shulker Box", "lime_shulker_box");
    MU const Item PINK_SHULKER_BOX = Item(225, 0, "Pink Shulker Box", "pink_shulker_box");
    MU const Item GRAY_SHULKER_BOX = Item(226, 0, "Gray Shulker Box", "gray_shulker_box");
    MU const Item LIGHT_GRAY_SHULKER_BOX = Item(227, 0, "Light Gray Shulker Box", "silver_shulker_box");
    MU const Item CYAN_SHULKER_BOX = Item(228, 0, "Cyan Shulker Box", "cyan_shulker_box");
    MU const Item PURPLE_SHULKER_BOX = Item(229, 0, "Purple Shulker Box", "purple_shulker_box");
    MU const Item BLUE_SHULKER_BOX = Item(230, 0, "Blue Shulker Box", "blue_shulker_box");
    MU const Item BROWN_SHULKER_BOX = Item(231, 0, "Brown Shulker Box", "brown_shulker_box");
    MU const Item GREEN_SHULKER_BOX = Item(232, 0, "Green Shulker Box", "green_shulker_box");
    MU const Item RED_SHULKER_BOX = Item(233, 0, "Red Shulker Box", "red_shulker_box");
    MU const Item BLACK_SHULKER_BOX = Item(234, 0, "Black Shulker Box", "black_shulker_box");

    MU const Item WHITE_GLAZED_TERRACOTTA = Item(235, 0, "White Glazed Terracotta", "white_glazed_terracotta");
    MU const Item ORANGE_GLAZED_TERRACOTTA = Item(236, 0, "Orange Glazed Terracotta", "orange_glazed_terracotta");
    MU const Item MAGENTA_GLAZED_TERRACOTTA = Item(237, 0, "Magenta Glazed Terracotta", "magenta_glazed_terracotta");
    MU const Item LIGHT_BLUE_GLAZED_TERRACOTTA =
            Item(238, 0, "Light Blue Glazed Terracotta", "light_blue_glazed_terracotta");
    MU const Item YELLOW_GLAZED_TERRACOTTA = Item(239, 0, "Yellow Glazed Terracotta", "yellow_glazed_terracotta");
    MU const Item LIME_GLAZED_TERRACOTTA = Item(240, 0, "Lime Glazed Terracotta", "lime_glazed_terracotta");
    MU const Item PINK_GLAZED_TERRACOTTA = Item(241, 0, "Pink Glazed Terracotta", "pink_glazed_terracotta");
    MU const Item GRAY_GLAZED_TERRACOTTA = Item(242, 0, "Gray Glazed Terracotta", "gray_glazed_terracotta");
    MU const Item LIGHT_GRAY_GLAZED_TERRACOTTA =
            Item(243, 0, "Light Gray Glazed Terracotta", "light_gray_glazed_terracotta");
    MU const Item CYAN_GLAZED_TERRACOTTA = Item(244, 0, "Cyan Glazed Terracotta", "cyan_glazed_terracotta");
    MU const Item PURPLE_GLAZED_TERRACOTTA = Item(245, 0, "Purple Glazed Terracotta", "purple_glazed_terracotta");
    MU const Item BLUE_GLAZED_TERRACOTTA = Item(246, 0, "Blue Glazed Terracotta", "blue_glazed_terracotta");
    MU const Item BROWN_GLAZED_TERRACOTTA = Item(247, 0, "Brown Glazed Terracotta", "brown_glazed_terracotta");
    MU const Item GREEN_GLAZED_TERRACOTTA = Item(248, 0, "Green Glazed Terracotta", "green_glazed_terracotta");
    MU const Item RED_GLAZED_TERRACOTTA = Item(249, 0, "Red Glazed Terracotta", "red_glazed_terracotta");
    MU const Item BLACK_GLAZED_TERRACOTTA = Item(250, 0, "Black Glazed Terracotta", "black_glazed_terracotta");

    MU const Item WHITE_CONCRETE = Item(251, 0, "White Concrete", "concrete");
    MU const Item ORANGE_CONCRETE = Item(251, 1, "Orange Concrete", "concrete");
    MU const Item MAGENTA_CONCRETE = Item(251, 2, "Magenta Concrete", "concrete");
    MU const Item LIGHT_BLUE_CONCRETE = Item(251, 3, "Light Blue Concrete", "concrete");
    MU const Item YELLOW_CONCRETE = Item(251, 4, "Yellow Concrete", "concrete");
    MU const Item LIME_CONCRETE = Item(251, 5, "Lime Concrete", "concrete");
    MU const Item PINK_CONCRETE = Item(251, 6, "Pink Concrete", "concrete");
    MU const Item GRAY_CONCRETE = Item(251, 7, "Gray Concrete", "concrete");
    MU const Item LIGHT_GRAY_CONCRETE = Item(251, 8, "Light Gray Concrete", "concrete");
    MU const Item CYAN_CONCRETE = Item(251, 9, "Cyan Concrete", "concrete");
    MU const Item PURPLE_CONCRETE = Item(251, 10, "Purple Concrete", "concrete");
    MU const Item BLUE_CONCRETE = Item(251, 11, "Blue Concrete", "concrete");
    MU const Item BROWN_CONCRETE = Item(251, 12, "Brown Concrete", "concrete");
    MU const Item GREEN_CONCRETE = Item(251, 13, "Green Concrete", "concrete");
    MU const Item RED_CONCRETE = Item(251, 14, "Red Concrete", "concrete");
    MU const Item BLACK_CONCRETE = Item(251, 15, "Black Concrete", "concrete");

    MU const Item WHITE_CONCRETE_POWDER = Item(252, 0, "White Concrete Powder", "concrete_powder");
    MU const Item ORANGE_CONCRETE_POWDER = Item(252, 1, "Orange Concrete Powder", "concrete_powder");
    MU const Item MAGENTA_CONCRETE_POWDER = Item(252, 2, "Magenta Concrete Powder", "concrete_powder");
    MU const Item LIGHT_BLUE_CONCRETE_POWDER = Item(252, 3, "Light Blue Concrete Powder", "concrete_powder");
    MU const Item YELLOW_CONCRETE_POWDER = Item(252, 4, "Yellow Concrete Powder", "concrete_powder");
    MU const Item LIME_CONCRETE_POWDER = Item(252, 5, "Lime Concrete Powder", "concrete_powder");
    MU const Item PINK_CONCRETE_POWDER = Item(252, 6, "Pink Concrete Powder", "concrete_powder");
    MU const Item GRAY_CONCRETE_POWDER = Item(252, 7, "Gray Concrete Powder", "concrete_powder");
    MU const Item LIGHT_GRAY_CONCRETE_POWDER = Item(252, 8, "Light Gray Concrete Powder", "concrete_powder");
    MU const Item CYAN_CONCRETE_POWDER = Item(252, 9, "Cyan Concrete Powder", "concrete_powder");
    MU const Item PURPLE_CONCRETE_POWDER = Item(252, 10, "Purple Concrete Powder", "concrete_powder");
    MU const Item BLUE_CONCRETE_POWDER = Item(252, 11, "Blue Concrete Powder", "concrete_powder");
    MU const Item BROWN_CONCRETE_POWDER = Item(252, 12, "Brown Concrete Powder", "concrete_powder");
    MU const Item GREEN_CONCRETE_POWDER = Item(252, 13, "Green Concrete Powder", "concrete_powder");
    MU const Item RED_CONCRETE_POWDER = Item(252, 14, "Red Concrete Powder", "concrete_powder");
    MU const Item BLACK_CONCRETE_POWDER = Item(252, 15, "Black Concrete Powder", "concrete_powder");

    MU const Item STRUCTURE_BLOCK = Item(255, 0, "Structure Block", "structure_block");

    MU const Item IRON_SHOVEL = Item(256, 0, ItemTool, "Iron Shovel", "iron_shovel", true);
    MU const Item IRON_PICKAXE = Item(257, 0, ItemTool, "Iron Pickaxe", "iron_pickaxe", true);
    MU const Item IRON_AXE = Item(258, 0, ItemTool, "Iron Axe", "iron_axe", true);
    MU const Item FLINT_AND_STEEL = Item(259, 0, ItemTool, "Flint and Steel", "flint_and_steel", true);
    MU const Item APPLE = Item(260, 0, "Apple", "apple");
    MU const Item BOW = Item(261, 0, ItemBow, "Bow", "bow");
    MU const Item ARROW = Item(262, 0, "Arrow", "arrow");

    MU const Item COAL = Item(263, 0, "Coal", "coal");
    MU const Item CHARCOAL = Item(263, 1, "Charcoal", "coal");

    MU const Item DIAMOND = Item(264, 0, "Diamond", "diamond");
    MU const Item IRON_INGOT = Item(265, 0, "Iron Ingot", "iron_ingot");
    MU const Item GOLD_INGOT = Item(266, 0, "Gold Ingot", "gold_ingot");

    MU const Item IRON_SWORD = Item(267, 0, ItemSword, "Iron Sword", "iron_sword", true);

    MU const Sword WOODEN_SWORD = Sword(268, "Wooden Sword", "wooden_sword", ToolMaterials::TOOL_WOOD);
    MU const Tool WOODEN_SHOVEL = Tool(269, "Wooden Shovel", "wooden_shovel", ToolMaterials::TOOL_WOOD);
    MU const Tool WOODEN_PICKAXE = Tool(270, "Wooden Pickaxe", "wooden_pickaxe", ToolMaterials::TOOL_WOOD);
    MU const Tool WOODEN_AXE = Tool(271, "Wooden Axe", "wooden_axe", ToolMaterials::TOOL_WOOD);

    MU const Sword STONE_SWORD = Sword(272, "Stone Sword", "stone_sword", ToolMaterials::TOOL_STONE);
    MU const Tool STONE_SHOVEL = Tool(273, "Stone Shovel", "stone_shovel", ToolMaterials::TOOL_STONE);
    MU const Tool STONE_PICKAXE = Tool(274, "Stone Pickaxe", "stone_pickaxe", ToolMaterials::TOOL_STONE);
    MU const Tool STONE_AXE = Tool(275, "Stone Axe", "stone_axe", ToolMaterials::TOOL_STONE);

    MU const Sword DIAMOND_SWORD = Sword(276, "Diamond Sword", "diamond_sword", ToolMaterials::TOOL_DIAMOND);
    MU const Tool DIAMOND_SHOVEL = Tool(277, "Diamond Shovel", "diamond_shovel", ToolMaterials::TOOL_DIAMOND);
    MU const Tool DIAMOND_PICKAXE = Tool(278, "Diamond Pickaxe", "diamond_pickaxe", ToolMaterials::TOOL_DIAMOND);
    MU const Tool DIAMOND_AXE = Tool(279, "Diamond Axe", "diamond_axe", ToolMaterials::TOOL_DIAMOND);

    MU const Item STICK = Item(280, 0, "Stick", "stick");
    MU const Item BOWL = Item(281, 0, "Bowl", "bowl");
    MU const Item MUSHROOM_STEW = Item(282, 0, "Mushroom Stew", "mushroom_stew");

    MU const Sword GOLDEN_SWORD = Sword(283, "Golden Sword", "golden_sword", ToolMaterials::TOOL_GOLD);
    MU const Tool GOLDEN_SHOVEL = Tool(284, "Golden Shovel", "golden_shovel", ToolMaterials::TOOL_GOLD);
    MU const Tool GOLDEN_PICKAXE = Tool(285, "Golden Pickaxe", "golden_pickaxe", ToolMaterials::TOOL_GOLD);
    MU const Tool GOLDEN_AXE = Tool(286, "Golden Axe", "golden_axe", ToolMaterials::TOOL_GOLD);

    MU const Item STRING = Item(287, 0, "String", "string");
    MU const Item FEATHER = Item(288, 0, "Feather", "feather");
    MU const Item GUNPOWDER = Item(289, 0, "Gunpowder", "gunpowder");

    MU const Tool WOODEN_HOE = Tool(290, "Wooden Hoe", "wooden_hoe", ToolMaterials::TOOL_WOOD);
    MU const Tool STONE_HOE = Tool(291, "Stone Hoe", "stone_hoe", ToolMaterials::TOOL_STONE);
    MU const Tool IRON_HOE = Tool(292, "Iron Hoe", "iron_hoe", ToolMaterials::TOOL_IRON);
    MU const Tool DIAMOND_HOE = Tool(293, "Diamond Hoe", "diamond_hoe", ToolMaterials::TOOL_DIAMOND);
    MU const Tool GOLDEN_HOE = Tool(294, "Golden Hoe", "golden_hoe", ToolMaterials::TOOL_GOLD);

    MU const Item WHEAT_SEEDS = Item(295, 0, "Wheat Seeds", "wheat_seeds");
    MU const Item WHEAT = Item(296, 0, "Wheat", "wheat");
    MU const Item BREAD = Item(297, 0, "Bread", "bread");

    MU const Armor LEATHER_HELMET =
            Armor(298, "Leather Helmet", "leather_helmet", EntityEquipSlot::HEAD, ArmorMaterials::ARMOR_LEATHER);
    MU const Armor LEATHER_TUNIC =
            Armor(299, "Leather Tunic", "leather_chestplate", EntityEquipSlot::CHEST, ArmorMaterials::ARMOR_LEATHER);
    MU const Armor LEATHER_PANTS =
            Armor(300, "Leather Pants", "leather_leggings", EntityEquipSlot::LEGS, ArmorMaterials::ARMOR_LEATHER);
    MU const Armor LEATHER_BOOTS =
            Armor(301, "Leather Boots", "leather_boots", EntityEquipSlot::FEET, ArmorMaterials::ARMOR_LEATHER);

    MU const Armor CHAINMAIL_HELMET =
            Armor(302, "Chainmail Helmet", "chainmail_helmet", EntityEquipSlot::HEAD, ArmorMaterials::ARMOR_CHAIN);
    MU const Armor CHAINMAIL_CHESTPLATE = Armor(303, "Chainmail Chestplate", "chainmail_chestplate",
                                                EntityEquipSlot::CHEST, ArmorMaterials::ARMOR_CHAIN);
    MU const Armor CHAINMAIL_LEGGINGS =
            Armor(304, "Chainmail Leggings", "chainmail_leggings", EntityEquipSlot::LEGS, ArmorMaterials::ARMOR_CHAIN);
    MU const Armor CHAINMAIL_BOOTS =
            Armor(305, "Chainmail Boots", "chainmail_boots", EntityEquipSlot::FEET, ArmorMaterials::ARMOR_CHAIN);

    MU const Armor IRON_HELMET =
            Armor(306, "Iron Helmet", "iron_helmet", EntityEquipSlot::HEAD, ArmorMaterials::ARMOR_IRON);
    MU const Armor IRON_CHESTPLATE =
            Armor(307, "Iron Chestplate", "iron_chestplate", EntityEquipSlot::CHEST, ArmorMaterials::ARMOR_IRON);
    MU const Armor IRON_LEGGINGS =
            Armor(308, "Iron Leggings", "iron_leggings", EntityEquipSlot::LEGS, ArmorMaterials::ARMOR_IRON);
    MU const Armor IRON_BOOTS =
            Armor(309, "Iron Boots", "iron_boots", EntityEquipSlot::FEET, ArmorMaterials::ARMOR_IRON);

    MU const Armor DIAMOND_HELMET =
            Armor(310, "Diamond Helmet", "diamond_helmet", EntityEquipSlot::HEAD, ArmorMaterials::ARMOR_DIAMOND);
    MU const Armor DIAMOND_CHESTPLATE = Armor(311, "Diamond Chestplate", "diamond_chestplate", EntityEquipSlot::CHEST,
                                              ArmorMaterials::ARMOR_DIAMOND);
    MU const Armor DIAMOND_LEGGINGS =
            Armor(312, "Diamond Leggings", "diamond_leggings", EntityEquipSlot::LEGS, ArmorMaterials::ARMOR_DIAMOND);
    MU const Armor DIAMOND_BOOTS =
            Armor(313, "Diamond Boots", "diamond_boots", EntityEquipSlot::FEET, ArmorMaterials::ARMOR_DIAMOND);

    MU const Armor GOLDEN_HELMET =
            Armor(314, "Golden Helmet", "golden_helmet", EntityEquipSlot::HEAD, ArmorMaterials::ARMOR_GOLD);
    MU const Armor GOLDEN_CHESTPLATE =
            Armor(315, "Golden Chestplate", "golden_chestplate", EntityEquipSlot::CHEST, ArmorMaterials::ARMOR_GOLD);
    MU const Armor GOLDEN_LEGGINGS =
            Armor(316, "Golden Leggings", "golden_leggings", EntityEquipSlot::LEGS, ArmorMaterials::ARMOR_GOLD);
    MU const Armor GOLDEN_BOOTS =
            Armor(317, "Golden Boots", "golden_boots", EntityEquipSlot::FEET, ArmorMaterials::ARMOR_GOLD);

    MU const Item FLINT = Item(318, 0, "Flint", "flint");
    MU const Item RAW_PORKCHOP = Item(319, 0, "Raw Porkchop", "porkchop");
    MU const Item COOKED_PORKCHOP = Item(320, 0, "Cooked Porkchop", "cooked_porkchop");
    MU const Item PAINTING = Item(321, 0, "Painting", "painting");

    MU const Item GOLDEN_APPLE = Item(322, 0, "Golden Apple", "golden_apple");
    MU const Item ENCHANTED_GOLDEN_APPLE = Item(322, 1, "Enchanted Golden Apple", "golden_apple");

    MU const Item SIGN = Item(323, 0, "Sign", "sign");
    MU const Item OAK_DOOR = Item(324, 0, "Oak Door", "wooden_door");
    MU const Item BUCKET = Item(325, 0, "Bucket", "bucket");
    MU const Item WATER_BUCKET = Item(326, 0, "Water Bucket", "water_bucket");
    MU const Item LAVA_BUCKET = Item(327, 0, "Lava Bucket", "lava_bucket");
    MU const Item MINECART = Item(328, 0, "Minecart", "minecart");
    MU const Item SADDLE = Item(329, 0, "Saddle", "saddle");
    MU const Item IRON_DOOR = Item(330, 0, "Iron Door", "iron_door");
    MU const Item REDSTONE = Item(331, 0, "Redstone", "redstone");
    MU const Item SNOWBALL = Item(332, 0, "Snowball", "snowball");
    MU const Item OAK_BOAT = Item(333, 0, "Oak Boat", "boat");
    MU const Item LEATHER = Item(334, 0, "Leather", "leather");
    MU const Item MILK_BUCKET = Item(335, 0, "Milk Bucket", "milk_bucket");
    MU const Item BRICK = Item(336, 0, "Brick", "brick");
    MU const Item CLAY = Item(337, 0, "Clay", "clay_ball");
    MU const Item SUGAR_CANES_2 = Item(338, 0, "Sugar Canes", "reeds");
    MU const Item PAPER = Item(339, 0, "Paper", "paper");
    MU const Item BOOK = Item(340, 0, "Book", "book");
    MU const Item SLIMEBALL = Item(341, 0, "Slimeball", "slime_ball");
    MU const Item MINECART_WITH_CHEST = Item(342, 0, "Minecart with Chest", "chest_minecart");
    MU const Item MINECART_WITH_FURNACE = Item(343, 0, "Minecart with Furnace", "furnace_minecart");
    MU const Item EGG = Item(344, 0, "Egg", "egg");
    MU const Item COMPASS = Item(345, 0, "Compass", "compass");
    MU const Item FISHING_ROD = Item(346, 0, ItemFishingRod, "Fishing Rod", "fishing_rod", true);
    MU const Item CLOCK = Item(347, 0, "Clock", "clock");
    MU const Item GLOWSTONE_DUST = Item(348, 0, "Glowstone Dust", "glowstone_dust");

    MU const Item RAW_FISH = Item(349, 0, "Raw Fish", "fish");
    MU const Item RAW_SALMON = Item(349, 1, "Raw Fish", "fish");
    MU const Item RAW_CLOWN_FISH = Item(349, 2, "Raw Fish", "fish");
    MU const Item PUFFERFISH = Item(349, 3, "Pufferfish", "fish");

    MU const Item COOKED_FISH = Item(350, 0, "Cooked Fish", "cooked_fish");
    MU const Item COOKED_SALMON = Item(350, 1, "Cooked Salmon", "cooked_fish");

    MU const Item INK_SACK = Item(351, 0, "Ink Sack", "dye");
    MU const Item ROSE_RED = Item(351, 1, "Rose Red", "dye");
    MU const Item CACTUS_GREEN = Item(351, 2, "Cactus Green", "dye");
    MU const Item COCOA_BEANS = Item(351, 3, "Cocoa Beans", "dye");
    MU const Item LAPIS_LAZULI = Item(351, 4, "Lapis Lazuli", "dye");
    MU const Item PURPLE_DYE = Item(351, 5, "Purple Dye", "dye");
    MU const Item CYAN_DYE = Item(351, 6, "Cyan Dye", "dye");
    MU const Item LIGHT_GRAY_DYE = Item(351, 7, "Light Gray Dye", "dye");
    MU const Item GRAY_DYE = Item(351, 8, "Gray Dye", "dye");
    MU const Item PINK_DYE = Item(351, 9, "Pink Dye", "dye");
    MU const Item LIME_DYE = Item(351, 10, "Lime Dye", "dye");
    MU const Item DANDELION_YELLOW = Item(351, 11, "Dandelion Yellow", "dye");
    MU const Item LIGHT_BLUE_DYE = Item(351, 12, "Light Blue Dye", "dye");
    MU const Item MAGENTA_DYE = Item(351, 13, "Magenta Dye", "dye");
    MU const Item ORANGE_DYE = Item(351, 14, "Orange Dye", "dye");
    MU const Item BONE_MEAL = Item(351, 15, "Bone Meal", "dye");

    MU const Item BONE = Item(352, 0, "Bone", "bone");
    MU const Item SUGAR = Item(353, 0, "Sugar", "sugar");
    MU const Item CAKE = Item(354, 0, "Cake", "cake");

    MU const Item WHITE_BED = Item(355, 0, "White Bed", "bed");
    MU const Item ORANGE_BED = Item(355, 1, "Orange Bed", "bed");
    MU const Item MAGENTA_BED = Item(355, 2, "Magenta Bed", "bed");
    MU const Item LIGHT_BLUE_BED = Item(355, 3, "Light Blue Bed", "bed");
    MU const Item YELLOW_BED = Item(355, 4, "Yellow Bed", "bed");
    MU const Item LIME_BED = Item(355, 5, "Lime Bed", "bed");
    MU const Item PINK_BED = Item(355, 6, "Pink Bed", "bed");
    MU const Item GRAY_BED = Item(355, 7, "Gray Bed", "bed");
    MU const Item LIGHT_GRAY_BED = Item(355, 8, "Light Gray Bed", "bed");
    MU const Item CYAN_BED = Item(355, 9, "Cyan Bed", "bed");
    MU const Item PURPLE_BED = Item(355, 10, "Purple Bed", "bed");
    MU const Item BLUE_BED = Item(355, 11, "Blue Bed", "bed");
    MU const Item BROWN_BED = Item(355, 12, "Brown Bed", "bed");
    MU const Item GREEN_BED = Item(355, 13, "Green Bed", "bed");
    MU const Item RED_BED = Item(355, 14, "Red Bed", "bed");
    MU const Item BLACK_BED = Item(355, 15, "Black Bed", "bed");

    MU const Item REDSTONE_REPEATER = Item(356, 0, "Redstone Repeater", "repeater");
    MU const Item COOKIE = Item(357, 0, "Cookie", "cookie");

    MU const Item MAP = Item(358, 0, "Map", "filled_map");
    MU const Item LOCATOR_MAP = Item(358, 2, "Locator Map", "filled_map");
    MU const Item OCEAN_EXPLORER_MAP = Item(358, 3, "Ocean Explorer Map", "filled_map");
    MU const Item WOODLAND_EXPLORER_MAP = Item(358, 4, "Woodland Explorer Map", "filled_map");
    MU const Item TREASURE_MAP = Item(358, 5, "Treasure Map", "filled_map");
    MU const Item LOCKED_MAP = Item(358, 6, "Locked Map", "filled_map");


    MU const Item SHEARS = Item(359, 0, ItemTool, "Shears", "shears", true);
    MU const Item MELON = Item(360, 0, "Melon", "melon");
    MU const Item PUMPKIN_SEEDS = Item(361, 0, "Pumpkin Seeds", "pumpkin_seeds");
    MU const Item MELON_SEEDS = Item(362, 0, "Melon Seeds", "melon_seeds");
    MU const Item RAW_BEEF = Item(363, 0, "Raw Beef", "beef");
    MU const Item STEAK = Item(364, 0, "Steak", "cooked_beef");
    MU const Item RAW_CHICKEN = Item(365, 0, "Raw Chicken", "chicken");
    MU const Item COOKED_CHICKEN = Item(366, 0, "Cooked Chicken", "cooked_chicken");
    MU const Item ROTTEN_FLESH = Item(367, 0, "Rotten Flesh", "rotten_flesh");
    MU const Item ENDER_PEARL = Item(368, 0, "Ender Pearl", "ender_pearl");
    MU const Item BLAZE_ROD = Item(369, 0, "Blaze Rod", "blaze_rod");
    MU const Item GHAST_TEAR = Item(370, 0, "Ghast Tear", "ghast_tear");
    MU const Item GOLD_NUGGET = Item(371, 0, "Gold Nugget", "gold_nugget");

    // Item NETHER_WART = Item(372, "Nether Wart", "nether_wart");

    MU const Item POTION = Item(373, 0, "Potion", "potion");
    MU const Item POTION_AWKWARD = Item(373, 0, "Awkward", "potion");
    MU const Item MUNDANE_POTION = Item(373, 0, "Mundane Potion", "potion");
    MU const Item POTION_OF_EXTENDED_INVISIBILITY = Item(373, 0, "Potion of Extended Invisibility", "potion");
    MU const Item POTION_OF_EXTENDED_LEAPING = Item(373, 0, "Potion of Extended Leaping", "potion");
    MU const Item POTION_OF_EXTENDED_NIGHT_VISION = Item(373, 0, "Potion of Extended Night Vision", "potion");
    MU const Item POTION_OF_EXTENDED_POISON = Item(373, 0, "Potion of Extended Poison", "potion");
    MU const Item POTION_OF_EXTENDED_REGENERATION = Item(373, 0, "Potion of Extended Regeneration", "potion");
    MU const Item POTION_OF_EXTENDED_SLOWNESS = Item(373, 0, "Potion of Extended Slowness", "potion");
    MU const Item POTION_OF_EXTENDED_STRENGTH = Item(373, 0, "Potion of Extended Strength", "potion");
    MU const Item POTION_OF_EXTENDED_SWIFTNESS = Item(373, 0, "Potion of Extended Swiftness", "potion");
    MU const Item POTION_OF_EXTENDED_WATER_BREATHING = Item(373, 0, "Potion of Extended Water Breathing", "potion");
    MU const Item POTION_OF_EXTENDED_WEAKNESS = Item(373, 0, "Potion of Extended Weakness", "potion");
    MU const Item POTION_OF_FIRE_RESISTANCE = Item(373, 0, "Potion of Fire Resistance", "potion");
    MU const Item POTION_OF_HARMING = Item(373, 0, "Potion of Harming", "potion");
    MU const Item POTION_OF_HARMING_II = Item(373, 0, "Potion of Harming II", "potion");
    MU const Item POTION_OF_HEALING = Item(373, 0, "Potion of Healing", "potion");
    MU const Item POTION_OF_HEALING_II = Item(373, 0, "Potion of Healing II", "potion");
    MU const Item POTION_OF_INVISIBILITY = Item(373, 0, "Potion of Invisibility", "potion");
    MU const Item POTION_OF_LEAPING = Item(373, 0, "Potion of Leaping", "potion");
    MU const Item POTION_OF_LEAPING_II = Item(373, 0, "Potion of Leaping II", "potion");
    MU const Item POTION_OF_LUCK = Item(373, 0, "Potion of Luck", "potion");
    MU const Item POTION_OF_NIGHT_VISION = Item(373, 0, "Potion of Night Vision", "potion");
    MU const Item POTION_OF_POISON = Item(373, 0, "Potion of Poison", "potion");
    MU const Item POTION_OF_POISON_II = Item(373, 0, "Potion of Poison II", "potion");
    MU const Item POTION_OF_REGENERATION = Item(373, 0, "Potion of Regeneration", "potion");
    MU const Item POTION_OF_REGENERATION_II = Item(373, 0, "Potion of Regeneration II", "potion");
    MU const Item POTION_OF_SLOWNESS = Item(373, 0, "Potion of Slowness", "potion");
    MU const Item POTION_OF_STRENGTH = Item(373, 0, "Potion of Strength", "potion");
    MU const Item POTION_OF_STRENGTH_II = Item(373, 0, "Potion of Strength II", "potion");
    MU const Item POTION_OF_SWIFTNESS = Item(373, 0, "Potion of Swiftness", "potion");
    MU const Item POTION_OF_SWIFTNESS_II = Item(373, 0, "Potion of Swiftness II", "potion");
    MU const Item POTION_OF_WATER_BREATHING = Item(373, 0, "Potion of Water Breathing", "potion");
    MU const Item POTION_OF_WEAKNESS = Item(373, 0, "Potion of Weakness", "potion");
    MU const Item THICK_POTION = Item(373, 0, "Thick Potion", "potion");
    MU const Item UNCRAFTABLE_POTION = Item(373, 0, "Uncraftable Potion", "potion");
    MU const Item WATER_BOTTLE = Item(373, 0, "Water Bottle", "potion");

    MU const Item GLASS_BOTTLE = Item(374, 0, "Glass Bottle", "glass_bottle");
    MU const Item SPIDER_EYE = Item(375, 0, "Spider Eye", "spider_eye");
    MU const Item FERMENTED_SPIDER_EYE = Item(376, 0, "Fermented Spider Eye", "fermented_spider_eye");
    MU const Item BLAZE_POWDER = Item(377, 0, "Blaze Powder", "blaze_powder");
    MU const Item MAGMA_CREAM = Item(378, 0, "Magma Cream", "magma_cream");
    // Item BREWING_STAND = Item(379, 0, "Brewing Stand", "brewing_stand");
    // Item CAULDRON = Item(380, 0, "Cauldron", "cauldron");
    MU const Item EYE_OF_ENDER = Item(381, 0, "Eye of Ender", "ender_eye");
    MU const Item GLISTERING_MELON = Item(382, 0, "Glistering Melon", "speckled_melon");

    MU const Item SPAWN_EGG = Item(383, 0, "Spawn Egg", "");
    // times 1000 million

    MU const Item BOTTLE_O_ENCHANTING = Item(384, 0, "Bottle o' Enchanting", "experience_bottle");
    MU const Item FIRE_CHARGE = Item(385, 0, "Fire Charge", "fire_charge");
    MU const Item BOOK_AND_QUILL = Item(386, 0, "Book and Quill", "writable_book");
    MU const Item WRITTEN_BOOK = Item(387, 0, "Written Book", "written_book");
    MU const Item EMERALD = Item(388, 0, "Emerald", "emerald");
    MU const Item ITEM_FRAME = Item(389, 0, "Item Frame", "item_frame");
    // Item FLOWER_POT = Item(390, 0, "Flower Pot", "flower_pot");
    MU const Item CARROT = Item(391, 0, "Carrot", "carrot");
    MU const Item POTATO = Item(392, 0, "Potato", "potato");
    MU const Item BAKED_POTATO = Item(393, 0, "Baked Potato", "baked_potato");
    MU const Item POISONOUS_POTATO = Item(394, 0, "Poisonous Potato", "poisonous_potato");
    MU const Item EMPTY_MAP = Item(395, 0, "Empty Map", "map");
    MU const Item GOLDEN_CARROT = Item(396, 0, "Golden Carrot", "golden_carrot");

    MU const Skull SKELETON_SKULL = Skull(397, 0, "Skeleton Skull", "skull");
    MU const Skull WITHER_SKELETON_SKULL = Skull(397, 1, "Wither Skeleton Skull", "skull");
    MU const Skull ZOMBIE_HEAD = Skull(397, 2, "Zombie Head", "skull");
    MU const Skull PLAYER_HEAD = Skull(397, 3, "Player Head", "skull");
    MU const Skull CREEPER_HEAD = Skull(397, 4, "Creeper Head", "skull");
    MU const Skull DRAGON_HEAD = Skull(397, 5, "Dragon Head", "skull");

    MU const Item CARROT_ON_A_STICK = Item(398, 0, "Carrot on a Stick", "carrot_on_a_stick");
    MU const Item NETHER_STAR = Item(399, 0, "Nether Star", "nether_star");
    MU const Item PUMPKIN_PIE = Item(400, 0, "Pumpkin Pie", "pumpkin_pie");
    MU const Item FIREWORK_ROCKET = Item(401, 0, "Firework Rocket", "fireworks");
    MU const Item FIREWORK_STAR = Item(402, 0, "Firework Star", "firework_charge");
    MU const Item ENCHANTED_BOOK = Item(403, 0, "Enchanted Book", "enchanted_book");
    MU const Item REDSTONE_COMPARATOR = Item(404, 0, "Redstone Comparator", "comparator");
    MU const Item NETHER_BRICK = Item(405, 0, "Nether Brick", "netherbrick");
    MU const Item NETHER_QUARTZ = Item(406, 0, "Nether Quartz", "quartz");
    MU const Item MINECART_WITH_TNT = Item(407, 0, "Minecart with TNT", "tnt_minecart");
    MU const Item MINECART_WITH_HOPPER = Item(408, 0, "Minecart with Hopper", "hopper_minecart");
    MU const Item PRISMARINE_SHARD = Item(409, 0, "Prismarine Shard", "prismarine_shard");
    MU const Item PRISMARINE_CRYSTALS = Item(410, 0, "Prismarine Crystals", "prismarine_crystals");
    MU const Item RAW_RABBIT = Item(411, 0, "Raw Rabbit", "rabbit");
    MU const Item COOKED_RABBIT = Item(412, 0, "Cooked Rabbit", "cooked_rabbit");
    MU const Item RABBIT_STEW = Item(413, 0, "Rabbit Stew", "rabbit_stew");
    MU const Item RABBITS_FOOT = Item(414, 0, "Rabbit’s Foot", "rabbit_foot");
    MU const Item RABBIT_HIDE = Item(415, 0, "Rabbit Hide", "rabbit_hide");
    MU const Item ARMOR_STAND = Item(416, 0, "Armor Stand", "armor_stand");
    MU const Item IRON_HORSE_ARMOR = Item(417, 0, "Iron Horse Armor", "iron_horse_armor");
    MU const Item GOLDEN_HORSE_ARMOR = Item(418, 0, "Golden Horse Armor", "golden_horse_armor");
    MU const Item DIAMOND_HORSE_ARMOR = Item(419, 0, "Diamond Horse Armor", "diamond_horse_armor");
    MU const Item LEAD = Item(420, 0, "Lead", "lead");
    MU const Item NAME_TAG = Item(421, 0, "Name Tag", "name_tag");
    MU const Item MINECART_WITH_COMMAND_BLOCK = Item(422, 0, "Minecart with Command Block", "command_block_minecart");
    MU const Item RAW_MUTTON = Item(423, 0, "Raw Mutton", "mutton");
    MU const Item COOKED_MUTTON = Item(424, 0, "Cooked Mutton", "cooked_mutton");

    MU const Item BLACK_BANNER = Item(425, 0, "Black Banner", "banner");
    MU const Item RED_BANNER = Item(425, 1, "Red Banner", "banner");
    MU const Item GREEN_BANNER = Item(425, 2, "Green Banner", "banner");
    MU const Item BROWN_BANNER = Item(425, 3, "Brown Banner", "banner");
    MU const Item BLUE_BANNER = Item(425, 4, "Blue Banner", "banner");
    MU const Item PURPLE_BANNER = Item(425, 5, "Purple Banner", "banner");
    MU const Item CYAN_BANNER = Item(425, 6, "Cyan Banner", "banner");
    MU const Item LIGHT_BANNER = Item(425, 7, "Light Gray Banner", "banner");
    MU const Item GRAY_BANNER = Item(425, 8, "Gray Banner", "banner");
    MU const Item PINK_BANNER = Item(425, 9, "Pink Banner", "banner");
    MU const Item LIME_BANNER = Item(425, 10, "Lime Banner", "banner");
    MU const Item YELLOW_BANNER = Item(425, 11, "Yellow Banner", "banner");
    MU const Item LIGHT_BLUE_BANNER = Item(425, 12, "Light Blue Banner", "banner");
    MU const Item MAGENTA_BANNER = Item(425, 13, "Magenta Banner", "banner");
    MU const Item ORANGE_BANNER = Item(425, 14, "Orange Banner", "banner");
    MU const Item WHITE_BANNER = Item(425, 15, "White Banner", "banner");

    MU const Item END_CRYSTAL = Item(426, 0, "End Crystal", "end_crystal");
    MU const Item SPRUCE_DOOR = Item(427, 0, "Spruce Door", "spruce_door");
    MU const Item BIRCH_DOOR = Item(428, 0, "Birch Door", "birch_door");
    MU const Item JUNGLE_DOOR = Item(429, 0, "Jungle Door", "jungle_door");
    MU const Item ACACIA_DOOR = Item(430, 0, "Acacia Door", "acacia_door");
    MU const Item DARK_OAK_DOOR = Item(431, 0, "Dark Oak Door", "dark_oak_door");
    MU const Item CHORUS_FRUIT = Item(432, 0, "Chorus Fruit", "chorus_fruit");
    MU const Item POPPED_CHORUS_FRUIT = Item(433, 0, "Popped Chorus Fruit", "popped_chorus_fruit");
    MU const Item BEETROOT = Item(434, 0, "Beetroot", "beetroot");
    MU const Item BEETROOT_SEEDS = Item(435, 0, "Beetroot Seeds", "beetroot_seeds");
    MU const Item BEETROOT_SOUP = Item(436, 0, "Beetroot Soup", "beetroot_soup");
    MU const Item DRAGONS_BREATH = Item(437, 0, "Dragon’s Breath", "dragon_breath");
    MU const Item SPLASH_POTION = Item(438, 0, "Splash Potion", "splash_potion");
    MU const Item SPECTRAL_ARROW = Item(439, 0, "Spectral Arrow", "spectral_arrow");
    MU const Item TIPPED_ARROW = Item(440, 0, "Tipped Arrow", "tipped_arrow");
    MU const Item LINGERING_POTION = Item(441, 0, "Lingering Potion", "lingering_potion");
    MU const Item SHIELD = Item(442, 0, "Shield", "shield");
    MU const Item ELYTRA = Item(443, 0, ItemElytra, "Elytra", "elytra");
    MU const Item SPRUCE_BOAT = Item(444, 0, "Spruce Boat", "spruce_boat");
    MU const Item BIRCH_BOAT = Item(445, 0, "Birch Boat", "birch_boat");
    MU const Item JUNGLE_BOAT = Item(446, 0, "Jungle Boat", "jungle_boat");
    MU const Item ACACIA_BOAT = Item(447, 0, "Acacia Boat", "acacia_boat");
    MU const Item DARK_OAK_BOAT = Item(448, 0, "Dark Oak Boat", "dark_oak_boat");
    MU const Item TOTEM_OF_UNDYING = Item(449, 0, "Totem of Undying", "totem_of_undying");
    MU const Item SHULKER_SHELL = Item(450, 0, "Shulker Shell", "shulker_shell");
    MU const Item IRON_NUGGET = Item(452, 0, "Iron Nugget", "iron_nugget");
    MU const Item KNOWLEDGE_BOOK = Item(453, 0, "Knowledge Book", "knowledge_book");

    MU const Trident TRIDENT = Trident(546, 0, "Trident", "trident");

    MU const Item HEART_OF_THE_SEA = Item(571, 0, "Heart of the Sea", "heart_of_the_sea");
    MU const Item NAUTILUS_CORE = HEART_OF_THE_SEA;


    MU const Item DISC_13 = Item(2256, 0, "13 Disc", "record_13");
    MU const Item DISC_CAT = Item(2257, 0, "Cat Disc", "record_cat");
    MU const Item DISC_BLOCKS = Item(2258, 0, "Blocks Disc", "record_blocks");
    MU const Item DISC_CHIRP = Item(2259, 0, "Chirp Disc", "record_chirp");
    MU const Item DISC_FAR = Item(2260, 0, "Far Disc", "record_far");
    MU const Item DISC_MALL = Item(2261, 0, "Mall Disc", "record_mall");
    MU const Item DISC_MELLOHI = Item(2262, 0, "Mellohi Disc", "record_mellohi");
    MU const Item DISC_STAL = Item(2263, 0, "Stal Disc", "record_stal");
    MU const Item DISC_STRAD = Item(2264, 0, "Strad Disc", "record_strad");
    MU const Item WARD_DISC = Item(2265, 0, "Ward Disc", "record_ward");
    MU const Item DISC_11 = Item(2266, 0, "11 Disc", "record_11");
    MU const Item DISC_WAIT = Item(2267, 0, "Wait Disc", "record_wait");

} // namespace Items
