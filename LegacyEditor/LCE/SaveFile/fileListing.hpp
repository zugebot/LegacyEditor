#pragma once

#include <functional>
#include <map>
#include <set>
#include <vector>

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/file.hpp"



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


class ConsoleParser;


class FileListing {
private:
    static constexpr u32 FILE_HEADER_SIZE = 144;

public:

    // data
    std::vector<File> allFiles;
    i32 oldestVersion{};
    i32 currentVersion{};
    CONSOLE console = CONSOLE::NONE;
    bool separateEntities = false;
    bool separateRegions = false;

    // pointers
    FileList* dimFileLists[3] = {&region_nether, &region_overworld, &region_end};
    FileList* entityFileLists[3] = {&entity_nether, &entity_overworld, &entity_end};
    FileList region_nether, region_overworld, region_end;
    FileList entity_nether, entity_overworld, entity_end;
    FileList maps, structures, players;
    File *largeMapDataMappings{}, *level{}, *grf{}, *village{};


    FileListing() = default;
    explicit FileListing(ConsoleParser& consoleParser);
    MU explicit FileListing(ConsoleParser* consoleParser);
    ~FileListing() { deallocate(); }

    void read(Data& dataIn);
    Data write(CONSOLE consoleOut);
    void saveToFolder(const std::string& folder);

    MU void convertRegions(CONSOLE consoleOut);
    MU void deleteAllChunks();


    void deallocate();
    void clearPointers();
    void updatePointers();
    void removeFileTypes(const std::set<FileType>& typesToRemove);
    MU void addFiles(std::vector<File> filesIn);
    std::vector<File> collectFiles(FileType fileType);


private:
    /// For use in removeFileTypes
    std::map<FileType, std::function<void()>> clearActionsDelete = {
        {FileType::STRUCTURE, [this]() { structures.removeAll(); }},
        {FileType::MAP, [this]() { maps.removeAll(); }},
        {FileType::PLAYER, [this]() { players.removeAll(); }},
        {FileType::REGION_NETHER, [this]() { region_nether.removeAll(); }},
        {FileType::REGION_OVERWORLD, [this]() { region_overworld.removeAll(); }},
        {FileType::REGION_END, [this]() { region_end.removeAll(); }},
        {FileType::ENTITY_NETHER, [this]() { entity_nether.removeAll(); }},
        {FileType::ENTITY_OVERWORLD, [this]() { entity_overworld.removeAll(); }},
        {FileType::ENTITY_END, [this]() { entity_end.removeAll(); }},
        {FileType::VILLAGE, [this]() {
                 delete[] village->data.data;
                 village->data.data = nullptr;
                 village = nullptr; }},
        {FileType::DATA_MAPPING, [this]() {
                 delete[] largeMapDataMappings->data.data;
                 largeMapDataMappings->data.data = nullptr;
                 largeMapDataMappings = nullptr;
         }},
        {FileType::LEVEL, [this]() {
             delete[] level->data.data;
             level->data.data = nullptr;
             level = nullptr;
         }},
        {FileType::GRF, [this]() {
             delete[] grf->data.data;
             grf->data.data = nullptr;
             grf = nullptr;
         }},
    };


    std::map<FileType, std::function<void()>> clearActionsRemove = {
            {FileType::STRUCTURE, [this]() { structures.clear(); }},
            {FileType::MAP, [this]() { maps.clear(); }},
            {FileType::PLAYER, [this]() { players.clear(); }},
            {FileType::REGION_NETHER, [this]() { region_nether.clear(); }},
            {FileType::REGION_OVERWORLD, [this]() { region_overworld.clear(); }},
            {FileType::REGION_END, [this]() { region_end.clear(); }},
            {FileType::ENTITY_NETHER, [this]() { entity_nether.clear(); }},
            {FileType::ENTITY_OVERWORLD, [this]() { entity_overworld.clear(); }},
            {FileType::ENTITY_END, [this]() { entity_end.clear(); }},
            {FileType::VILLAGE, [this]() { village = nullptr; }},
            {FileType::DATA_MAPPING, [this]() {
                 largeMapDataMappings = nullptr;
             }},
            {FileType::LEVEL, [this]() {
                 level = nullptr;
             }},
            {FileType::GRF, [this]() {
                 grf = nullptr;
             }},
    };
};