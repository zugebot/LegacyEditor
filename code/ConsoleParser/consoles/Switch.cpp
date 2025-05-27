#include "Switch.hpp"

#include "lce/processor.hpp"
#include "tinf/tinf.h"
#include "common/utils.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int Switch::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);
        readExternalFolders(saveProject);

        return SUCCESS;
    }


    int Switch::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        printf("Switch.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Switch::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }


    std::vector<fs::path> Switch::findExternalFolder(editor::SaveProject &saveProject) {
        fs::path folder = m_filePath;
        folder.replace_extension(".sub");
        if (!fs::is_directory(folder) || folder.empty()) {
            return {};
        }
        return { folder };
    }


    int Switch::readExternalFolders(SaveProject& saveProject) {
        auto folders = findExternalFolder(saveProject);
        if (folders.empty()) {
            return printf_err(FILE_ERROR, "Failed to find associated external files.\n");
        } else {
            // there is only one folder
            int status = readExternalFolder(saveProject, folders[0]);
            saveProject.m_stateSettings.setNewGen(true);
            return status;
        }
    }


    int Switch::writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) {
        printf("FileListing::writeExternalFolder: not implemented!");
        return NOT_IMPLEMENTED;
    }


}
