#include "fileListing.hpp"

#include <filesystem>
#include <iostream>
#include <algorithm>

#include "LegacyEditor/utils/endian.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"


void FileListing::saveToFolder(stringRef_t folderIn) {

    std::string folder = folderIn;
    if (folderIn.empty()) {
        folder = dir_path + "dump_" + consoleToStr(console);
    }

    if (folder.length() < 20) {
        printf("tried to delete short directory, will not risk");
        return;
    }

    namespace fs = std::filesystem;
    fs::path _dir_path{folder};
    if (fs::exists(_dir_path) && fs::is_directory(_dir_path)) {
        for (const auto &entry: fs::directory_iterator(_dir_path)) {
            try {
                fs::remove_all(entry.path());
            } catch (const fs::filesystem_error &e) {
                std::cerr << "Filesystem error: " << e.what() << '\n';
            }
        }
    }

    for (File &file: allFiles) {
        std::string fullPath = folderIn + "\\" + file.constructFileName(console);
        fs::path path(fullPath);

        if (!fs::exists(path.parent_path())) {
            fs::create_directories(path.parent_path());
        }

        DataManager fileOut(file.data);
        fileOut.writeToFile(fullPath);
    }
}


// TODO: this function probably doesn't work
File_vec FileListing::collectFiles(FileType fileType) {
    File_vec collectedFiles;
    allFiles.erase(
            std::remove_if(
                    allFiles.begin(),
                    allFiles.end(),
                    [&collectedFiles, &fileType](const File& file) {
                        bool isType = file.fileType == fileType;
                        if (isType) {
                            collectedFiles.push_back(file);
                        }
                        return isType;
                    }
                    ),
            allFiles.end()
    );
    clearActionsRemove[fileType]();
    return collectedFiles;
}


void FileListing::deallocate() {
    for (File& file : allFiles) {
        delete[] file.data.data;
        file.data.data = nullptr;
    }
    clearPointers();
    allFiles.clear();
    oldestVersion = 0;
    currentVersion = 0;

}


void FileListing::clearPointers() {
    region_overworld.clear();
    region_nether.clear();
    region_end.clear();
    entity_overworld = nullptr;
    entity_nether = nullptr;
    entity_end = nullptr;
    maps.clear();
    structures.clear();
    players.clear();
    largeMapDataMappings = nullptr;
    level = nullptr;
    grf = nullptr;
    village = nullptr;
}


void FileListing::updatePointers() {
    clearPointers();
    for (File& file : allFiles) {
        switch(file.fileType) {
            case FileType::STRUCTURE:
                structures.push_back(&file);
                break;
            case FileType::VILLAGE:
                village = &file;
                break;
            case FileType::DATA_MAPPING:
                largeMapDataMappings = &file;
                break;
            case FileType::MAP:
                maps.push_back(&file);
                break;
            case FileType::REGION_NETHER:
                region_nether.push_back(&file);
                break;
            case FileType::REGION_OVERWORLD:
                region_overworld.push_back(&file);
                break;
            case FileType::REGION_END:
                region_end.push_back(&file);
                break;
            case FileType::PLAYER:
                players.push_back(&file);
                break;
            case FileType::LEVEL:
                level = &file;
                break;
            case FileType::GRF:
                grf = &file;
                break;
            case FileType::ENTITY_NETHER:
                entity_nether = &file;
                break;
            case FileType::ENTITY_OVERWORLD:
                entity_overworld = &file;
                break;
            case FileType::ENTITY_END:
                entity_end = &file;
                break;
            default:
                break;
        }
    }
}


void FileListing::removeFileTypes(const std::set<FileType>& typesToRemove) {

    size_t count = allFiles.size();
    for (int index = 0; index < count;) {
        if (typesToRemove.contains(allFiles[index].fileType)) {
            allFiles[index].deleteData();
            allFiles[index].deleteNBTCompound();
            std::swap(allFiles[index], allFiles.back());
            allFiles.pop_back();
            count--;
        } else {
            index++;
        }
    }

    for (const auto& fileType : typesToRemove) {
        clearActionsRemove[fileType]();
    }

    updatePointers();
}


MU void FileListing::addFiles(std::vector<File> filesIn) {
    allFiles.insert(allFiles.end(), filesIn.begin(), filesIn.end());
    updatePointers();
}


MU void FileListing::convertRegions(CONSOLE consoleOut) {
    for (FileList* fileList : dimFileLists) {
        for (File* file : *fileList) {
            RegionManager region(console);
            region.read(file);
            Data data = region.write(consoleOut);
            file->data.steal(data);
        }
    }
}


MU ND int FileListing::convertTo(stringRef_t inFileStr, stringRef_t outFileStr, CONSOLE consoleOut) {
    int status = readFile(inFileStr);
    if (status != STATUS::SUCCESS) return status;
    
    saveToFolder(dir_path + "dump_" + consoleToStr(console));


    removeFileTypes({FileType::PLAYER, FileType::DATA_MAPPING});


    for (auto* fileList : dimFileLists) {
        for (File* file: *fileList) {
            RegionManager region(this->console);
            region.read(file);
            Data data1 = region.write(consoleOut);
            file->data.steal(data1);
        }
    }
    
    status = writeFile(consoleOut, outFileStr);
    return status;
}


MU ND int FileListing::convertAndReplaceRegions(stringRef_t inFileStr,
                                                  stringRef_t inFileRegionReplacementStr,
                                                  stringRef_t outFileStr, CONSOLE consoleOut) {

    int status = readFile(inFileStr);
    if (status != STATUS::SUCCESS) return status;

    FileListing replace;
    status = replace.readFile(inFileRegionReplacementStr);
    if (status != STATUS::SUCCESS) return status;

    removeFileTypes({FileType::REGION_NETHER,
                     FileType::REGION_OVERWORLD,
                     FileType::REGION_END});

    addFiles(replace.collectFiles(FileType::REGION_NETHER));
    addFiles(replace.collectFiles(FileType::REGION_OVERWORLD));
    addFiles(replace.collectFiles(FileType::REGION_END));

    for (auto* fileList : dimFileLists) {
        for (File* file: *fileList) {
            RegionManager region(replace.console);
            region.read(file);
            Data data1 = region.write(consoleOut);
            file->data.steal(data1);
        }
    }
    
    replace.deallocate();

    status = writeFile(consoleOut, outFileStr);
    return status;
}














