/**
* loot_chest_world.cpp
*
* Creates a WiiU-format world containing loot chests for every weight value in
* the stronghold-library loot table, for both the Elytra (TU68) and Aquatic
* (TU69) updates.
*
* Layout (top-down, z increases southward):
*   z = GROUP_Z:        version-label sign ("ELYTRA" / "AQUATIC")
*   z = GROUP_Z + 2:    row 0  — chest at (col*2, 64, z), weight sign at (col*2, 64, z+1)
*   z = GROUP_Z + 5:    row 1
*   ...  (row pitch = 3)
*   [5-block gap between the two groups]
*
* Up to COLS_PER_ROW (15) chests per row, column pitch = 2 blocks (avoids
* double-chest detection).  All coordinates stay within -432..431.
*
* Usage:
*   ./LootChestWorld <input_wiiu_world_path> <output_folder_path>
*
* The input world is used only as a save-file skeleton (level.dat, etc.).
* All overworld region files are replaced by the newly generated region.
*/
#include <string>

#include "include/lce/blocks/blockID.hpp"
#include "include/lce/processor.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/World.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "code/convert/Schematic.hpp"
#include "common/nbt.hpp"

using namespace editor;


// ============================================================
//  Loot-table data  (weight, lootTableSeed)
// ============================================================

struct LootEntry {
    int weight;
    long long seed;
};

static constexpr LootEntry ELYTRA_LOOT[] = {
        {0, 4013553},
        {1, 5090235},
        {2, 1474331},
        {3, 2103470},
        {4, 521955},
        {5, 12117206},
        {6, 1987577},
        {7, 47063},
        {8, 1362751},
        {9, 4539203},
        {10, 1273322},
        {11, 10438},
        {12, 564273},
        {13, 4701176},
        {14, 3251529},
        {15, 5079293},
        {16, 829311},
        {17, 3071970},
        {18, 285680},
        {19, 3178196},
        {20, 4756284},
        {21, 1653949},
        {22, 3519097},
        {23, 3120750},
        {24, 4473369},
        {25, 2360345},
        {26, 4903120},
        {27, 2651093},
        {28, 1736119},
        {29, 8613251},
        {30, 9332300},
        {31, 1087419},
        {32, 934973},
        {33, 374628},
        {34, 3544608},
        {35, 2264790},
        {36, 584349},
        {37, 2591730},
        {38, 2456383},
        {39, 1180268},
        {40, 1179139},
        {41, 2724054},
        {42, 2957139},
        {43, 2116222},
        {44, 1108543},
        {45, 4387358},
        {46, 2128527},
        {47, 1496567},
        {48, 1778505},
        {49, 6291030},
        {50, 2124291},
        {51, 32784},
        {52, 2164349},
        {53, 230837},
        {54, 2899900},
        {55, 5986560},
        {56, 3840292},
        {57, 1695768},
        {58, 6342359},
        {59, 233660},
        {60, 7773442},
        {61, 2090064},
        {62, 2620662},
        {63, 1298137},
        {64, 2932909},
        {65, 1042011},
        {66, 2765895},
        {67, 2443875},
        {68, 4290148},
        {69, 14577125},
        {70, 3280887},
        {71, 1761172},
        {72, 6199194},
        {73, 1041414},
        {74, 4231333},
        {75, 1615319},
        {76, 2246775},
        {77, 1058884},
        {78, 896895},
        {79, 4985430},
        {80, 500093},
        {81, 2756161},
        {82, 66037},
        {83, 358470},
        {84, 7027616},
        {85, 6220318},
        {86, 1047880},
        {87, 2068467},
        {88, 455072},
        {89, 2853684},
        {90, 809547},
        {91, 903686},
        {92, 1241613},
        {93, 2428822},
        {94, 3931520},
        {95, 528208},
        {96, 2848104},
        {97, 5154566},
        {98, 994256},
        {99, 2342100},
        {100, 260918},
        {101, 9557331},
        {102, 5802972},
        {103, 698740},
        {104, 2843765},
        {105, 720282},
        {106, 397298},
        {107, 2811198},
        {108, 1751383}};
static constexpr int ELYTRA_COUNT = static_cast<int>(std::size(ELYTRA_LOOT));

