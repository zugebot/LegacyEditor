#pragma once

#include <functional>
#include <map>
#include <set>
#include <list>

#include "LegacyEditor/LCE/FileInfo/FileInfo.hpp"
#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "file.hpp"


namespace editor {


    class FileList : public std::vector<File*> {
    public:
        void removeAll() {
            for (File* file : *this) {
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

        std::string filename;
        std::list<File> allFiles;
        i32 oldestVersion{};
        i32 currentVersion{};
        CONSOLE console = CONSOLE::NONE;
        FileInfo fileInfo;
        bool separateEntities = false;
        bool separateRegions = false;

        /// Pointers

        FileList* dimFileLists[3] = {&region_nether, &region_overworld, &region_end};
        FileList region_nether, region_overworld, region_end;
        FileList maps, structures, players;
        File *entity_nether{}, *entity_overworld{}, *entity_end{};
        File *largeMapDataMappings{}, *level{}, *grf{}, *village{};

        /// Constructors

        FileListing() = default;
        ~FileListing() { deallocate(); }

        /// Details

        void printDetails() const;
        void printFileList() const;

        /// Functions

        MU ND int saveToFolder(stringRef_t folderIn = "") const;
        MU void convertRegions(CONSOLE consoleOut);
        void ensureAllRegionFilesExist();

        /// Modify State

        void deallocate();
        void removeFileTypes(const std::set<FileType>& typesToRemove);
        MU void addFiles(File_vec filesIn);
        File_vec collectFiles(FileType fileType);

        /// Read / Write from console files

        MU ND int read(stringRef_t inFileStr);
        MU ND int write(stringRef_t outfileStr, CONSOLE consoleOut);


        /// Conversion

        MU ND int convertTo(stringRef_t inFileStr, stringRef_t outFileStr, CONSOLE consoleOut);
        MU ND int convertAndReplaceRegions(stringRef_t inFileStr, stringRef_t inFileRegionReplacementStr,
                                           stringRef_t outFileStr, CONSOLE consoleOut);

        MU ND int readExternalRegions(stringRef_t inFilePath);
        MU ND int writeExternalRegions(stringRef_t outFilePath);

        MU void pruneRegions();

    private:

        /// Reader

        MU ND int readFile(stringRef_t inFilePath);
        MU ND int readFileInfo(stringRef_t inFilePath);
        MU ND int readWiiU(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readNSXorPS4(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readVita(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readPs3(FILE* f_in, Data& data, u64 source_binary_size, u32 file_size);
        MU ND int readRpcs3(FILE* f_in, Data& data, u64 source_binary_size);
        MU ND int readXbox360DAT(FILE* f_in, Data& data, u32 file_size, u32 src_size);
        MU ND int readXbox360BIN(FILE* f_in, Data& data, u64 source_binary_size);
        MU   void readData(const Data& dataIn);

        /// Writer

        MU ND int writeFile(stringRef_t outfileStr, CONSOLE consoleOut);
        MU ND int writeFileInfo(stringRef_t outFilePath, CONSOLE consoleOut) const;
        MU ND static int writeWiiU(stringRef_t outfileStr, const Data& dataOut);
        MU ND static int writeVita(stringRef_t outfileStr, const Data& dataOut);
        MU ND static int writeRPCS3(stringRef_t outfileStr, const Data& dataOut);
        MU ND static int writePS3();
        MU ND static int writePs4();
        MU ND static int writeNSX();
        MU ND static int writeXbox360_DAT();
        MU ND static int writeXbox360_BIN();
        Data writeData(CONSOLE consoleOut);


        /// File pointer stuff

        void clearPointers();
        void updatePointers();

        /// For use in removeFileTypes

        std::map<FileType, std::function<void()>> clearActionsDelete = {
                {FileType::STRUCTURE, [this] { structures.removeAll(); }},
                {FileType::MAP, [this] { maps.removeAll(); }},
                {FileType::PLAYER, [this] { players.removeAll(); }},
                {FileType::REGION_NETHER, [this] { region_nether.removeAll(); }},
                {FileType::REGION_OVERWORLD, [this] { region_overworld.removeAll(); }},
                {FileType::REGION_END, [this] { region_end.removeAll(); }},
                {FileType::ENTITY_NETHER, [this] { entity_nether->deleteData(); entity_nether = nullptr; }},
                {FileType::ENTITY_OVERWORLD, [this] { entity_overworld->deleteData(); entity_overworld = nullptr; }},
                {FileType::ENTITY_END, [this] { entity_end->deleteData(); entity_end = nullptr; }},
                {FileType::VILLAGE, [this] { village->deleteData(); village = nullptr; }},
                {FileType::DATA_MAPPING, [this] { largeMapDataMappings->deleteData(); largeMapDataMappings = nullptr; }},
                {FileType::LEVEL, [this] { level->deleteData(); level = nullptr; }},
                {FileType::GRF, [this] { grf->deleteData(); grf = nullptr; }},
        };


        std::map<FileType, std::function<void()>> clearActionsRemove = {
                {FileType::STRUCTURE, [this] { structures.clear(); }},
                {FileType::MAP, [this] { maps.clear(); }},
                {FileType::PLAYER, [this] { players.clear(); }},
                {FileType::REGION_NETHER, [this] { region_nether.clear(); }},
                {FileType::REGION_OVERWORLD, [this] { region_overworld.clear(); }},
                {FileType::REGION_END, [this] { region_end.clear(); }},
                {FileType::ENTITY_NETHER, [this] { entity_nether = nullptr; }},
                {FileType::ENTITY_OVERWORLD, [this] { entity_overworld = nullptr; }},
                {FileType::ENTITY_END, [this] { entity_end = nullptr; }},
                {FileType::VILLAGE, [this] { village = nullptr; }},
                {FileType::DATA_MAPPING, [this] { largeMapDataMappings = nullptr; }},
                {FileType::LEVEL, [this] { level = nullptr; }},
                {FileType::GRF, [this] { grf = nullptr; }},
        };
    };
}
