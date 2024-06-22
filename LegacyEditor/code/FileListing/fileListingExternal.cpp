#include "fileListing.hpp"

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/utils/dataManager.hpp"



namespace editor {


    /**
     * \brief
     * \param inDirPath the directory containing the GAMEDATA files.
     * \return
     */
    int FileListing::readExternalRegionsPS4_OR_NSX(const fs::path& inDirPath) {
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



}



