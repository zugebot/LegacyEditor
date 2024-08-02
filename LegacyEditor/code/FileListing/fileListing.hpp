#pragma once

#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <set>

#include "include/ghc/fs_std.hpp"

#include "lce/processor.hpp"
#include "lce/include/picture.hpp"

#include "conversionSettings.hpp"

#include "LegacyEditor/code/ConsoleParser/ConsoleParser.hpp"
#include "LegacyEditor/code/FileInfo/FileInfo.hpp"
#include "LegacyEditor/code/LCEFile/LCEFile.hpp"
#include "LegacyEditor/code/Region/RegionManager.hpp"
#include "LegacyEditor/utils/error_status.hpp"


class ConsoleParser;

namespace editor {


    class FileList : public std::vector<LCEFile*> {
    public:
        void removeAll() {
            for (LCEFile* file : *this) {
                delete[] file->data.data;
                file->data.data = nullptr;
            }
            clear();
        }
    };


    class FileListing {
        std::unordered_map<lce::CONSOLE, std::unique_ptr<ConsoleParser>> consoleInstances;

    public:

        /// SETTINGS
        static bool AUTO_REMOVE_PLAYERS;
        static bool AUTO_REMOVE_DATA_MAPPING;

        fs::path myFilePath;
        std::list<LCEFile> myAllFiles;
        i32 myOldestVersion{};
        i32 myCurrentVersion{};
        lce::CONSOLE myConsole = lce::CONSOLE::NONE;
        FileInfo fileInfo{};

        bool myHasSeparateRegions = false;

        /// Pointers

        FileList* dimFileLists[3] = {&region_nether, &region_overworld, &region_end};

        FileList region_nether;
        FileList region_overworld;
        FileList region_end;
        FileList maps;
        FileList structures;
        FileList players;


        LCEFile *entity_nether{};
        LCEFile *entity_overworld{};
        LCEFile *entity_end{};
        LCEFile *largeMapDataMappings{};
        LCEFile *level{};
        LCEFile *grf{};
        LCEFile *village{};

        Picture icon0png;

        /// Constructors

        FileListing();
        ~FileListing();

        /// Details

        void printDetails() const;
        void printFileList() const;

        /// Functions

        MU ND int dumpToFolder(const fs::path& inDirPath) const;

        /// Modify State

        void deallocate();
        void removeFileTypes(const std::set<lce::FILETYPE>& typesToRemove);
        MU void addFiles(std::list<LCEFile> filesIn);
        std::list<LCEFile> collectFiles(lce::FILETYPE fileType);

        /// Parse from console files

        MU ND int read(const fs::path& inFilePath);
        MU ND int write(ConvSettings& theSettings);

        /// Conversion

        MU ND int convertTo(const fs::path& inFilePath, const fs::path& outFilePath, lce::CONSOLE consoleOut);
        MU ND int convertAndReplaceRegions(const fs::path& inFilePath, const fs::path& inFileRegionReplacementPath,
                                           const fs::path& outFilePath, const lce::CONSOLE consoleOut);

        /// Region Helpers

        MU void convertRegions(lce::CONSOLE consoleOut);
        MU void pruneRegions();
        MU void replaceRegionOW(size_t regionIndex, editor::RegionManager& region, const lce::CONSOLE consoleOut);

        /// File pointer stuff

        void clearPointers();
        void updatePointers();

    private:

        /// Parser

        MU ND int findConsole(const fs::path& inFilePath);
        MU ND int readSave();
        int writeSave(ConvSettings& theSettings);


        /// For use in removeFileTypes

        std::map<lce::FILETYPE, std::function<void()>> actionsClearDelete = {
                {lce::FILETYPE::STRUCTURE, [this] { structures.removeAll(); }},
                {lce::FILETYPE::MAP, [this] { maps.removeAll(); }},
                {lce::FILETYPE::PLAYER, [this] { players.removeAll(); }},
                {lce::FILETYPE::REGION_NETHER, [this] { region_nether.removeAll(); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this] { region_overworld.removeAll(); }},
                {lce::FILETYPE::REGION_END, [this] { region_end.removeAll(); }},
                {lce::FILETYPE::ENTITY_NETHER, [this] { entity_nether->deleteData(); entity_nether = nullptr; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this] { entity_overworld->deleteData(); entity_overworld = nullptr; }},
                {lce::FILETYPE::ENTITY_END, [this] { entity_end->deleteData(); entity_end = nullptr; }},
                {lce::FILETYPE::VILLAGE, [this] { village->deleteData(); village = nullptr; }},
                {lce::FILETYPE::DATA_MAPPING, [this] { largeMapDataMappings->deleteData(); largeMapDataMappings = nullptr; }},
                {lce::FILETYPE::LEVEL, [this] { level->deleteData(); level = nullptr; }},
                {lce::FILETYPE::GRF, [this] { grf->deleteData(); grf = nullptr; }},
        };


        std::map<lce::FILETYPE, std::function<void()>> actionsClearRemove = {
                {lce::FILETYPE::STRUCTURE, [this] { structures.clear(); }},
                {lce::FILETYPE::MAP, [this] { maps.clear(); }},
                {lce::FILETYPE::PLAYER, [this] { players.clear(); }},
                {lce::FILETYPE::REGION_NETHER, [this] { region_nether.clear(); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this] { region_overworld.clear(); }},
                {lce::FILETYPE::REGION_END, [this] { region_end.clear(); }},
                {lce::FILETYPE::ENTITY_NETHER, [this] { entity_nether = nullptr; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this] { entity_overworld = nullptr; }},
                {lce::FILETYPE::ENTITY_END, [this] { entity_end = nullptr; }},
                {lce::FILETYPE::VILLAGE, [this] { village = nullptr; }},
                {lce::FILETYPE::DATA_MAPPING, [this] { largeMapDataMappings = nullptr; }},
                {lce::FILETYPE::LEVEL, [this] { level = nullptr; }},
                {lce::FILETYPE::GRF, [this] { grf = nullptr; }},
        };


        std::map<lce::FILETYPE, std::function<void(LCEFile&)>> actionsUpdate = {
                {lce::FILETYPE::STRUCTURE, [this](LCEFile& file) { structures.push_back(&file); }},
                {lce::FILETYPE::VILLAGE, [this](LCEFile& file) { village = &file; }},
                {lce::FILETYPE::DATA_MAPPING, [this](LCEFile& file) { largeMapDataMappings = &file; }},
                {lce::FILETYPE::MAP, [this](LCEFile& file) { maps.push_back(&file); }},
                {lce::FILETYPE::REGION_NETHER, [this](LCEFile& file) { region_nether.push_back(&file); }},
                {lce::FILETYPE::REGION_OVERWORLD, [this](LCEFile& file) { region_overworld.push_back(&file); }},
                {lce::FILETYPE::REGION_END, [this](LCEFile& file) { region_end.push_back(&file); }},
                {lce::FILETYPE::PLAYER, [this](LCEFile& file) { players.push_back(&file); }},
                {lce::FILETYPE::LEVEL, [this](LCEFile& file) { level = &file; }},
                {lce::FILETYPE::GRF, [this](LCEFile& file) { grf = &file; }},
                {lce::FILETYPE::ENTITY_NETHER, [this](LCEFile& file) { entity_nether = &file; }},
                {lce::FILETYPE::ENTITY_OVERWORLD, [this](LCEFile& file) { entity_overworld = &file; }},
                {lce::FILETYPE::ENTITY_END, [this](LCEFile& file) { entity_end = &file; }},
        };


    };
}

