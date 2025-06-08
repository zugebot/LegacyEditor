#include "Windurango.hpp"

#include "lce/processor.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int Windurango::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        try {
            fs::path wd_displayNamePath = m_filePath.parent_path() / "wd_displayname.txt";
            Buffer displayNameBuffer = DataReader::readFile(wd_displayNamePath);
            std::string displayName;
            displayName.resize(displayNameBuffer.size());
            mempcpy(displayName.data(), displayNameBuffer.data(), displayNameBuffer.size());
            saveProject.m_displayMetadata.worldName = stringToWstring(displayName);
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        readExternalFolders(saveProject);

        return SUCCESS;
    }

    int Windurango::inflateListing(SaveProject& saveProject) {
        return NewGenConsoleParser::inflateListing(saveProject);
    }


    int Windurango::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        printf("Switch.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Windurango::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }


    std::vector<fs::path> Windurango::findExternalFolder(editor::SaveProject &saveProject) {
        fs::path folder = m_filePath.parent_path();
        if (folder.empty()) {
            return {};
        }
        return { folder };
    }


    int Windurango::readExternalFolders(SaveProject& saveProject) {
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


    int Windurango::writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) {
        printf("FileListing::writeExternalFolder: not implemented!");
        return NOT_IMPLEMENTED;
    }


    std::optional<fs::path> Windurango::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "THUMB";
    }
}
