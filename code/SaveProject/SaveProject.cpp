#include "SaveProject.hpp"

namespace editor {


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
            int status2 = it->inflateFromLayout(m_stateSettings.filePath(), this);
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
            int status = it->deflateToSave(this, theWriteSettings);
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
        printf("2. Oldest  Version: %d\n", m_fileListing.oldestVersion());
        printf("3. Current Version: %d\n", m_fileListing.currentVersion());
        printf("4. Total  File Count: %zu\n", m_fileListing.size());
        printf("5. Player File Count: %zu\n", m_fileListing.countFiles(lce::FILETYPE::PLAYER));
        printFileList();
    }


    MU void SaveProject::printFileList() const {
        printf("\n** Files Contained **\n");
        int index = 0;
        for (c_auto& myAllFile : m_fileListing) {
            printf("%.2d [%7d]: %s\n", index, myAllFile.m_data.size(),
                   myAllFile.constructFileName(m_stateSettings.console()).c_str());
            index++;
        }
        printf("\n");
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
        for (const LCEFile &file: m_fileListing) {
            const fs::path fullFilePath = dumpPath / file.constructFileName(m_stateSettings.console());

            // ensure dump folder exists
            if (!exists(fullFilePath.parent_path())) {
                create_directories(fullFilePath.parent_path());
            }

            // write files
            try {
                DataWriter::writeFile(fullFilePath, file.m_data.span());
            } catch (const std::exception& e) {
                return FILE_ERROR;
            }
        }

        return SUCCESS;
    }


} // namespace editor