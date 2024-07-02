#pragma once

#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <set>

#include "include/ghc/fs_std.hpp"

#include "lce/processor.hpp"

#include "LegacyEditor/code/FileListing/LCEFile.hpp"
#include "LegacyEditor/code/FileInfo/FileInfo.hpp"
#include "LegacyEditor/code/Region/RegionManager.hpp"
#include "LegacyEditor/utils/error_status.hpp"


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
        static constexpr u32 WSTRING_SIZE = 64;
        static constexpr u32 UNION_HEADER_SIZE = 12;
        static constexpr u32 FILELISTING_HEADER_SIZE = 12;
        static constexpr u32 FILE_HEADER_SIZE = 144;

    public:

        /// Data
        static bool AUTO_REMOVE_PLAYERS;
        static bool AUTO_REMOVE_DATA_MAPPING;

        fs::path myFilePath;
        std::list<LCEFile> myAllFiles;
        i32 myOldestVersion{};
        i32 myCurrentVersion{};
        lce::CONSOLE myConsole = lce::CONSOLE::NONE;
        FileInfo fileInfo{};
        bool myHasLoadedFileInfo = false;
        bool hasSeparateEntities = false;
        bool myHasSeparateRegions = false;

        /// Pointers

        FileList* dimFileLists[3] = {&region_nether, &region_overworld, &region_end};
        FileList region_nether, region_overworld, region_end;
        FileList maps, structures, players;
        LCEFile *entity_nether{}, *entity_overworld{}, *entity_end{};
        LCEFile *largeMapDataMappings{}, *level{}, *grf{}, *village{};

        /// Constructors

        FileListing() = default;
        ~FileListing() { deallocate(); }

        /// Details

        void printDetails() const;
        void printFileList() const;

        /// Functions

        MU ND int dumpToFolder(const fs::path& inDirPath) const;
        MU void convertRegions(lce::CONSOLE consoleOut);

        /// Modify State

        void deallocate();
        void removeFileTypes(const std::set<LCEFileType>& typesToRemove);
        MU void addFiles(LCEFile_vec filesIn);
        LCEFile_vec collectFiles(LCEFileType fileType);

        /// Read / Write from console files

        MU ND int read(const fs::path& inFilePath, c_bool readEXTFile = false);
        MU ND int write(const fs::path& outFilePath, const lce::CONSOLE consoleOut);


        /// Conversion

        MU ND int convertTo(const fs::path& inFilePath, const fs::path& outFilePath, lce::CONSOLE consoleOut);
        MU ND int convertAndReplaceRegions(const fs::path& inFilePath, const fs::path& inFileRegionReplacementPath,
                                           const fs::path& outFilePath, const lce::CONSOLE consoleOut);

        /// Read / Write external PS4 folders
        std::vector<fs::path> findExternalRegionPS4Folders();
        MU ND int readExternalRegionsPS4_OR_NSX(const fs::path& inDirPath);
        MU ND static int writeExternalRegionsPS4(const fs::path& outDirPath);

        /// Read / Write external Nintendo Switch folders
        MU ND fs::path findExternalRegionNSXFolders() const;
        MU ND int readExternalRegionsNSX(const fs::path& inDirPath);
        MU ND static int writeExternalRegionsNSX(const fs::path& outDirPath);



        /// Region Helpers

        MU void pruneRegions();
        MU void replaceRegionOW(size_t regionIndex, editor::RegionManager& region, const lce::CONSOLE consoleOut);


    private:

        /// Reader

        MU ND int readFile(const std::filesystem::__cxx11::path& inFilePath);
        MU ND int readFileInfo(const fs::path& inDirPath);
        MU ND int readWiiU(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readNSXorPS4(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readVita(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readPs3(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readRpcs3(FILE* f_in, Data& data, u64 source_binary_size);
        MU ND int readXbox360DAT(FILE* f_in, Data& data, u32 file_size, u32 src_size);
        MU ND int readXbox360BIN(FILE* f_in, Data& data, u64 source_binary_size);
        MU   void readData(const Data& dataIn);

        /// Writer

        MU ND int writeFile(const fs::path& outFilePath, const lce::CONSOLE consoleOut);
        MU ND int writeFileInfo(const fs::path& outFilePath, const lce::CONSOLE consoleOut) const;
        MU ND static int writeWiiU(FILE* f_out, const Data& dataOut);
        MU ND static int writeVita(FILE* f_out, const Data& dataOut);
        MU ND static int writeRPCS3(FILE* f_out, const Data& dataOut);
        MU ND static int writePS3();
        MU ND static int writePs4();
        MU ND static int writeNSX();
        MU ND static int writeXbox360_DAT();
        MU ND static int writeXbox360_BIN();
        Data writeData(lce::CONSOLE consoleOut);

        /// File pointer stuff

        void clearPointers();
        void updatePointers();

        /// For use in removeFileTypes

        std::map<LCEFileType, std::function<void()>> clearActionsDelete = {
                {LCEFileType::STRUCTURE, [this] { structures.removeAll(); }},
                {LCEFileType::MAP, [this] { maps.removeAll(); }},
                {LCEFileType::PLAYER, [this] { players.removeAll(); }},
                {LCEFileType::REGION_NETHER, [this] { region_nether.removeAll(); }},
                {LCEFileType::REGION_OVERWORLD, [this] { region_overworld.removeAll(); }},
                {LCEFileType::REGION_END, [this] { region_end.removeAll(); }},
                {LCEFileType::ENTITY_NETHER, [this] { entity_nether->deleteData(); entity_nether = nullptr; }},
                {LCEFileType::ENTITY_OVERWORLD, [this] { entity_overworld->deleteData(); entity_overworld = nullptr; }},
                {LCEFileType::ENTITY_END, [this] { entity_end->deleteData(); entity_end = nullptr; }},
                {LCEFileType::VILLAGE, [this] { village->deleteData(); village = nullptr; }},
                {LCEFileType::DATA_MAPPING, [this] { largeMapDataMappings->deleteData(); largeMapDataMappings = nullptr; }},
                {LCEFileType::LEVEL, [this] { level->deleteData(); level = nullptr; }},
                {LCEFileType::GRF, [this] { grf->deleteData(); grf = nullptr; }},
        };


        std::map<LCEFileType, std::function<void()>> clearActionsRemove = {
                {LCEFileType::STRUCTURE, [this] { structures.clear(); }},
                {LCEFileType::MAP, [this] { maps.clear(); }},
                {LCEFileType::PLAYER, [this] { players.clear(); }},
                {LCEFileType::REGION_NETHER, [this] { region_nether.clear(); }},
                {LCEFileType::REGION_OVERWORLD, [this] { region_overworld.clear(); }},
                {LCEFileType::REGION_END, [this] { region_end.clear(); }},
                {LCEFileType::ENTITY_NETHER, [this] { entity_nether = nullptr; }},
                {LCEFileType::ENTITY_OVERWORLD, [this] { entity_overworld = nullptr; }},
                {LCEFileType::ENTITY_END, [this] { entity_end = nullptr; }},
                {LCEFileType::VILLAGE, [this] { village = nullptr; }},
                {LCEFileType::DATA_MAPPING, [this] { largeMapDataMappings = nullptr; }},
                {LCEFileType::LEVEL, [this] { level = nullptr; }},
                {LCEFileType::GRF, [this] { grf = nullptr; }},
        };


    };
}

