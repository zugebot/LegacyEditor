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

#include <iostream>
#include <string>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"
#include "include/lce/blocks/blockID.hpp"

#include "code/include.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "code/Region/Region.hpp"
#include "code/convert/Schematic.hpp"
#include "common/nbt.hpp"

using namespace editor;


// ============================================================
//  Loot-table data  (weight, lootTableSeed)
// ============================================================

struct LootEntry { int weight; long long seed; };

static constexpr LootEntry ELYTRA_LOOT[] = {
    {0,26358},{1,10827},{2,3068},{3,11846},{4,2558},{5,20114},{6,14349},{7,2933},
    {8,6079},{9,24950},{10,2423},{11,1843},{12,19685},{13,4222},{14,81455},
    {15,13376},{16,1672},{17,1010},{18,5111},{19,28621},{20,12291},{21,9686},
    {22,843},{23,6014},{24,9111},{25,13023},{26,9519},{27,820},{28,30285},
    {29,18038},{30,6196},{31,3531},{32,653},{33,70480},{34,14872},{35,7053},
    {36,3392},{37,25437},{38,16630},{39,5195},{40,12068},{41,33892},{42,19568},
    {43,13871},{44,6080},{45,11239},{46,11350},{47,19318},{48,28309},{49,23419},
    {50,4995},{51,2373},{52,37720},{53,791},{54,208},{55,1283},{56,18053},
    {57,9723},{58,22158},{59,876},{60,1176},{61,17922},{62,22395},{63,8634},
    {64,7576},{65,25896},{66,347},{67,49567},{68,8499},{69,3864},{70,25761},
    {71,17038},{72,29789},{73,17478},{74,3035},{75,87269},{76,32685},{77,1788},
    {78,11490},{79,6557},{80,2896},{81,68746},{82,10364},{83,15012},{84,10429},
    {85,33396},{86,25577},{87,564},{88,5584},{89,1895},{90,31146},{91,10163},
    {92,23438},{93,5417},{94,45451},{95,27241},{96,9992},{97,487},{98,12986},
    {99,703},{100,71500},{101,9163},{102,380},{103,3494},{104,26489},{105,17426},
    {106,9024},{107,2252},{108,3391}
};
static constexpr int ELYTRA_COUNT = static_cast<int>(std::size(ELYTRA_LOOT));

static constexpr LootEntry AQUATIC_LOOT[] = {
    {0,16903},{1,16426},{2,20063},{3,31968},{4,876},{5,17955},{6,64671},
    {7,17478},{8,51946},{9,1672},{10,31786},{11,19568},{12,8499},{13,12068},
    {14,15539},{15,15012},{16,9992},{17,14539},{18,9024},{19,43492},{20,12986},
    {21,23332},{22,45905},{23,15591},{24,44288},{25,7053},{26,1788},{27,5195},
    {28,564},{29,27241},{30,4995},{31,4532},{32,14349},{33,7628},{34,7137},
    {35,35561},{36,5584},{37,5061},{38,5111},{39,4584},{40,32784},{41,3035},
    {42,22158},{43,2558},{44,67263},{45,20114},{46,50147},{47,11846},{48,18655},
    {49,52524},{50,17038},{51,28434},{52,8802},{53,20166},{54,791},{55,11387},
    {56,33892},{57,95840},{58,2933},{59,44032},{60,1843},{61,9111},{62,42212},
    {63,380},{64,3729},{65,34253},{66,11239},{67,10163},{68,48593},{69,1176},
    {70,653},{71,703},{72,26242},{73,93684},{74,18038},{75,14516},{76,6196},
    {77,2252},{78,32346},{79,31865},{80,208},{81,26358},{82,7248},{83,29302},
    {84,40666},{85,6248},{86,29070},{87,13023},{88,4172},{89,4222},{90,27036},
    {91,22892},{92,10438},{93,21802},{94,21343},{95,55212},{96,12851},{97,23229},
    {98,5028},{99,18758},{100,44900},{101,10298},{102,21171},{103,6080},
    {104,8222},{105,19095},{106,4888},{107,4365},{108,77626},{109,3068},
    {110,13376},{111,35430},{112,5417},{113,34364},{114,487},{115,3864},
    {116,10364},{117,3391},{118,2373},{119,6014},{120,1283},{121,820},
    {122,50040},{123,347},{124,65334},{125,18169},{126,17426}
};
static constexpr int AQUATIC_COUNT = static_cast<int>(std::size(AQUATIC_LOOT));


// ============================================================
//  Layout constants
// ============================================================

static constexpr int CHEST_Y       = 64;   // y level for chests / signs
static constexpr int FLOOR_Y       = 63;   // y level for stone floor
static constexpr int COLS_PER_ROW  = 15;   // chests per row
static constexpr int COL_STEP      = 2;    // x spacing between chests in a row
static constexpr int ROW_PITCH     = 3;    // z slots per row: chest@z, sign@z+1, gap@z+2
static constexpr int GROUP_GAP     = 5;    // empty z blocks between the two version groups


