#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <bitset>
#include <map>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"
#include "include/lce/blocks/blockID.hpp"
#include "code/SaveFile/World.hpp"
#include "code/include.hpp"
#include "common/fmt.hpp"

using namespace lce::blocks;

// --- ADVANCED ARG SYSTEM ---
namespace ARG_Engine {
    const int CHUNK_LIMIT = 27; // 27 chunks out from 0,0
    const int TOTAL_BLOCKS = 432;

    // The "Key" to the ARG - encoded into the world structure
    const std::vector<std::string> LOG_ENTRIES = {
            "SYSTEM_AUTH_REQUIRED",
            "BUFFER_OVERFLOW_AT_Y256",
            "WHO_IS_THE_OBSERVER"
    };
}

// --- MATHEMATICAL ARCHITECT ---

class Architect {
public:
    // Generates a "Menger Sponge" variant (Fractal)
    static void buildMenger(editor::World& world, int x, int y, int z, int size) {
        if (size < 1) return;
        int newSize = size / 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    int sum = (i == 1) + (j == 1) + (k == 1);
                    if (sum > 1) continue; // The "Hollow" rule of the sponge
                    if (size <= 3) {
                        world.setBlock(x + i, y + j, z + k, (BlockID::QUARTZ_BLOCK_ID << 4));
                    } else {
                        buildMenger(world, x + i * newSize, y + j * newSize, z + k * newSize, newSize);
                    }
                }
            }
        }
    }

    // Creates "Glitch Ruins" - structures that look like corrupted houses
    static void spawnGlitchRuin(editor::World& world, int cx, int cz, int groundY) {
        u16 frame = (BlockID::OBSIDIAN_ID << 4);
        u16 logic = (BlockID::SEA_LANTERN_ID << 4);

        for (int y = 0; y < 15; y++) {
            for (int x = -5; x <= 5; x++) {
                for (int z = -5; z <= 5; z++) {
                    // Use a bit-shift "corruption" to decide if a block exists
                    // This creates the "half-rendered" look
                    if (((x ^ y ^ z) * 13) % 7 > 2) {
                        world.setBlock(cx + x, groundY + y, cz + z, frame);
                        if (y == 7) world.setBlock(cx + x, groundY + y, cz + z, logic);
                    }
                }
            }
        }
    }

    // Encodes strings into 8x8 floor tiles (Data-Tiles)
    static void writeDataTile(editor::World& world, int x, int z, char data) {
        std::bitset<8> bits(data);
        for (int i = 0; i < 8; i++) {
            u16 block = bits[i] ? (BlockID::WOOL_ID << 4) | 0 : (BlockID::WOOL_ID << 4) | 15;
            world.setBlock(x + (i % 3), 64, z + (i / 3), block);
        }
    }
};

// --- THE MAIN GENERATOR ---

void generateComplexARGWorld(editor::World& world) {
    printf("--- INITIALIZING SECTOR 0xFF HEAVY INJECTION ---\n");

    // 1. THE VOID FLOOR (Checkerboard of Madness)
    for (int x = -ARG_Engine::TOTAL_BLOCKS; x <= ARG_Engine::TOTAL_BLOCKS; x += 16) {
        printf("Generating Latent Logic: %d/432\n", x);
        for (int z = -ARG_Engine::TOTAL_BLOCKS; z <= ARG_Engine::TOTAL_BLOCKS; z += 16) {

            // Generate Procedural "Nodes"
            int groundY = 64;

            // Draw a high-contrast grid
            for (int dx = 0; dx < 16; dx++) {
                for (int dz = 0; dz < 16; dz++) {
                    bool pattern = ( (x+dx) / 8 + (z+dz) / 8 ) % 2 == 0;
                    u16 b = pattern ? (BlockID::STAINED_HARDENED_CLAY_ID << 4) | 15 : (BlockID::OBSIDIAN_ID << 4);
                    world.setBlock(x + dx, groundY, z + dz, b);

                    // The "Underworld" - Bedrock layer with holes to the void
                    if ((x+dx) % 17 == 0 && (z+dz) % 17 == 0) {
                        world.setBlock(x + dx, 0, z + dz, (BlockID::AIR_ID << 4));
                    } else {
                        world.setBlock(x + dx, 0, z + dz, (BlockID::BEDROCK_ID << 4));
                    }
                }
            }
        }
    }

    // 2. THE PRIME MONOLITHS
    // Spawns towers at coordinates that are prime numbers
    auto isPrime = [](int n) {
        n = abs(n);
        if (n < 2) return false;
        for (int i = 2; i * i <= n; i++) if (n % i == 0) return false;
        return true;
    };

    for (int i = 20; i < 200; i++) {
        if (isPrime(i)) {
            // High altitude data-spikes
            for (int y = 100; y < 180; y++) {
                world.setBlock(i, y, i, (BlockID::SEA_LANTERN_ID << 4));
                world.setBlock(-i, y, i, (BlockID::END_ROD_ID << 4));
            }
        }
    }

    // 3. THE RECURSIVE CORE (Center of the ARG)
    printf("Compiling Menger Fractal at Core...\n");
    Architect::buildMenger(world, -13, 150, -13, 27); // A floating 27x27x27 fractal cube

    // 4. THE DATA TRENCH (A massive hole in the world in the shape of a string)
    printf("Carving Data Trench...\n");
    std::string secret = "STOP_LOOKING";
    for (int i = 0; i < secret.length(); i++) {
        int tx = -150 + (i * 12);
        for (int y = 10; y < 64; y++) {
            for (int tz = -5; tz <= 5; tz++) {
                world.setBlock(tx, y, tz, (BlockID::AIR_ID << 4));
                // Line the trench with "Binary code"
                if (y == 11) {
                    Architect::writeDataTile(world, tx, tz, secret[i]);
                }
            }
        }
    }

    // 5. THE BARRIER LABYRINTH
    // Surrounds the spawn point with invisible logic
    for (int r = 5; r < 50; r += 5) {
        for (double a = 0; a < 6.28; a += 0.2) {
            int bx = (int)(cos(a) * r);
            int bz = (int)(sin(a) * r);
            world.setBlock(bx, 65, bz, (BlockID::BARRIER_ID << 4));
        }
    }

    // 6. GLITCH VILLAGES (The "Deleted" Population)
    Architect::spawnGlitchRuin(world, 100, 100, 64);
    Architect::spawnGlitchRuin(world, -100, -100, 64);
    Architect::spawnGlitchRuin(world, 100, -100, 64);
}

// --- STANDARD LCE MAIN ---
int main() {
    // [Insert your path logic here as per previous messages]
    const std::string wiiu_path = R"(E:\Emulators\cemu_1.27.1\mlc01\usr\save\00050000\101d9d00\user\80000001\)";

    editor::World world;
    if (world.read(wiiu_path + "260104235224") != 0) return -1;

    world.setWorldName(L"SECTOR_0xFF_FINAL");
    generateComplexARGWorld(world);

    editor::WriteSettings settings(editor::sch::AquaticTU69, lce::CONSOLE::WIIU, wiiu_path);
    world.write(settings);

    printf("--- SYSTEM OVERRIDE COMPLETE. SECTOR 0xFF IS LIVE. ---\n");
    return 0;
}