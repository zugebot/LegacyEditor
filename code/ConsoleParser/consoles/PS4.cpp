#include "PS4.hpp"

#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"
#include "common/utils.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "code/SaveFile/fileListing.hpp"


namespace editor {


    SaveLayout PS4::discoverSaveLayout(const fs::path& rootFolder) {
        SaveLayout layout;
        
        return layout;
    }


    int PS4::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
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


    int PS4::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {

        printf("PS4.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }

    int PS4::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }


    std::vector<fs::path> PS4::findExternalFolder(SaveProject& saveProject) {
        // go from "root/00000001/savedata0/GAMEDATA" to "root/00000001/savedata0"
        const fs::path mainDirPath = saveProject.m_stateSettings.filePath().parent_path();


        // get sfo data from "root/00000001/savedata0/sce_sys/param.sfo"
        const fs::path sfoFilePath = mainDirPath / "sce_sys" / "param.sfo";
        if (!fs::exists(sfoFilePath)) {
            printf("input folder does not have a sce_sys/param.sfo, returning early");
            return {""};
        }

        const fs::path icon0Path = mainDirPath / "sce_sys" / "icon0.png";
        if (!saveProject.m_displayMetadata.icon0png.isValid() && fs::exists(icon0Path)) {
            saveProject.m_displayMetadata.icon0png.loadFromFile(icon0Path.string().c_str());
        }

        // get the "CUSA00744-240620222358.0"-alike str from the main "param.sfo"
        SFOManager mainSFO(sfoFilePath.string());
        const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUBTITLE"));
        saveProject.m_displayMetadata.worldName = subtitle;


        std::string mainAttr = mainSFO.getAttribute("SAVEDATA_DIRECTORY");
        auto mainAttrParts = split(mainAttr, '.');

        if (mainAttrParts.size() != 2) {
            printf("param.sfo does not seem to be formatted correctly.");
            return {""};
        }

        // go from "root/00000001/savedata0" to "root"
        const fs::path toCheckDirPath = mainDirPath.parent_path().parent_path();
        // the vector of directories to add regions from
        std::vector<fs::path> directoriesToReturn{};
        // checks each folder in "root"
        for (c_auto &entry: fs::directory_iterator(toCheckDirPath)) {
            // skip entries that are not directories
            if (!fs::is_directory(entry.status())) {
                continue;
            }

            // skips checking the input folder
            if (mainDirPath.parent_path() == entry.path()) {
                continue;
            }

            if (entry.path().filename().string().contains("OPTIONS")) {
                continue;
            }

            if (entry.path().filename().string().contains("CACHE")) {
                continue;
            }

            fs::path tempCheckDirPath = entry.path() / "savedata0";
            if (!fs::exists(tempCheckDirPath)) {
                continue;
            }

            fs::path tempSFOFilePath = entry.path() / "savedata0" / "sce_sys" / "param.sfo";
            if (!fs::exists(tempSFOFilePath)) {
                continue;
            }

            // get the "CUSA00744-240620222358.0"-alike str from the temp "param.sfo"
            SFOManager tempSFO(tempSFOFilePath.string());
            std::string tempAttr = tempSFO.getAttribute("SAVEDATA_DIRECTORY");
            auto tempAttrParts = split(tempAttr, '.');

            if (tempAttrParts.size() != 2) {
                continue;
            }

            // skip PS4 worlds that are not the same as the one being looked for
            if (mainAttrParts[0] != tempAttrParts[0]) {
                continue;
            }

            directoriesToReturn.push_back(tempCheckDirPath);
        }

        return directoriesToReturn;
    }


    int PS4::readExternalFolders(SaveProject& saveProject) {
        auto folders = findExternalFolder(saveProject);
        int status;
        for (c_auto& folder : folders) {
            status = readExternalFolder(saveProject, folder);
            if (status != 0) {
                printf("Failed to read associated external files.\n");
                break;
            }
        }
        saveProject.m_stateSettings.setNewGen(true);
        return status;
    }


    int PS4::writeExternalFolders(SaveProject& saveProject, const fs::path& outDirPath) {
        return NOT_IMPLEMENTED;
    }


}