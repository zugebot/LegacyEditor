#include "SaveProject.hpp"

#include "code/SaveFile/writeSettings.hpp"


namespace editor {

    MU std::list<LCEFile> SaveProject::collectFiles(lce::FILETYPE fileType) {
        std::list<LCEFile> collectedFiles;

        for (auto it = m_allFiles.begin(); it != m_allFiles.end(); ) {
            if (it->m_fileType == fileType) {
                collectedFiles.splice(collectedFiles.end(), m_allFiles, it++);
            } else {
                ++it;
            }
        }

        return collectedFiles;
    }


    MU std::list<LCEFile> SaveProject::collectFiles(const std::set<lce::FILETYPE>& typesToCollect) {
        std::list<LCEFile> collectedFiles;

        for (auto it = m_allFiles.begin(); it != m_allFiles.end(); ) {
            if (typesToCollect.contains(it->m_fileType)) {
                collectedFiles.splice(collectedFiles.end(), m_allFiles, it++);
            } else {
                ++it;
            }
        }

        return collectedFiles;
    }


    void SaveProject::removeFileTypes(const std::set<lce::FILETYPE>& typesToRemove) {
        auto iter = m_allFiles.begin();
        while (iter != m_allFiles.end()) {
            if (typesToRemove.contains(iter->m_fileType)) {
                iter = m_allFiles.erase(iter);
            } else {
                ++iter;
            }
        }
    }


    MU void SaveProject::addFiles(std::list<LCEFile>&& filesIn) {
        m_allFiles.splice(m_allFiles.end(), filesIn);
    }




    int SaveProject::read(const fs::path& theFilePath) {
        m_stateSettings.setFilePath(theFilePath);

        i32 status1 = detectConsole(theFilePath, m_stateSettings);
        if (status1 != SUCCESS) {
            printf("Failed to find console from %s\n", theFilePath.string().c_str());
            return status1;
        }

        // read save file
        auto it = makeParserForConsole(m_stateSettings.console(), m_stateSettings.isXbox360Bin());
        if (it != nullptr) {
            int status2 = it->inflateFromLayout(*this, m_stateSettings.filePath());
            if (status2 != SUCCESS) {
                printf("Failed to read save from %s\n", theFilePath.string().c_str());
            }
            return status2;
        } else {
            printf("Failed to read save from %s\n", theFilePath.string().c_str());
            return STATUS::INVALID_CONSOLE;
        }
    }


    int SaveProject::write(WriteSettings& theWriteSettings) {
        if (!theWriteSettings.areSettingsValid()) {
            printf("Write Settings are not valid, exiting\n");
            return STATUS::INVALID_ARGUMENT;
        }

        auto it = makeParserForConsole(theWriteSettings.getConsole());
        if (it != nullptr) {
            int status = it->deflateToSave(*this, theWriteSettings);
            if (status != 0) {
                printf("failed to write save %s.\n", theWriteSettings.getInFolderPath().string().c_str());
            }
            m_stateSettings.setConsole(theWriteSettings.getConsole());
            return status;
        } else {
            printf("failed to write gamedata to %s", theWriteSettings.getInFolderPath().string().c_str());
            return STATUS::INVALID_CONSOLE;
        }
    }




    MU void SaveProject::printDetails() const {
        printf("\n** FileListing Details **\n");
        printf("1. Filename: %s\n", m_stateSettings.filePath().string().c_str());
        printf("2. Oldest  Version: %d\n", oldestVersion());
        printf("3. Current Version: %d\n", currentVersion());
        printf("4. Total  File Count: %zu\n", m_allFiles.size());
        printf("5. Player File Count: %zu\n", countFiles(lce::FILETYPE::PLAYER));
        printf("6. Map    File Count: %zu\n", countFiles(lce::FILETYPE::MAP));
        printFileList();
    }


    MU void SaveProject::printFileList() const {
        printf("\n** Files Contained **\n");
        int index = 0;
        for (c_auto& myAllFile : m_allFiles) {
            printf("%.2d [%7llu]: %s\n", index, myAllFile.detectSize(),
                   myAllFile.constructFileName(m_stateSettings.console()).c_str());
            index++;
        }
        printf("\n");
        std::cout << std::flush;
    }


    MU int SaveProject::dumpToFolder(const std::string& detail = "") const {

        // get valid dump path
        fs::path dumpPath;
        {
            std::string folderName = (getCurrentDateTimeString() + "_" +
                                      consoleToStr(m_stateSettings.console())) + "_" +
                                      detail;
            int attempt = 0;
            do {
                std::string tempFolderName = folderName;
                tempFolderName += "_" + std::to_string(attempt);
                attempt++;
                dumpPath = fs::current_path() / "dump" / tempFolderName;

            } while (exists(dumpPath));
        }



        std::cout << "Dumping to folder: " << dumpPath << "\n";

        // puts each file in "DIR/dump/CONSOLE/".
        for (const LCEFile &file : m_allFiles) {
            const fs::path fullFilePath = dumpPath / file.constructFileName(m_stateSettings.console());

            // ensure dump folder exists
            if (!exists(fullFilePath.parent_path())) {
                create_directories(fullFilePath.parent_path());
            }

            fs::copy(file.path(), fullFilePath);
        }

        return SUCCESS;
    }


} // namespace editor