static constexpr LootEntry AQUATIC_LOOT[] = {
        {0, 1215587},
        {1, 3931520},
        {2, 1474331},
        {3, 3551417},
        {4, 1987577},
        {5, 4387358},
        {6, 4064072},
        {7, 13646965},
        {8, 3705642},
        {9, 8552523},
        {10, 5460733},
        {11, 1179139},
        {12, 47063},
        {13, 260918},
        {14, 1362751},
        {15, 528208},
        {16, 6199194},
        {17, 2103470},
        {18, 13985422},
        {19, 3844871},
        {20, 3774946},
        {21, 3071970},
        {22, 2342100},
        {23, 2522294},
        {24, 285680},
        {25, 809547},
        {26, 7728811},
        {27, 2164349},
        {28, 7698311},
        {29, 4397982},
        {30, 564089},
        {31, 3380960},
        {32, 4013553},
        {33, 2428822},
        {34, 4692652},
        {35, 1041414},
        {36, 5257774},
        {37, 896895},
        {38, 2394431},
        {39, 233660},
        {40, 32784},
        {41, 2456383},
        {42, 1042011},
        {43, 2110640},
        {44, 2332608},
        {45, 4713724},
        {46, 1695768},
        {47, 2323138},
        {48, 719263},
        {49, 994256},
        {50, 1298137},
        {51, 2423503},
        {52, 1180268},
        {53, 397298},
        {54, 4256075},
        {55, 3145312},
        {56, 5090235},
        {57, 1736119},
        {58, 6758697},
        {59, 2591730},
        {60, 1615319},
        {61, 4903120},
        {62, 1058884},
        {63, 5858950},
        {64, 4117644},
        {65, 10203784},
        {66, 671930},
        {67, 1660465},
        {68, 3627066},
        {69, 1087419},
        {70, 8697129},
        {71, 7752853},
        {72, 2620662},
        {73, 2443875},
        {74, 66037},
        {75, 374628},
        {76, 1267856},
        {77, 2651093},
        {78, 6296305},
        {79, 2128527},
        {80, 3519097},
        {81, 6007150},
        {82, 7999451},
        {83, 2765895},
        {84, 5802972},
        {85, 2246775},
        {86, 1026869},
        {87, 230837},
        {88, 3981545},
        {89, 1778505},
        {90, 1273322},
        {91, 12152392},
        {92, 10438},
        {93, 4100822},
        {94, 11856713},
        {95, 6937911},
        {96, 2724054},
        {97, 3044693},
        {98, 2843765},
        {99, 2163172},
        {100, 4290148},
        {101, 10902267},
        {102, 584349},
        {103, 5251034},
        {104, 1241613},
        {105, 13596123},
        {106, 1343744},
        {107, 1057426},
        {108, 5713903},
        {109, 2899900},
        {110, 698740},
        {111, 455072},
        {112, 2886460},
        {113, 564273},
        {114, 4828110},
        {115, 720282},
        {116, 5685532},
        {117, 2811198},
        {118, 358470},
        {119, 1761172},
        {120, 2360345},
        {121, 5339579},
        {122, 2837217},
        {123, 1496567},
        {124, 1047880},
        {125, 1365931},
        {126, 521955}};
static constexpr int AQUATIC_COUNT = static_cast<int>(std::size(AQUATIC_LOOT));


// ============================================================
//  Layout constants
// ============================================================

static constexpr int CHEST_Y = 4;      // y level for chests / signs
static constexpr int FLOOR_Y = 3;      // y level for stone floor
static constexpr int COLS_PER_ROW = 10; // chests per row
static constexpr int COL_STEP = 2;      // x spacing between chests in a row
static constexpr int ROW_PITCH = 3;     // z slots per row: chest@z, sign@z+1, gap@z+2
static constexpr int GROUP_GAP = 5;     // empty z blocks between the two version groups

// ============================================================
//  Block-placement helpers
// ============================================================

/// Place a chest at (wx, CHEST_Y, wz) with a stronghold-library loot table.
static void placeChest(World& world, int wx, int wz, long long lootSeed) {
    constexpr u16 CHEST_FACING_SOUTH = 0; // data=2: chest opens from the south
    constexpr u16 CHEST_BLOCK = static_cast<u16>((lce::blocks::CHEST_ID << 4) | CHEST_FACING_SOUTH);
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);

    world.setBlock(wx, CHEST_Y, wz, CHEST_BLOCK);
    world.setBlock(wx, FLOOR_Y, wz, STONE_BLOCK);

    world.addTileEntity(wx, CHEST_Y, wz,
                        makeCompound({{"id", makeString("Chest")},
                                      {"x", makeInt(wx)},
                                      {"y", makeInt(CHEST_Y)},
                                      {"z", makeInt(wz)},
                                      {"LootTable", makeString("minecraft:chests/stronghold_library")},
                                      {"LootTableSeed", makeLong(lootSeed)}}));
}

