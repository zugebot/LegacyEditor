#include "SaveProject.hpp"

#include "code/SaveFile/writeSettings.hpp"
#include "common/utils.hpp"


i32 safe_remove_all(const fs::path& base,
                               const fs::path& target)
{
    std::error_code ec;

    fs::path canonBase   = fs::canonical(base,   ec);
    if (ec) throw std::system_error(ec, "canonical(base)");

    fs::path canonTarget = fs::canonical(target, ec);
    if (ec) throw std::system_error(ec, "canonical(target)");

    /* ---------- containment test (component-wise) ---------------- */

    auto  bi = canonBase.begin(),  be = canonBase.end();
    auto  ti = canonTarget.begin(), te = canonTarget.end();

    for ( ; bi != be && ti != te && *bi == *ti; ++bi, ++ti) /*noop*/ ;

    const bool isInside = (bi == be) && (ti != te);   // prefix match but longer
    if (!isInside)
        throw std::runtime_error("target escapes base directory");

    /* ---------- additional safety rails -------------------------- */

    if (canonTarget == canonTarget.root_path())
        throw std::runtime_error("refuse to delete file-system root");

    if (canonTarget == canonBase)
        throw std::runtime_error("refuse to delete the base directory itself");

    if (fs::is_symlink(target))
        throw std::runtime_error("refuse to follow/delete a symlink");

    /* ---------- finally try to delete ---------------------------- */

    i32 nRemoved = static_cast<i32>(fs::remove_all(canonTarget, ec));
    if (ec) throw std::system_error(ec, "remove_all");

    return nRemoved;
}


namespace editor {


    const std::set<lce::FILETYPE> SaveProject::s_OLD_REGION_ANY = {
            lce::FILETYPE::OLD_REGION_NETHER,
            lce::FILETYPE::OLD_REGION_OVERWORLD,
            lce::FILETYPE::OLD_REGION_END
    };

    const std::set<lce::FILETYPE> SaveProject::s_NEW_REGION_ANY = {
            lce::FILETYPE::NEW_REGION_NETHER,
            lce::FILETYPE::NEW_REGION_OVERWORLD,
            lce::FILETYPE::NEW_REGION_END
    };

    const std::set<lce::FILETYPE> SaveProject::s_ENTITY_ANY = {
            lce::FILETYPE::ENTITY_NETHER,
            lce::FILETYPE::ENTITY_OVERWORLD,
            lce::FILETYPE::ENTITY_END
    };

    fs::path SaveProject::s_TEMP_FOLDER_BASE;


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

        fs::path safe_base   = s_TEMP_FOLDER_BASE;
        m_tempFolder = safe_base / getCurrentDateTimeString();
        if (!m_tempFolder.empty() && !fs::exists(m_tempFolder)) {
            fs::create_directories(m_tempFolder);
        }

        i32 status1 = ::editor::detectConsole(theFilePath, m_stateSettings);
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

        int ret = SUCCESS;
        auto it = makeParserForConsole(theWriteSettings.m_schematic.save_console);
        if (it != nullptr) {
            int status = it->deflateToSave(*this, theWriteSettings);
            if (status != 0) {
                printf("failed to write save %s.\n", theWriteSettings.getInFolderPath().string().c_str());
            }
            m_stateSettings.setConsole(theWriteSettings.m_schematic.save_console);
            ret = status;
        } else {
            printf("failed to write gamedata to %s", theWriteSettings.getInFolderPath().string().c_str());
            ret = STATUS::INVALID_CONSOLE;
        }

        fs::path tempFolderPath = fs::path("temp") / getCurrentDateTimeString();
        for (auto& file : m_allFiles) {
            file.setTempFolderPath(tempFolderPath);
        }

        return ret;
    }


    i32 SaveProject::cleanup() const {

        i32 filesDeleted = safe_remove_all(s_TEMP_FOLDER_BASE, m_tempFolder);

        return filesDeleted;
    }


    MU void SaveProject::printDetails() const {
        printf("\n** Savefile Details **\n");
        printf("1. Filename: %s\n", m_stateSettings.filePath().string().c_str());
        printf("2. Oldest  Version: %d\n", oldestVersion());
        printf("3. Current Version: %d\n", latestVersion());
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
                   myAllFile.constructFileName().c_str());
            index++;
        }
        printf("\n");
        std::cout << std::flush;
    }


    lce::CONSOLE SaveProject::detectConsole(const fs::path& savePath) {
        StateSettings settings;
        i32 status1 = ::editor::detectConsole(savePath, settings);
        if (status1 != SUCCESS) {
            return lce::CONSOLE::NONE;
        }

        return settings.console();
    }


    MU int SaveProject::dumpToFolder(const std::string& detail = "") const {

        // get valid dump path
        fs::path dumpPath;
        {
            std::string folderName = (getCurrentDateTimeString() + "_" +
                                      consoleToStr(m_stateSettings.console())) +
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
            fs::path fullFilePath = dumpPath / file.constructFileName();
            fullFilePath.make_preferred();

            // ensure dump folder exists
            if (!exists(fullFilePath.parent_path())) {
                create_directories(fullFilePath.parent_path());
            }

            fs::copy(file.path(), fullFilePath);
        }

        return SUCCESS;
    }


} // namespace editor