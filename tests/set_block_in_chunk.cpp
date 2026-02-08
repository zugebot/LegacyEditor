#include <iostream>

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"
#include "include/lce/blocks/blockID.hpp"

#include "code/SaveFile/World.hpp"
#include "code/include.hpp"
#include "common/fmt.hpp"

static inline void setBlockIfInBounds(editor::World& world, int x, int y, int z, u16 rawBlock) {
    // If you have world bounds checks, put them here. Otherwise just set.
    world.setBlock(x, y, z, rawBlock);
}

void createSphere(editor::World& world, int cx, int cy, int cz, int r) {
    int r2 = r * r;

    // Thickness 1-ish: keep blocks whose distance^2 is within [r^2 - t, r^2 + t]
    // Using a slightly larger band reduces holes caused by integer grid sampling.
    int band = 2 * r; // about 1 block thick

    u16 block = static_cast<u16>(lce::blocks::DIAMOND_BLOCK_ID << 4);

    for (int y = cy - r; y <= cy + r; ++y) {
        for (int z = cz - r; z <= cz + r; ++z) {
            for (int x = cx - r; x <= cx + r; ++x) {
                const int dx = x - cx;
                const int dy = y - cy;
                const int dz = z - cz;

                const int d2 = dx * dx + dy * dy + dz * dz;

                if (d2 >= (r2 - band) && d2 <= (r2 + band)) {
                    if (x < 0 && z < 0) block = lce::blocks::REDSTONE_BLOCK_ID << 4;
                    else if (x < 0 && z >= 0) block = lce::blocks::DIAMOND_BLOCK_ID << 4;
                    else if (x >= 0 && z < 0) block = lce::blocks::GOLD_BLOCK_ID << 4;
                    else if (x >= 0 && z >= 0) block = lce::blocks::EMERALD_BLOCK_ID << 4;

                    if (x >= 32 && x < 40+16 && z >= 16 && z < 24+16) block = lce::blocks::IRON_ORE_ID << 4;

                    setBlockIfInBounds(world, x, y, z, block);
                }
            }
        }
    }
}







int main() {
    const std::string wiiu = R"(E:\Emulators\cemu_1.27.1\mlc01\usr\save\00050000\101d9d00\user\80000001\)";

    const std::string TEST_IN  = wiiu + "260104235224";

    constexpr auto consoleOut = lce::CONSOLE::WIIU;
    const std::string TEST_OUT = wiiu;
    editor::WriteSettings settings(editor::sch::AquaticTU69, consoleOut, TEST_OUT);

    editor::World world;

    if (world.read(TEST_IN) != 0) {
        cmn::log(cmn::eLog::error, "Failed to load file: {}\n", TEST_IN);
        return -1;
    }

    world.setWorldName(L"Test World Edit");

    world.setBlock( 0, 8,  0, lce::blocks::GOLD_BLOCK_ID << 4);
    world.setBlock( 16, 8,  0, lce::blocks::IRON_BLOCK_ID << 4);
    world.setBlock( 20, 8,  0, lce::blocks::BLOCK_OF_COAL_ID << 4);
    world.setBlock( 24, 8,  0, lce::blocks::REDSTONE_BLOCK_ID << 4);

    world.setBlock(-1, 8,  0, lce::blocks::EMERALD_BLOCK_ID << 4);
    world.setBlock( 0, 8, -1, lce::blocks::DIAMOND_BLOCK_ID << 4);
    world.setBlock(-1, 8, -1, lce::blocks::LAPIS_LAZULI_BLOCK_ID << 4);

    createSphere(world, 0, 128, 0, 100);

    const int statusOut = world.write(settings);
    if (statusOut != 0) {
        printf("converting to %s failed...\n", consoleToCStr(consoleOut));
        return statusOut;
    }

    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());
    return 0;
}