/// Place a standing sign (weight label) at (wx, CHEST_Y, wz).
/// SIGN_FACING_SOUTH (data=0): face points south, readable from the south.
static void placeWeightSign(World& world, int wx, int wz, int weight) {
    constexpr u16 SIGN_FACING_SOUTH = 8; // data=8: sign faces south, readable when approaching from the south
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);
    constexpr u16 SIGN_BLOCK =
            static_cast<u16>((lce::blocks::STANDING_SIGN_BLOCK_ID << 4) | SIGN_FACING_SOUTH);

    world.setBlock(wx, CHEST_Y, wz, SIGN_BLOCK);
    world.setBlock(wx, FLOOR_Y, wz, STONE_BLOCK);

    world.addTileEntity(wx, CHEST_Y, wz,
                        makeCompound({{"id", makeString("Sign")},
                                      {"x", makeInt(wx)},
                                      {"y", makeInt(CHEST_Y)},
                                      {"z", makeInt(wz)},
                                      {"Text1", makeString("Weight: " + std::to_string(weight))},
                                      {"Text2", makeString("")},
                                      {"Text3", makeString("")},
                                      {"Text4", makeString("")}}));
}

/// Place a version-label standing sign at (wx, CHEST_Y, wz).
/// SIGN_FACING_SOUTH (data=0): face points south, readable when approaching from the south.
static void placeVersionSign(World& world, int wx, int wz, const std::string& label) {
    constexpr u16 SIGN_FACING_SOUTH = 8; // data=8: sign faces south, readable when approaching from the south
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);
    constexpr u16 SIGN_BLOCK =
            static_cast<u16>((lce::blocks::STANDING_SIGN_BLOCK_ID << 4) | SIGN_FACING_SOUTH);

    world.setBlock(wx, CHEST_Y, wz, SIGN_BLOCK);
    world.setBlock(wx, FLOOR_Y, wz, STONE_BLOCK);

    world.addTileEntity(wx, CHEST_Y, wz,
                        makeCompound({{"id", makeString("Sign")},
                                      {"x", makeInt(wx)},
                                      {"y", makeInt(CHEST_Y)},
                                      {"z", makeInt(wz)},
                                      {"Text1", makeString(label)},
                                      {"Text2", makeString("")},
                                      {"Text3", makeString("")},
                                      {"Text4", makeString("")}}));
}


// ============================================================
//  Group placer  (returns z coordinate past the last row)
// ============================================================

/**
* Places one version group into `region`:
*   - A version-label sign at (0, CHEST_Y, groupZ)
*   - Rows of chests + weight signs starting at groupZ + 2
*
* Row layout (z direction):
*   rowBaseZ + 0  →  chest block
*   rowBaseZ + 1  →  weight sign (south of chest, readable from further south)
*   rowBaseZ + 2  →  empty gap
*
* X layout: column c → x = c * COL_STEP (pitch of 2 avoids double-chests).
*
* Returns the first free Z coordinate after this group.
*/
static int placeGroup(World& world,
                      int groupZ,
                      const LootEntry* entries,
                      int count,
                      const std::string& versionLabel) {
    // Place the version-label sign for the whole group
    placeVersionSign(world, 0, groupZ, versionLabel);

    const int firstRowZ = groupZ + 2;

    for (int i = 0; i < count; ++i) {
        int row = i / COLS_PER_ROW;
        int col = i % COLS_PER_ROW;

        int wx = col * COL_STEP;
        int chestZ = firstRowZ + row * ROW_PITCH;
        int signZ = chestZ + 1; // sign is one block south of the chest

        placeChest(world, wx, chestZ, entries[i].seed);
        placeWeightSign(world, wx, signZ, entries[i].weight);
    }

    int numRows = (count + COLS_PER_ROW - 1) / COLS_PER_ROW;
    int lastSignZ = firstRowZ + (numRows - 1) * ROW_PITCH + 1;
    return lastSignZ + 2; // first free Z past this group
}


// ============================================================
//  main
// ============================================================

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Usage: %s <input_wiiu_world_path> <output_folder_path>\n",
               argv[0]);
        return 1;
    }

    const std::string inputPath = argv[1];
    const std::string outputPath = argv[2];

    constexpr auto consoleOut = lce::CONSOLE::WIIU;
    // if you write to a playstation console, you need to add a 4th argument to settings
    editor::WriteSettings settings(editor::sch::AquaticTU69, consoleOut, outputPath);

    editor::World world(settings.m_schematic.chunk_lastVersion);
    world.read(inputPath);
    world.setWorldName(L"Utter is actually not bad");

    MU int nextZ = placeGroup(world, 0, ELYTRA_LOOT, ELYTRA_COUNT, "ELYTRA");
    MU int abcde = placeGroup(world, nextZ + GROUP_GAP, AQUATIC_LOOT, AQUATIC_COUNT, "AQUATIC");

    const int statusOut = world.write(settings);
    if (statusOut != 0) {
        printf("converting to %s failed...\n", consoleToCStr(consoleOut));
        return statusOut;
    }

    printf("World written to '%s'\n", outputPath.c_str());
    printf("  ELYTRA entries : %d\n", ELYTRA_COUNT);
    printf("  AQUATIC entries: %d\n", AQUATIC_COUNT);
    return 0;
}
