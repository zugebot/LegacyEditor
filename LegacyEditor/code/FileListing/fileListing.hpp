#pragma once

#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <set>

#include "include/ghc/fs_std.hpp"

#include "lce/processor.hpp"
#include "lce/include/picture.hpp"

#include "stateSettings.hpp"
#include "writeSettings.hpp"

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
        std::unordered_map<lce::CONSOLE, std::array<std::unique_ptr<ConsoleParser>, 2>> consoleInstances;

    public:
        StateSettings myReadSettings;
        // this can probably be renamed to "myInternalFiles"
        std::list<LCEFile> myAllFiles;

        // these can probably be stored in a std::list called external files
        FileInfo fileInfo{};
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

        MU ND int read(const fs::path& theFilePath);
        MU ND int write(WriteSettings& theWriteSettings);

        /// Conversion

        MU ND int convertTo(const fs::path& inFilePath, const fs::path& outFilePath, lce::CONSOLE consoleOut);
        MU ND int convertAndReplaceRegions(const fs::path& inFilePath, const fs::path& inFileRegionReplacementPath,
                                           const fs::path& outFilePath, lce::CONSOLE consoleOut);

        /// Region Helpers

        MU void convertRegions(lce::CONSOLE consoleOut);
        MU void pruneRegions();
        MU void replaceRegionOW(size_t regionIndex, editor::RegionManager& region, lce::CONSOLE consoleOut);

        /// File pointer stuff

        void clearPointers();
        void updatePointers();

    private:

        /// Parser

        MU ND int findConsole(const fs::path& inFilePath);
        MU ND int readSave();
        int writeSave(WriteSettings& theSettings);

    public:

        /// Pointers
        struct {
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

            std::map<lce::FILETYPE, std::function<void()>> clearDelete;
            std::map<lce::FILETYPE, std::function<void()>> clearRemove;
            std::map<lce::FILETYPE, std::function<void(LCEFile&)>> addUpdate;
        } ptrs;

        void initializeActions();

    };


}

