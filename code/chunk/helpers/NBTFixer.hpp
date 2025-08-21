#pragma once

#include <string>
#include <unordered_set>

#include "code/Chunk/chunkData.hpp"
#include "common/nbt.hpp"
#include "jimmiebergmann/Yaml.hpp"
#include "lce/registry/itemRegistry.hpp"


namespace editor {

    class NBTFixer {
        static NBTFixer   s_instance;
        static bool s_configLoaded;
        static std::unordered_set<std::string> s_crashItems;

        NBTFixer();

    public:

        static constexpr u32 MAX_DEPTH = 5;

        MU static bool isValidItem(const std::string& id) {
            if EXPECT_FALSE(!id.starts_with("minecraft")) return false;
            const lce::Item* item = lce::registry::ItemRegistry::getItemFromIdentifier(id);
            return (item != nullptr);
        }

        MU static bool isCrashItem(const std::string & id) {
            return s_crashItems.count(id) > 0;
        }


        MU static bool isShulkerBox(const std::string& key) {
            static std::unordered_set<std::string> containers = {
                    "minecraft:shulker_box",
                    "minecraft:undyed_shulker_box",
                    "minecraft:white_shulker_box",
                    "minecraft:orange_shulker_box",
                    "minecraft:magenta_shulker_box",
                    "minecraft:light_blue_shulker_box",
                    "minecraft:yellow_shulker_box",
                    "minecraft:lime_shulker_box",
                    "minecraft:pink_shulker_box",
                    "minecraft:gray_shulker_box",
                    "minecraft:light_gray_shulker_box",
                    "minecraft:cyan_shulker_box",
                    "minecraft:purple_shulker_box",
                    "minecraft:blue_shulker_box",
                    "minecraft:brown_shulker_box",
                    "minecraft:green_shulker_box",
                    "minecraft:red_shulker_box",
                    "minecraft:black_shulker_box"
            };
            return containers.contains(key);
        }


        MU static bool isContainer(const std::string& key) {
            static std::unordered_set<std::string> containers = {
                    "minecraft:chest",
                    "minecraft:brewing_stand",
                    "minecraft:dispenser",
                    "minecraft:dropper",
                    "minecraft:furnace",
                    "minecraft:hopper",
                    "minecraft:jukebox",
                    "minecraft:blastfurnace",
                    "minecraft:blastfurnace",
            };
            return containers.contains(key) || isShulkerBox(key);
        }



        MU static void ensureValidItem(NBTBase& item, int depth = 0) {

            if (item("id") && item("id").is<std::string>()) {
                const auto& itemId = item["id"].get<std::string>();

                // ensure items inside embedded shulker box are correct
                if (depth < MAX_DEPTH) {
                    if (isShulkerBox(itemId)) {
                        if (item("tag")("BlockEntityTag")("Items")) {
                            fixTileEntity(item["tag"]["BlockEntityTag"], true, depth + 1);
                        }
                    }
                }

                // if (!NBTFixer::isValidItem(itemId)) {
                //     std::cout << "Found invalid item: " << itemId << "\n";
                //     item["id"] = makeString("minecraft:air");
                //     if (item("tag")) {
                //         item.removeTag("tag");
                //     }
                // }

                // if (NBTFixer::isCrashItem(itemId)) {
                //     std::cout << "Found crash item: " << itemId << "\n";
                //     item["id"] = makeString("minecraft:air");
                //     if (item("tag")) {
                //         item.removeTag("tag");
                //     }
                // }
            }
        }


        MU static void fixTileEntity(NBTBase& tileEntity, bool skipIdCheck = false, int depth = 0) {

            const auto& idTag = tileEntity("id");
            if (skipIdCheck || idTag && isContainer(idTag.get<std::string>())) {
                if (!tileEntity("Items"))
                    return;

                auto& items = tileEntity.ensureList("Items", eNBT::COMPOUND);

                if (depth < MAX_DEPTH) {
                    for (auto& item : items) {
                        ensureValidItem(item, depth + 1);
                    }
                }
            }
        }


        MU static void fixTileEntities(ChunkData* chunkData) {
            for (auto& te: chunkData->tileEntities) {
                fixTileEntity(te);
            }
        }




        MU static void fixEntity(NBTBase& entity) {
            if (entity("id") == makeString("minecraft:item_frame") &&
                entity("Item").is<NBTCompound>()) {

                NBTBase& item = entity["Item"];
                ensureValidItem(item);
            }
        }


        MU static void fixEntities(ChunkData* chunkData) {
            for (auto& entity : chunkData->entities) {
                fixEntity(entity);
            }
        }



    };
}
