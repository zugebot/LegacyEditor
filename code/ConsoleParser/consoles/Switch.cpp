#include "Switch.hpp"

#include "lce/processor.hpp"
#include "tinf/tinf.h"
#include "common/utils.hpp"

#include "code/FileListing/fileListing.hpp"
#include "code/SaveProject/SaveProject.hpp"


namespace editor {


    int Switch::inflateFromLayout(const fs::path& theFilePath, SaveProject* saveProject) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);
        readExternalFiles(saveProject);

        return SUCCESS;
    }


    int Switch::deflateToSave(MU SaveProject* saveProject, MU WriteSettings& theSettings) const {
        printf("Switch.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Switch::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }



    int Switch::readExternalFiles(SaveProject* saveProject) {
        fs::path folder = m_filePath;
        folder.replace_extension(".sub");

        int status;
        if (!fs::is_directory(folder)) {
            return printf_err(FILE_ERROR, "Failed to read associated external files.\n");
        } else if (!folder.empty()) {
            status = readExternalFolder(folder, saveProject);
            saveProject->m_stateSettings.setNewGen(true);
        } else {
            return printf_err(FILE_ERROR, "Failed to find associated external files.\n");
        }
        return status;
    }


    MU int Switch::writeExternalFolder(MU const fs::path& outDirPath) {
        printf("FileListing::writeExternalFolder: not implemented!");
        return NOT_IMPLEMENTED;
    }


}
