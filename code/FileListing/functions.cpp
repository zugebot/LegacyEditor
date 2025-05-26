#include "fileListing.hpp"

#include "include/ghc/fs_std.hpp"


#include "code/ConsoleParser/helpers/detectConsole.hpp"
#include "code/Region/RegionManager.hpp"
#include "common/nbt.hpp"


namespace editor {


    void FileListing::deallocate() {
        m_allFiles.clear();
    }


    MU std::list<LCEFile> FileListing::collectFiles(lce::FILETYPE fileType) {
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


    MU std::list<LCEFile> FileListing::collectFiles(const std::set<lce::FILETYPE>& typesToCollect) {
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


    void FileListing::removeFileTypes(const std::set<lce::FILETYPE>& typesToRemove) {
        auto iter = m_allFiles.begin();
        while (iter != m_allFiles.end()) {
            if (typesToRemove.contains(iter->m_fileType)) {
                iter->clear();
                iter = m_allFiles.erase(iter);
            } else {
                ++iter;
            }
        }
    }


    MU void FileListing::addFiles(std::list<LCEFile>&& filesIn) {
        m_allFiles.splice(m_allFiles.end(), filesIn);
    }


    MU void FileListing::convertRegions(lce::CONSOLE consoleOut) {
        static const std::set<lce::FILETYPE> regionTypes = {
                lce::FILETYPE::OLD_REGION_NETHER,
                lce::FILETYPE::OLD_REGION_OVERWORLD,
                lce::FILETYPE::OLD_REGION_END
        };

        for (LCEFile& file : view_of(regionTypes)) {
            // if (file.m_console == consoleOut) continue;

            RegionManager region;
            region.read(&file);
            region.convertChunks(consoleOut);

            file.m_data = region.write(consoleOut);
            file.m_console = consoleOut;
        }
    }


    void FileListing::pruneRegions() {
        for (auto iter = m_allFiles.begin(); iter != m_allFiles.end(); ) {
            if (iter->isRegionType()) {
                c_i16 regionX = iter->getRegionX();
                c_i16 regionZ = iter->getRegionZ();
                if (!(regionX == 0 || regionX == -1) || !(regionZ == 0 || regionZ == -1)) {
                    iter->clear();
                    iter = m_allFiles.erase(iter);
                    continue;
                }
            }
            ++iter;
        }
    }



}