#include "fileListing.hpp"

#include <cstdio>

#include "include/ghc/fs_std.hpp"


#include "include/lce/processor.hpp"

#include "common/NBT.hpp"

#include "code/ConsoleParser/headerUnion.hpp"
#include "code/ConsoleParser/include.hpp"
#include "code/scripts.hpp"


namespace editor {


    FileListing::FileListing() {
        initializeActions();

        typedef std::array<std::unique_ptr<ConsoleParser>, 2> consoleParserArrayUQPtr;
        consoleInstances.emplace(lce::CONSOLE::XBOX360, consoleParserArrayUQPtr{std::make_unique<Xbox360DAT>(), std::make_unique<Xbox360BIN>()});
        consoleInstances.emplace(lce::CONSOLE::PS3, consoleParserArrayUQPtr{std::make_unique<PS3>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::RPCS3, consoleParserArrayUQPtr{std::make_unique<RPCS3>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::PS4, consoleParserArrayUQPtr{std::make_unique<PS4>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::VITA, consoleParserArrayUQPtr{std::make_unique<Vita>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::WIIU, consoleParserArrayUQPtr{std::make_unique<WiiU>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::SWITCH, consoleParserArrayUQPtr{std::make_unique<Switch>(), nullptr});
        consoleInstances.emplace(lce::CONSOLE::XBOX1, consoleParserArrayUQPtr{std::make_unique<Xbox1>(), nullptr});
    }


    FileListing::~FileListing() {
        deallocate();
    }


    int FileListing::read(const fs::path& theFilePath) {
        myReadSettings.setFilePath(theFilePath);

        i32 status1 = findConsole(theFilePath);
        if (status1 != SUCCESS) {
            printf("Failed to find console from %s\n", theFilePath.string().c_str());
            return status1;
        }

        i32 status2 = readSave();
        if (status2 != SUCCESS) {
            printf("Failed to read save from %s\n", theFilePath.string().c_str());
            return status2;
        }

        return SUCCESS;
    }


    int FileListing::readSave() {
        int readerIndex = 0;
        auto it = consoleInstances.find(myReadSettings.getConsole());
        if (it != consoleInstances.end()) {

            if (myReadSettings.getIsXbox360BIN() && myReadSettings.getConsole() == lce::CONSOLE::XBOX360)
                readerIndex = 1; // use the .bin reader instead

            int status = it->second[readerIndex]->read(this, myReadSettings.getFilePath());
            // printf("detected save as %s\n", lce::consoleToStr(myConsole).c_str());
            return status;
        }
        return INVALID_CONSOLE;
    }


    int FileListing::writeSave(WriteSettings& theSettings) {
        auto it = consoleInstances.find(theSettings.getConsole());
        if (it != consoleInstances.end()) {
            int status = it->second[0]->write(this, theSettings);
            if (status != 0) {
                printf("failed to write save %s.\n", theSettings.getInFolderPath().string().c_str());
            }
            return status;
        }
        return INVALID_CONSOLE;
    }


    int FileListing::write(WriteSettings& theWriteSettings) {
        if (!theWriteSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        // TODO: create default output file path if not set

        if (myReadSettings.getConsole() != theWriteSettings.getConsole()) {
            if (theWriteSettings.shouldRemovePlayers) {
                removeFileTypes({lce::FILETYPE::PLAYER});
            }
            if (theWriteSettings.shouldRemoveDataMapping) {
                removeFileTypes({lce::FILETYPE::DATA_MAPPING});
            }
            if (theWriteSettings.shouldRemoveMaps) {
                removeFileTypes({lce::FILETYPE::MAP});
            }
        }

        const auto consoleOut = theWriteSettings.getConsole();
        if (lce::consoleIsBigEndian(myReadSettings.getConsole()) != lce::consoleIsBigEndian(consoleOut)) {
            std::cout << "[-] reading and writing all chunks to change their endian, this will take a minute." << std::endl;
            for (size_t index = 0; index < ptrs.region_overworld.size(); index++) {
                editor::convertChunksToAquatic(index, ptrs.region_overworld, myReadSettings.getConsole(), consoleOut);
            }
            for (size_t index = 0; index < ptrs.region_nether.size(); index++) {
                editor::convertChunksToAquatic(index, ptrs.region_nether, myReadSettings.getConsole(), consoleOut);
            }
            for (size_t index = 0; index < ptrs.region_end.size(); index++) {
                editor::convertChunksToAquatic(index, ptrs.region_end, myReadSettings.getConsole(), consoleOut);
            }
            removeFileTypes({lce::FILETYPE::STRUCTURE});
            removeFileTypes({lce::FILETYPE::GRF});
        } else {
            
            convertRegions(theWriteSettings.getConsole());
        }



        int status = writeSave(theWriteSettings);
        if (status != 0) {
            printf("failed to write gamedata to %s", theWriteSettings.getInFolderPath().string().c_str());
        }
        return status;
    }


    int FileListing::findConsole(const fs::path& inFilePath) {
        static constexpr uint32_t CON_MAGIC = 0x434F4E20;
        static constexpr uint32_t ZLIB_MAGIC = 0x789C;


        FILE* f_in = fopen(inFilePath.string().c_str(), "rb");
        if (f_in == nullptr) {
            return printf_err(FILE_ERROR, ERROR_4, inFilePath.string().c_str());
        }

        fseek(f_in, 0, SEEK_END);
        c_u64 input_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);
        if (input_size < 12) {
            return printf_err(FILE_ERROR, ERROR_5);
        }
        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, 12, f_in);
        fclose(f_in);

        Data data;
        data.setScopeDealloc(true);
        if (headerUnion.getInt1() <= 2) {
            if (headerUnion.getShort5() == ZLIB_MAGIC) {
                if (headerUnion.getInt2Swap() >= headerUnion.getDestSize()) {
                    myReadSettings.setConsole(lce::CONSOLE::WIIU);
                } else {
                    const std::string parentDir = myReadSettings.getFilePath().parent_path().filename().string();
                    myReadSettings.setConsole(lce::CONSOLE::SWITCH);
                    if (parentDir == "savedata0") {
                        myReadSettings.setConsole(lce::CONSOLE::PS4);
                    }
                }
            } else {
                // TODO: change this to write custom checker for FILE_COUNT * 144 == diff. with
                // TODO: with custom vitaRLE decompress checker
                c_u32 indexFromSF = headerUnion.getInt2Swap() - headerUnion.getInt3Swap();
                if (indexFromSF > 0 && indexFromSF < 65536) {
                    myReadSettings.setConsole(lce::CONSOLE::VITA);
                } else { // compressed ps3
                    myReadSettings.setConsole(lce::CONSOLE::PS3);
                }
            }
        } else if (headerUnion.getInt2() <= 2) {
            /// if (int2 == 0) it is an xbox savefile unless it's a massive
            /// file, but there won't be 2 files in a savegame file for PS3
            myReadSettings.setConsole(lce::CONSOLE::XBOX360);
            myReadSettings.setIsXbox360BIN(false);
            // TODO: don't use arbitrary guess for a value
        } else if (headerUnion.getInt2() < 100) { // uncompressed PS3 / RPCS3
            /// otherwise if (int2) > 100 then it is a random file
            /// because likely ps3 won't have more than 100 files
            myReadSettings.setConsole(lce::CONSOLE::RPCS3);
        } else if (headerUnion.getInt1() == CON_MAGIC) {
            myReadSettings.setConsole(lce::CONSOLE::XBOX360);
            myReadSettings.setIsXbox360BIN(true);
        } else {
            return printf_err(INVALID_SAVE, ERROR_3);
        }

        return SUCCESS;
    }


    void FileListing::initializeActions() {
        ptrs.clearDelete = {
                {lce::FILETYPE::STRUCTURE, [this] { ptrs.structures.removeAll(); }},
                {lce::FILETYPE::MAP, [this] { ptrs.maps.removeAll(); }},
                {lce::FILETYPE::PLAYER, [this] { ptrs.players.removeAll(); }},
                {lce::FILETYPE::REGION_NETHER, [this] { ptrs.region_nether.removeAll(); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this] { ptrs.region_overworld.removeAll(); }},
                {lce::FILETYPE::REGION_END, [this] { ptrs.region_end.removeAll(); }},
                {lce::FILETYPE::ENTITY_NETHER, [this] { ptrs.entity_nether->deleteData(); ptrs.entity_nether = nullptr; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this] { ptrs.entity_overworld->deleteData(); ptrs.entity_overworld = nullptr; }},
                {lce::FILETYPE::ENTITY_END, [this] { ptrs.entity_end->deleteData(); ptrs.entity_end = nullptr; }},
                {lce::FILETYPE::VILLAGE, [this] { ptrs.village->deleteData(); ptrs.village = nullptr; }},
                {lce::FILETYPE::DATA_MAPPING, [this] { ptrs.largeMapDataMappings->deleteData(); ptrs.largeMapDataMappings = nullptr; }},
                {lce::FILETYPE::LEVEL, [this] { ptrs.level->deleteData(); ptrs.level = nullptr; }},
                {lce::FILETYPE::GRF, [this] { ptrs.grf->deleteData(); ptrs.grf = nullptr; }},
        };

        ptrs.clearRemove = {
                {lce::FILETYPE::STRUCTURE, [this] { ptrs.structures.clear(); }},
                {lce::FILETYPE::MAP, [this] { ptrs.maps.clear(); }},
                {lce::FILETYPE::PLAYER, [this] { ptrs.players.clear(); }},
                {lce::FILETYPE::REGION_NETHER, [this] { ptrs.region_nether.clear(); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this] { ptrs.region_overworld.clear(); }},
                {lce::FILETYPE::REGION_END, [this] { ptrs.region_end.clear(); }},
                {lce::FILETYPE::ENTITY_NETHER, [this] { ptrs.entity_nether = nullptr; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this] { ptrs.entity_overworld = nullptr; }},
                {lce::FILETYPE::ENTITY_END, [this] { ptrs.entity_end = nullptr; }},
                {lce::FILETYPE::VILLAGE, [this] { ptrs.village = nullptr; }},
                {lce::FILETYPE::DATA_MAPPING, [this] { ptrs.largeMapDataMappings = nullptr; }},
                {lce::FILETYPE::LEVEL, [this] { ptrs.level = nullptr; }},
                {lce::FILETYPE::GRF, [this] { ptrs.grf = nullptr; }},
        };

        ptrs.addUpdate = {
                {lce::FILETYPE::STRUCTURE, [this](LCEFile& file) { ptrs.structures.push_back(&file); }},
                {lce::FILETYPE::VILLAGE, [this](LCEFile& file) { ptrs.village = &file; }},
                {lce::FILETYPE::DATA_MAPPING, [this](LCEFile& file) { ptrs.largeMapDataMappings = &file; }},
                {lce::FILETYPE::MAP, [this](LCEFile& file) { ptrs.maps.push_back(&file); }},
                {lce::FILETYPE::REGION_NETHER, [this](LCEFile& file) { ptrs.region_nether.push_back(&file); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this](LCEFile& file) { ptrs.region_overworld.push_back(&file); }},
                {lce::FILETYPE::REGION_END, [this](LCEFile& file) { ptrs.region_end.push_back(&file); }},
                {lce::FILETYPE::PLAYER, [this](LCEFile& file) { ptrs.players.push_back(&file); }},
                {lce::FILETYPE::LEVEL, [this](LCEFile& file) { ptrs.level = &file; }},
                {lce::FILETYPE::GRF, [this](LCEFile& file) { ptrs.grf = &file; }},
                {lce::FILETYPE::ENTITY_NETHER, [this](LCEFile& file) { ptrs.entity_nether = &file; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this](LCEFile& file) { ptrs.entity_overworld = &file; }},
                {lce::FILETYPE::ENTITY_END, [this](LCEFile& file) { ptrs.entity_end = &file; }},
        };
    }


}