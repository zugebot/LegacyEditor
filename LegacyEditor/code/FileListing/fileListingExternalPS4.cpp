#include "fileListing.hpp"

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/utils/dataManager.hpp"
#include "LegacyEditor/utils/utils.hpp"


namespace editor {


    std::vector<fs::path> FileListing::findExternalRegionPS4Folders() {
        // go from "root/00000001/savedata0/GAMEDATA" to "root/00000001/savedata0"
        const fs::path mainDirPath = myFilePath.parent_path();


        // get sfo data from "root/00000001/savedata0/sce_sys/param.sfo"
        const fs::path sfoFilePath = mainDirPath / "sce_sys" / "param.sfo";
        if (!fs::exists(sfoFilePath)) {
            printf("input folder does not have a sce_sys/param.sfo, returning early");
            return {""};
        }

        // get the "CUSA00744-240620222358.0"-alike str from the main "param.sfo"
        SFOManager mainSFO(sfoFilePath.string());
        const std::wstring subtitle = stringToWstring(mainSFO.getAttribute("SUBTITLE"));
        fileInfo.basesavename = subtitle;


        std::string mainAttr = mainSFO.getAttribute("SAVEDATA_DIRECTORY");
        auto mainAttrParts = split(mainAttr, '.');

        if (mainAttrParts.size() != 2) {
            printf("main param.sfo does not seem to be formatted correctly.");
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
                printf("to check param.sfo does not seem to be formatted correctly.");
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


    int FileListing::writeExternalRegionsPS4(MU const fs::path& outDirPath) {
        printf("FileListing::writeExternalRegionsPS4: not implemented!");
        return NOT_IMPLEMENTED;
    }

}