// ============================================================
//  Chunk helpers
// ============================================================

static void initChunk(ChunkData* d, int cx, int cz) {
    d->chunkX              = cx;
    d->chunkZ              = cz;
    d->chunkHeight         = 256;
    d->blocks              = u16_vec(65536, 0);
    d->submerged           = u16_vec(65536, 0);
    d->skyLight            = u8_vec(32768, 0xFFu);
    d->blockLight          = u8_vec(32768, 0x00u);
    d->heightMap           = u8_vec(256,   0);
    d->biomes              = u8_vec(256,   1);   // 1 = plains
    d->terrainPopulatedFlags = 2046;
    d->entities            = NBTList(eNBT::COMPOUND);
    d->tileEntities        = NBTList(eNBT::COMPOUND);
    d->tileTicks           = NBTList(eNBT::COMPOUND);
    d->lastUpdate          = 0;
    d->inhabitedTime       = 0;
    d->lastVersion         = eChunkVersion::V_12;
    d->intel.hasBiomes     = true;
    d->validChunk          = true;
}

/// Return (creating if necessary) the ChunkHandle for the chunk that
/// contains world-block (wx, wz).  Only handles chunks in region (0,0).
static ChunkHandle* getOrMakeChunk(Region& region, int wx, int wz) {
    int cx = wx >> 4;   // world chunk X
    int cz = wz >> 4;   // world chunk Z
    // region (0,0) local positions equal world chunk coords directly
    ChunkHandle* handle = region.getChunk(cx, cz);
    if (!handle) return nullptr;

    if (!handle->data->validChunk) {
        initChunk(handle->data.get(), cx, cz);
        // Buffer must be non-empty so Region::write() picks it up for encoding
        handle->buffer.allocate(1);
        handle->buffer.data()[0] = 0;
    }
    return handle;
}

static void placeBlock(Region& region, int wx, int wy, int wz, u16 encoded) {
    ChunkHandle* h = getOrMakeChunk(region, wx, wz);
    if (!h) return;
    ChunkData* d = h->data.get();
    int lx = wx & 15;
    int lz = wz & 15;
    d->setBlock(lx, wy, lz, encoded);
    // update height map
    u8& hm = d->heightMap[lx + lz * 16];
    if (wy + 1 > static_cast<int>(hm))
        hm = static_cast<u8>(wy + 1);
}

static void addTileEntity(Region& region, int wx, int wz, NBTBase te) {
    ChunkHandle* h = getOrMakeChunk(region, wx, wz);
    if (!h) return;
    h->data->tileEntities.push_back(std::move(te));
}


// ============================================================
//  Block-placement helpers
// ============================================================

/// Place a chest at (wx, CHEST_Y, wz) with a stronghold-library loot table.
static void placeChest(Region& region, int wx, int wz, long long lootSeed) {
    constexpr u16 CHEST_FACING_NORTH = 2;  // data=2: chest opens from the south
    constexpr u16 CHEST_BLOCK = static_cast<u16>((lce::blocks::CHEST_ID << 4) | CHEST_FACING_NORTH);
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);

    placeBlock(region, wx, CHEST_Y,  wz, CHEST_BLOCK);
    placeBlock(region, wx, FLOOR_Y,  wz, STONE_BLOCK);

    addTileEntity(region, wx, wz,
        makeCompound({
            {"id",            makeString("Chest")},
            {"x",             makeInt(wx)},
            {"y",             makeInt(CHEST_Y)},
            {"z",             makeInt(wz)},
            {"LootTable",     makeString("minecraft:chests/stronghold_library")},
            {"LootTableSeed", makeLong(lootSeed)}
        }));
}

/// Place a standing sign (weight label) at (wx, CHEST_Y, wz).
/// SIGN_FACING_SOUTH (data=0): face points south, readable from the south.
static void placeWeightSign(Region& region, int wx, int wz, int weight) {
    constexpr u16 SIGN_FACING_SOUTH = 0;
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);
    constexpr u16 SIGN_BLOCK  =
        static_cast<u16>((lce::blocks::STANDING_SIGN_BLOCK_ID << 4) | SIGN_FACING_SOUTH);

    placeBlock(region, wx, CHEST_Y, wz, SIGN_BLOCK);
    placeBlock(region, wx, FLOOR_Y, wz, STONE_BLOCK);

    addTileEntity(region, wx, wz,
        makeCompound({
            {"id",    makeString("Sign")},
            {"x",     makeInt(wx)},
            {"y",     makeInt(CHEST_Y)},
            {"z",     makeInt(wz)},
            {"Text1", makeString("Weight: " + std::to_string(weight))},
            {"Text2", makeString("")},
            {"Text3", makeString("")},
            {"Text4", makeString("")}
        }));
}

