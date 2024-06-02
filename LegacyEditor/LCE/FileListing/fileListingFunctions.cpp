#include "fileListing.hpp"

#include <iostream>
#include <algorithm>

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/LCE/Region/RegionManager.hpp"


namespace editor {

    bool FileListing::AUTO_REMOVE_PLAYERS = true;
    bool FileListing::AUTO_REMOVE_DATA_MAPPING = true;


    void FileListing::printDetails() const {
        printf("> FileListing Details:\n");
        printf("1. Filename: %s\n", filename.c_str());
        printf("2. Oldest Version: %d\n", oldestVersion);
        printf("3. Current Version: %d\n", currentVersion);
        printf("4. Total File Count: %llu\n", allFiles.size());
        printf("5. Player File Count: %llu\n\n", players.size());
    }

    void FileListing::printFileList() const {
        int index = 0;
        for (auto iter = allFiles.begin(); iter != allFiles.end(); ++iter) {
            printf("%.2d: %s\n", index, iter->constructFileName(console, hasSeparateRegions).c_str());
            index++;
        }
        printf("\n");
    }


    int FileListing::saveToFolder(const stringRef_t folderIn) const {

        std::string folder = folderIn;
        if (folderIn.empty()) {
            folder = /*dir_path +*/ "dump/" + consoleToStr(console);
        }

        if (folder.length() < 20) {
            printf("tried to delete short directory, will not risk\n");
            return FILE_ERROR;
        }

        if (const fs::path _dir_path{folder}; exists(_dir_path) && is_directory(_dir_path)) {
            for (const auto &entry: fs::directory_iterator(_dir_path)) {
                try {
                    remove_all(entry.path());
                } catch (const fs::filesystem_error &e) {
                    std::cerr << "Filesystem error: " << e.what() << '\n';
                }
            }
        }

        for (const LCEFile &file: allFiles) {
            std::string fullPath = folder + "\\" + file.constructFileName(console, hasSeparateRegions);

            if (fs::path path(fullPath); !exists(path.parent_path())) {
                create_directories(path.parent_path());
            }

            if (DataManager fileOut(file.data); fileOut.writeToFile(fullPath)) {
                return FILE_ERROR;
            }
        }

        return SUCCESS;
    }


    // TODO: this function probably doesn't work
    LCEFile_vec FileListing::collectFiles(LCEFileType fileType) {
        LCEFile_vec collectedFiles;
        std::erase_if(
                allFiles,
                [&collectedFiles, &fileType](const LCEFile& file) {
                    const bool isType = file.fileType == fileType;
                    if (isType) {
                        collectedFiles.push_back(file);
                    }
                    return isType;
                });
        clearActionsRemove[fileType]();
        return collectedFiles;
    }


    void FileListing::ensureAllRegionFilesExist() {
        bool dim[3][2][2] = {false};
        {
            int dimCount = 0;
            for (const auto *fileList : dimFileLists) {
                for (const auto* regionFile: *fileList) {
                    if (regionFile->data.size != 0) {
                        const i16 regionX = regionFile->nbt->getTag("x").toPrim<i16>();
                        const i16 regionZ = regionFile->nbt->getTag("z").toPrim<i16>();
                        dim[dimCount][regionX + 1][regionZ + 1] = true;
                    }
                }
                dimCount++;
            }
        }

        int filesAdded = 0;
        for (size_t dim_i = 0; dim_i < 3; dim_i++) {
            for (size_t xIter = 2; xIter --> 0;) {
                for (size_t zIter = 2; zIter --> 0;) {
                    if (dim[dim_i][xIter][zIter]) {
                        continue;
                    }

                    filesAdded++;
                    // TODO: should not be CONSOLE::NONE
                    allFiles.emplace_back(lce::CONSOLE::NONE, nullptr, 0, 0);
                    LCEFile &file = allFiles.back();

                    file.nbt->setTag("x", createNBT_INT16(static_cast<i16>(xIter - 1)));
                    file.nbt->setTag("z", createNBT_INT16(static_cast<i16>(zIter - 1)));
                    switch (dim_i) {
                        case 0:
                            file.fileType = LCEFileType::REGION_NETHER;
                            region_nether.push_back(&file);
                            break;
                        case 1:
                            file.fileType = LCEFileType::REGION_OVERWORLD;
                            region_overworld.push_back(&file);
                            break;
                        case 2:
                            file.fileType = LCEFileType::REGION_END;
                            region_end.push_back(&file);
                            break;
                        default:;
                    }
                }
            }
        }
    }







