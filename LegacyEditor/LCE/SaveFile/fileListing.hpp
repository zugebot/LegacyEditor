#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "ConsoleParser.hpp"
#include "LegacyEditor/utils/dataManager.hpp"

#include <functional>
#include <map>
#include <set>
#include <vector>





class FileList : public std::vector<File*> {
public:
    ~FileList() {
        removeAll();
    }

    void removeAll() {
        for (File* file : *this) {
            delete[] file->data.data;
        }
        clear();
    }
};


class FileListing {
private:
    std::vector<FileList*> dimFileLists = {&netherFilePtrs, &overworldFilePtrs, &endFilePtrs};
public:
    CONSOLE console = CONSOLE::NONE;
    std::vector<File> allFiles;

    // pointers
    FileList overworldFilePtrs;
    FileList netherFilePtrs;
    FileList endFilePtrs;

    FileList mapFilePtrs;
    FileList structureFilePtrs;
    FileList playerFilePtrs;

    File* largeMapDataMappingsFilePtr{};
    File* levelFilePtr{};
    File* grfFilePtr{};
    File* villageFilePtr{};

    // data
    i32 oldestVersion{};
    i32 currentVersion{};

    FileListing() = default;
    explicit FileListing(ConsoleParser& consoleParser)
        : console(consoleParser.console) { read(consoleParser); }
    explicit FileListing(ConsoleParser* consoleParser)
        : console(consoleParser->console) { read(*consoleParser); }
    ~FileListing() {
        clear();
    }

    void read(Data& dataIn);
    Data write(CONSOLE consoleOut);
    void saveToFolder(const std::string& folder);

    void convertRegions(CONSOLE consoleOut);

    void deleteAllChunks();


    void clear();

    void clearPointers();
    void updatePointers();




    template <typename... FileTypes>
    void removeFileTypes(FileTypes... fileTypes) {
        std::set<FileType> typesToRemove{fileTypes...};
        allFiles.erase(
                std::remove_if(
                        allFiles.begin(),
                        allFiles.end(),
                        [&typesToRemove](const File& file) {
                            return typesToRemove.count(file.fileType) > 0;
                        }
                        ),
                allFiles.end()
        );
        for (const auto& fileType : typesToRemove) {
            if (clearActionsDelete.count(fileType)) {
                clearActionsDelete[fileType]();
            }
        }
    }


    void addFiles(std::vector<File> filesIn) {
        allFiles.insert(allFiles.end(), filesIn.begin(), filesIn.end());
        updatePointers();
    }



    std::vector<File> collectFiles(FileType fileType);



private:
    /// For use in removeFileTypes
    std::map<FileType, std::function<void()>> clearActions = {
        {FileType::STRUCTURE, [this]() { structureFilePtrs.removeAll(); }},
        {FileType::MAP, [this]() { mapFilePtrs.removeAll(); }},
        {FileType::PLAYER, [this]() { playerFilePtrs.removeAll(); }},
        {FileType::REGION_NETHER, [this]() { netherFilePtrs.removeAll(); }},
        {FileType::REGION_OVERWORLD, [this]() { overworldFilePtrs.removeAll(); }},
        {FileType::REGION_END, [this]() { endFilePtrs.removeAll(); }},
        {FileType::VILLAGE, [this]() {
                 delete[] villageFilePtr->data.data;
                 villageFilePtr = nullptr; }},
        {FileType::DATA_MAPPING, [this]() {
                 delete[] largeMapDataMappingsFilePtr->data.data;
             largeMapDataMappingsFilePtr = nullptr;
         }},
        {FileType::LEVEL, [this]() {
             delete[] levelFilePtr->data.data;
             levelFilePtr = nullptr;
         }},
        {FileType::GRF, [this]() {
             delete[] grfFilePtr->data.data;
             grfFilePtr = nullptr;
         }},
    };


    std::map<FileType, std::function<void()>> clearActionsDelete = {
            {FileType::STRUCTURE, [this]() { structureFilePtrs.clear(); }},
            {FileType::MAP, [this]() { mapFilePtrs.clear(); }},
            {FileType::PLAYER, [this]() { playerFilePtrs.clear(); }},
            {FileType::REGION_NETHER, [this]() { netherFilePtrs.clear(); }},
            {FileType::REGION_OVERWORLD, [this]() { overworldFilePtrs.clear(); }},
            {FileType::REGION_END, [this]() { endFilePtrs.clear(); }},
            {FileType::VILLAGE, [this]() {
                 villageFilePtr = nullptr; }},
            {FileType::DATA_MAPPING, [this]() {
                 largeMapDataMappingsFilePtr = nullptr;
             }},
            {FileType::LEVEL, [this]() {
                 levelFilePtr = nullptr;
             }},
            {FileType::GRF, [this]() {
                 grfFilePtr = nullptr;
             }},
    };
};