#include "fileListing.hpp"

#include <iostream>
#include <algorithm>

#include "include/ghc/fs_std.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/code/Region/RegionManager.hpp"


namespace editor {

    bool FileListing::AUTO_REMOVE_PLAYERS = true;
    bool FileListing::AUTO_REMOVE_DATA_MAPPING = true;


    void FileListing::printDetails() const {
        printf("> FileListing Details:\n");
        printf("1. Filename: %s\n", myFilePath.string().c_str());
        printf("2. Oldest Version: %d\n", myOldestVersion);
        printf("3. Current Version: %d\n", myCurrentVersion);
        printf("4. Total File Count: %zu\n", myAllFiles.size());
        printf("5. Player File Count: %zu\n\n", players.size());
    }

    void FileListing::printFileList() const {
        int index = 0;
        for (c_auto& myAllFile : myAllFiles) {
            printf("%.2d [%7d]: %s\n", index, myAllFile.data.size,
                   myAllFile.constructFileName(myConsole, myHasSeparateRegions).c_str());
            index++;
        }
        printf("\n");
    }


    /**
     * Pass in the path that you want "dump/CONSOLE" to be made in.
     * @param inDirPath
     * @return
     */
    int FileListing::dumpToFolder(const fs::path& inDirPath) const {
        const fs::path consoleDirPath = inDirPath / ("dump/" + consoleToStr(myConsole));

        // deletes all files in "DIR/dump/CONSOLE/".
        if (exists(consoleDirPath) && is_directory(consoleDirPath)) {
            for (c_auto &entry: fs::directory_iterator(consoleDirPath)) {
                try {
                    remove_all(entry.path());
                } catch (const fs::filesystem_error &e) {
                    std::cerr << "Filesystem error: " << e.what() << '\n';
                }
            }
        }

        // puts each file in "DIR/dump/CONSOLE/".
        for (const LCEFile &file: myAllFiles) {
            const fs::path fullFilePath = consoleDirPath
                / file.constructFileName(myConsole, myHasSeparateRegions);

            // makes folders (such as "data") in "DIR/dump/CONSOLE/" if they do not exist
            if (!exists(fullFilePath.parent_path())) {
                create_directories(fullFilePath.parent_path());
            }

            // writes the file to "DIR/dump/CONSOLE/FILENAME".
            DataManager fileOut(file.data);
            if (fileOut.writeToFile(fullFilePath)) {
                return FILE_ERROR;
            }
        }

        return SUCCESS;
    }


    // TODO: this function probably doesn't work
    LCEFile_vec FileListing::collectFiles(LCEFileType fileType) {
        LCEFile_vec collectedFiles;
        std::erase_if(
                myAllFiles,
                [&collectedFiles, &fileType](const LCEFile& file) {
                    c_bool isType = file.fileType == fileType;
                    if (isType) {
                        collectedFiles.push_back(file);
                    }
                    return isType;
                });
        clearActionsRemove[fileType]();
        return collectedFiles;
    }


    void FileListing::deallocate() {
        for (LCEFile& file : myAllFiles) {
            delete[] file.data.data;
            file.data.data = nullptr;
        }
        clearPointers();
        myAllFiles.clear();
        myOldestVersion = 0;
        myCurrentVersion = 0;

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
        for (LCEFile& file : myAllFiles) {
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
        auto iter = myAllFiles.begin();
        while (iter != myAllFiles.end()) {
            if (typesToRemove.contains(iter->fileType)) {
                iter->deleteData();
                iter = myAllFiles.erase(iter);
            } else {
                ++iter;
            }
        }
        for (c_auto& fileType : typesToRemove) {
            clearActionsRemove[fileType]();
        }

        updatePointers();
    }


    MU void FileListing::addFiles(std::vector<LCEFile> filesIn) {
        myAllFiles.insert(myAllFiles.end(), filesIn.begin(), filesIn.end());
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


    MU ND int FileListing::convertTo(const fs::path& inFilePath,
                                     const fs::path& outFilePath,
                                     lce::CONSOLE consoleOut) {
        int status = readFile(inFilePath);
        if (status != SUCCESS) { return status; }

        status = dumpToFolder("");

        removeFileTypes({LCEFileType::PLAYER, LCEFileType::DATA_MAPPING});

        for (c_auto* fileList : dimFileLists) {
            for (LCEFile* file: *fileList) {
                RegionManager region;
                region.read(file);
                const Data data1 = region.write(consoleOut);
                file->data.steal(data1);
            }
        }

        status = writeFile(outFilePath, consoleOut);
        return status;
    }


    MU ND int FileListing::convertAndReplaceRegions(const fs::path& inFilePath,
                                                    const fs::path& inFileRegionReplacementPath,
                                                    const fs::path& outFilePath, const lce::CONSOLE consoleOut) {

        int status = readFile(inFilePath);
        if (status != SUCCESS) { return status; }

        FileListing replace;
        status = replace.readFile(inFileRegionReplacementPath);
        if (status != SUCCESS) { return status; }

        removeFileTypes({LCEFileType::REGION_NETHER,
                         LCEFileType::REGION_OVERWORLD,
                         LCEFileType::REGION_END});

        addFiles(replace.collectFiles(LCEFileType::REGION_NETHER));
        addFiles(replace.collectFiles(LCEFileType::REGION_OVERWORLD));
        addFiles(replace.collectFiles(LCEFileType::REGION_END));

        for (c_auto* fileList : dimFileLists) {
            for (LCEFile* file: *fileList) {
                RegionManager region;
                region.read(file);
                const Data data1 = region.write(consoleOut);
                file->data.steal(data1);
            }
        }

        replace.deallocate();

        status = writeFile(outFilePath, consoleOut);
        return status;
    }


    void FileListing::pruneRegions() {
        for (auto iter = myAllFiles.begin(); iter != myAllFiles.end(); ) {
            if (iter->isRegionType()) {
                c_i16 regionX = iter->getRegionX();
                c_i16 regionZ = iter->getRegionZ();
                if (!(regionX == 0 || regionX == -1) || !(regionZ == 0 || regionZ == -1)) {
                    iter->deleteData();
                    iter = myAllFiles.erase(iter);
                    continue;
                }
            }
            ++iter;
        }
        clearPointers();
        updatePointers();
    }


    MU void FileListing::replaceRegionOW(size_t regionIndex, editor::RegionManager& region, const lce::CONSOLE consoleOut) {
        if (regionIndex >= region_overworld.size()) {
            throw std::runtime_error(
                "attempted to call FileListing::replaceRegionOW with an index that is out of bounds.");
        }
        region_overworld[regionIndex]->data.deallocate();
        region_overworld[regionIndex]->data = region.write(consoleOut);
    }



}