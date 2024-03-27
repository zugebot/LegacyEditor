#pragma once

#include <functional>
#include <map>
#include <set>
#include <list>

#include "LCEFile.hpp"
#include "LegacyEditor/LCE/FileInfo/FileInfo.hpp"
#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


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

        std::string filename;
        std::list<LCEFile> allFiles;
        i32 oldestVersion{};
        i32 currentVersion{};
        CONSOLE console = CONSOLE::NONE;
        FileInfo fileInfo;
        bool hasLoadedFileInfo = false;
        bool hasSeparateEntities = false;
        bool hasSeparateRegions = false;

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

        MU ND int saveToFolder(stringRef_t folderIn = "") const;
        MU void convertRegions(CONSOLE consoleOut);
        void ensureAllRegionFilesExist();

        /// Modify State

        void deallocate();
        void removeFileTypes(const std::set<LCEFileType>& typesToRemove);
        MU void addFiles(LCEFile_vec filesIn);
        LCEFile_vec collectFiles(LCEFileType fileType);

        /// Read / Write from console files

        MU ND int read(stringRef_t inFileStr, bool readEXTFile = false);
        MU ND int write(stringRef_t outfileStr, CONSOLE consoleOut);


        /// Conversion

        MU ND int convertTo(stringRef_t inFileStr, stringRef_t outFileStr, CONSOLE consoleOut);
        MU ND int convertAndReplaceRegions(stringRef_t inFileStr, stringRef_t inFileRegionReplacementStr,
                                           stringRef_t outFileStr, CONSOLE consoleOut);

        MU ND int readExternalRegions(stringRef_t inFilePath);
        MU ND static int writeExternalRegions(stringRef_t outFilePath);

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
        MU ND static int writeWiiU(FILE* f_out, const Data& dataOut);
        MU ND static int writeVita(FILE* f_out, const Data& dataOut);
        MU ND static int writeRPCS3(FILE* f_out, const Data& dataOut);
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