    void FileListing::deallocate() {
        for (LCEFile& file : allFiles) {
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
        for (LCEFile& file : allFiles) {
            switch(file.fileType) {
                case LCEFileType::STRUCTURE:
                    structures.push_back(&file);
                    break;
                case LCEFileType::VILLAGE:
                    village = &file;
                    break;
                case LCEFileType::DATA_MAPPING:
                    largeMapDataMappings = &file;
                    break;
                case LCEFileType::MAP:
                    maps.push_back(&file);
                    break;
                case LCEFileType::REGION_NETHER:
                    region_nether.push_back(&file);
                    break;
                case LCEFileType::REGION_OVERWORLD:
                    region_overworld.push_back(&file);
                    break;
                case LCEFileType::REGION_END:
                    region_end.push_back(&file);
                    break;
                case LCEFileType::PLAYER:
                    players.push_back(&file);
                    break;
                case LCEFileType::LEVEL:
                    level = &file;
                    break;
                case LCEFileType::GRF:
                    grf = &file;
                    break;
                case LCEFileType::ENTITY_NETHER:
                    entity_nether = &file;
                    break;
                case LCEFileType::ENTITY_OVERWORLD:
                    entity_overworld = &file;
                    break;
                case LCEFileType::ENTITY_END:
                    entity_end = &file;
                    break;
                default:
                    break;
            }
        }
    }

    // TODO: this does not work at all
    // TODO: this should be popping the current node, not the ending node
    void FileListing::removeFileTypes(const std::set<LCEFileType>& typesToRemove) {

        auto iter = allFiles.begin();
        while (iter != allFiles.end()) {
            if (typesToRemove.contains(iter->fileType)) {
                iter->deleteData();
                iter = allFiles.erase(iter);
            } else {
                ++iter;
            }
        }
        for (const auto& fileType : typesToRemove) {
            clearActionsRemove[fileType]();
        }

        updatePointers();
    }


    MU void FileListing::addFiles(std::vector<LCEFile> filesIn) {
        allFiles.insert(allFiles.end(), filesIn.begin(), filesIn.end());
        updatePointers();
    }


    MU void FileListing::convertRegions(const lce::CONSOLE consoleOut) {
        for (const FileList* fileList : dimFileLists) {
            for (LCEFile* file : *fileList) {
                // don't convert it if it's already the correct console version
                if (file->console == consoleOut) {
                    continue;
                }
                RegionManager region;
                region.read(file);
                const Data data = region.write(consoleOut);
                file->data.steal(data);
            }
        }
    }


    MU ND int FileListing::convertTo(stringRef_t inFileStr, stringRef_t outFileStr, const lce::CONSOLE consoleOut) {
        int status = readFile(inFileStr);
        if (status != SUCCESS) { return status; }

        status = saveToFolder(/*dir_path + */ "dump_" + consoleToStr(console));

        removeFileTypes({LCEFileType::PLAYER, LCEFileType::DATA_MAPPING});

        for (const auto* fileList : dimFileLists) {
            for (LCEFile* file: *fileList) {
                RegionManager region;
                region.read(file);
                const Data data1 = region.write(consoleOut);
                file->data.steal(data1);
            }
        }

        status = writeFile(outFileStr, consoleOut);
        return status;
    }


    MU ND int FileListing::convertAndReplaceRegions(stringRef_t inFileStr,
                                                    stringRef_t inFileRegionReplacementStr,
                                                    stringRef_t outFileStr, const lce::CONSOLE consoleOut) {

        int status = readFile(inFileStr);
        if (status != SUCCESS) { return status; }

        FileListing replace;
        status = replace.readFile(inFileRegionReplacementStr);
        if (status != SUCCESS) { return status; }

        removeFileTypes({LCEFileType::REGION_NETHER,
                         LCEFileType::REGION_OVERWORLD,
                         LCEFileType::REGION_END});

        addFiles(replace.collectFiles(LCEFileType::REGION_NETHER));
        addFiles(replace.collectFiles(LCEFileType::REGION_OVERWORLD));
        addFiles(replace.collectFiles(LCEFileType::REGION_END));

        for (const auto* fileList : dimFileLists) {
            for (LCEFile* file: *fileList) {
                RegionManager region;
                region.read(file);
                const Data data1 = region.write(consoleOut);
                file->data.steal(data1);
            }
        }

        replace.deallocate();

        status = writeFile(outFileStr, consoleOut);
        return status;
    }

    void FileListing::pruneRegions() {
        auto iter = allFiles.begin();
        while (iter != allFiles.end()) {
            if (!iter->isRegionType()) {
                ++iter;
                continue;
            }

            const i16 regionX = iter->nbt->getTag("x").toPrim<i16>();
            const i16 regionZ = iter->nbt->getTag("z").toPrim<i16>();

            if (regionX < -1 || regionX > 0 || regionZ < -1 || regionZ > 0) {
                iter->deleteData();
                iter = allFiles.erase(iter);
            } else {
                ++iter;
            }
        }
        clearPointers();
        updatePointers();
    }



}