/// Place a version-label standing sign at (wx, CHEST_Y, wz).
/// SIGN_FACING_SOUTH (data=0): face points south, readable when approaching from the south.
static void placeVersionSign(Region& region, int wx, int wz,
                              const std::string& label) {
    constexpr u16 SIGN_FACING_SOUTH = 0;
    constexpr u16 STONE_BLOCK = static_cast<u16>(lce::blocks::STONE_ID << 4);
    constexpr u16 SIGN_BLOCK  =
        static_cast<u16>((lce::blocks::STANDING_SIGN_BLOCK_ID << 4) | SIGN_FACING_SOUTH);

    placeBlock(region, wx, CHEST_Y, wz, SIGN_BLOCK);
    placeBlock(region, wx, FLOOR_Y, wz, STONE_BLOCK);

    addTileEntity(region, wx, wz,
        makeCompound({
            {"id",    makeString("Sign")},
            {"x",     makeInt(wx)},
            {"y",     makeInt(CHEST_Y)},
            {"z",     makeInt(wz)},
            {"Text1", makeString(label)},
            {"Text2", makeString("")},
            {"Text3", makeString("")},
            {"Text4", makeString("")}
        }));
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
static int placeGroup(Region& region,
                      int groupZ,
                      const LootEntry* entries,
                      int count,
                      const std::string& versionLabel) {
    // Place the version-label sign for the whole group
    placeVersionSign(region, 0, groupZ, versionLabel);

    const int firstRowZ = groupZ + 2;

    for (int i = 0; i < count; ++i) {
        int row = i / COLS_PER_ROW;
        int col = i % COLS_PER_ROW;

        int wx       = col * COL_STEP;
        int chestZ   = firstRowZ + row * ROW_PITCH;
        int signZ    = chestZ + 1;   // sign is one block south of the chest

        placeChest      (region, wx, chestZ, entries[i].seed);
        placeWeightSign (region, wx, signZ,  entries[i].weight);
    }

    int numRows    = (count + COLS_PER_ROW - 1) / COLS_PER_ROW;
    int lastSignZ  = firstRowZ + (numRows - 1) * ROW_PITCH + 1;
    return lastSignZ + 2;   // first free Z past this group
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

    const std::string inputPath  = argv[1];
    const std::string outputPath = argv[2];

    // ── 1. Load the template world ──────────────────────────────────────────
    SaveProject saveProject;
    if (int st = saveProject.read(inputPath); st != 0) {
        printf("ERROR: failed to load world from '%s'\n", inputPath.c_str());
        return st;
    }
    printf("Loaded world from '%s'\n", inputPath.c_str());

    // ── 2. Strip all existing region files ─────────────────────────────────
    saveProject.removeFileTypes({
        lce::FILETYPE::OLD_REGION_OVERWORLD,
        lce::FILETYPE::OLD_REGION_NETHER,
        lce::FILETYPE::OLD_REGION_END,
        lce::FILETYPE::NEW_REGION_OVERWORLD,
        lce::FILETYPE::NEW_REGION_NETHER,
        lce::FILETYPE::NEW_REGION_END,
        lce::FILETYPE::PLAYER,
        lce::FILETYPE::MAP,
        lce::FILETYPE::VILLAGE,
        lce::FILETYPE::DATA_MAPPING,
    });

    // ── 3. Build the overworld region ───────────────────────────────────────
    Region region(0, 0);

    // Place ELYTRA group starting at z = 0
    int nextZ = placeGroup(region, 0, ELYTRA_LOOT, ELYTRA_COUNT, "ELYTRA");

    // Place AQUATIC group with a gap after ELYTRA
    placeGroup(region, nextZ + GROUP_GAP, AQUATIC_LOOT, AQUATIC_COUNT, "AQUATIC");

    // ── 4. Encode all chunks and write the region buffer ───────────────────
    const sch::Schematic& schema = sch::AquaticTU69;
    WriteSettings settings(schema, lce::CONSOLE::WIIU, outputPath);

    Buffer regionBuf = region.write(settings);
    if (regionBuf.empty()) {
        printf("ERROR: region write produced no data\n");
        return 1;
    }

    // ── 5. Add the new region file to the project ──────────────────────────
    LCEFile& regionFile = saveProject.emplaceFile(
        lce::CONSOLE::WIIU,
        static_cast<u64>(std::time(nullptr)),
        saveProject.m_tempFolder,
        saveProject.m_tempFolder,
        ""          // fileName set below
    );
    regionFile.setType(lce::FILETYPE::OLD_REGION_OVERWORLD);
    regionFile.setRegionX(0);
    regionFile.setRegionZ(0);
    regionFile.setFileName(regionFile.constructFileName());
    regionFile.setBuffer(std::move(regionBuf));

    // ── 6. Write the world ─────────────────────────────────────────────────
    saveProject.setOldestVersion(schema.fileListing_oldestVersion);
    saveProject.setLatestVersion(schema.fileListing_latestVersion);

    if (int st = saveProject.write(settings); st != 0) {
        printf("ERROR: failed to write world to '%s'\n", outputPath.c_str());
        return st;
    }

    printf("World written to '%s'\n", outputPath.c_str());
    printf("  ELYTRA entries : %d\n", ELYTRA_COUNT);
    printf("  AQUATIC entries: %d\n", AQUATIC_COUNT);
    return 0;
}
