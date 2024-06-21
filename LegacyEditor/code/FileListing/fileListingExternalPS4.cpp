#include "fileListing.hpp"

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/utils/dataManager.hpp"


std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


static std::wstring stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}


namespace editor {


    std::vector<fs::path> FileListing::findExternalRegionFolders() {
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


    /**
     * \brief
     * \param inDirPath the directory containing the GAMEDATA files.
     * \return
     */
    int FileListing::readExternalRegions(const fs::path& inDirPath) {
        int fileIndex = -1;
        for (c_auto& file : fs::directory_iterator(inDirPath)) {

            // TODO: place non-used files in a cache?
            if (is_directory(file)) { continue; }
            fileIndex++;

            // initiate filename and filepath
            std::string filePathStr = file.path().string();
            std::string fileNameStr = file.path().filename().string();

            // open the file
            DataManager manager_in;
            manager_in.setLittleEndian();
            manager_in.readFromFile(filePathStr);
            c_u32 fileSize = manager_in.readInt32();

            Data dat_out(fileSize);
            const DataManager manager_out(dat_out);
            RLE_NSX_OR_PS4_DECOMPRESS(manager_in.ptr, manager_in.size - 4,
                                      manager_out.ptr, manager_out.size);

            // manager_out.writeToFile("C:\\Users\\Jerrin\\CLionProjects\\LegacyEditor\\out\\" + a_filename);

            // TODO: get timestamp from file itself
            u32 timestamp = 0;
            // TODO: should not be CONSOLE::NONE
            myAllFiles.emplace_back(myConsole, dat_out.data, fileSize, timestamp);
            LCEFile &lFile = myAllFiles.back();
            if (c_auto dimChar = static_cast<char>(static_cast<int>(fileNameStr.at(12)) - 48);
                dimChar < 0 || dimChar > 2) {
                lFile.fileType = LCEFileType::NONE;
            } else {
                static constexpr LCEFileType regDims[3] = {
                        LCEFileType::REGION_OVERWORLD,
                        LCEFileType::REGION_NETHER,
                        LCEFileType::REGION_END
                };
                lFile.fileType = regDims[dimChar];
            }
            c_i16 rX = static_cast<i8>(strtol(
                    fileNameStr.substr(13, 2).c_str(), nullptr, 16));
            c_i16 rZ = static_cast<i8>(strtol(
                    fileNameStr.substr(15, 2).c_str(), nullptr, 16));
            lFile.nbt->setTag("x", createNBT_INT16(rX));
            lFile.nbt->setTag("z", createNBT_INT16(rZ));
        }

        updatePointers();

        return SUCCESS;
    }








    int FileListing::writeExternalRegions(MU c_string_ref outFilePath) {
        printf("FileListing::writeExternalRegions: not implemented!");
        return NOT_IMPLEMENTED;
    }



}



