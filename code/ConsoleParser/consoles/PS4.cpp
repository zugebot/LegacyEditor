#include "PS4.hpp"

#include "include/sfo/sfo.hpp"
#include "include/tinf/tinf.h"
#include "common/utils.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "common/fmt.hpp"
#include "zlib-1.2.12/zlib.h"

#include <variant>


namespace editor {


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
        const std::wstring subtitle = stringToWstring(mainSFO.getStringAttribute("SUBTITLE").value_or("New World"));
        saveProject.m_displayMetadata.worldName = subtitle;


        std::optional<std::string> mainAttr = mainSFO.getStringAttribute("SAVEDATA_DIRECTORY");

        // invalid SFO
        if (!mainAttr) {
            return {""};
        }

        auto mainAttrParts = split(mainAttr.value(), '.');

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


            std::optional<std::string> tempAttr = tempSFO.getStringAttribute("SAVEDATA_DIRECTORY");

            // invalid SFO
            if (!tempAttr) {
                return {""};
            }

            auto tempAttrParts = split(tempAttr.value(), '.');

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


    static void placeSceSysFiles(SaveProject& saveProject,
                                 const fs::path& root,
                                 const std::string& productCode,
                                 const std::string& folderN,
                                 WriteSettings& theSettings) {

        // create "sce_sys" folder
        fs::path folder_sce_sys = root / "sce_sys";
        fs::create_directories(folder_sce_sys);

        // create "icon0.png" file
        Buffer buf;
        fs::path icon0png = folder_sce_sys / "icon0.png";
        DataWriter::writeFile(icon0png, buf.span());

        // R"(C:\Users\jerrin\AppData\Roaming\shadPS4\savedata\1\WORKING\CUSA00265-250816190340.0\sce_sys\param.sfo)";
        fs::path otherPath = theSettings.m_paramSfoToReplace;
        SFOManager other(otherPath.string());

        // create "param.sfo" file
        SFOManager sfo;
        fs::path sfoPath = folder_sce_sys / "param.sfo";


        if (saveProject.m_stateSettings.isShadPs4()) {
            sfo.setLexicographic(false);

            std::string data(8, '\0');
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", data);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "MAINTITLE",           "Minecraft: PlayStation®4 Edition");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "SUBTITLE",            "Tutorial"); // wStringToString(saveProject.m_displayMetadata.worldName));
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "DETAIL",              "\0");

            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "SAVEDATA_DIRECTORY",  folderN);
            sfo.addParam(eSFO_FMT::INT,          "SAVEDATA_LIST_PARAM", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "TITLE_ID",            productCode);

            std::string savedata_blocks("\x00\x02\x00\x00\x00\x00\x00\x00", 8);
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "SAVEDATA_BLOCKS",     savedata_blocks);

        } else {
            {
                auto attr = other.getAttribute("ACCOUNT_ID");
                if (!attr.has_value()) throw std::runtime_error("input param.sfo does not have attribute \"ACCOUNT_ID\"");

                auto account_id = attr.value().toString();
                sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", account_id);
            }
            sfo.addParam(eSFO_FMT::INT,          "ATTRIBUTE",           "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "CATEGORY",            "sd");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "DETAIL",              "\0"); // TODO: is this correct?
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "FORMAT",              "obs");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "MAINTITLE",           "Minecraft: PlayStation®4 Edition");
            {
                auto attr = other.getAttribute("PARAMS");
                if (!attr.has_value()) throw std::runtime_error("input param.sfo does not have attribute \"PARAMS\"");

                auto array = std::get<std::vector<uint8_t>>(attr.value().myValue);
                auto reader = DataReader(array.data(), array.size(), Endian::Little);

                std::vector<uint8_t> params(1024);
                DataWriter writer(params.data(), params.size(), Endian::Little);

                int modificationCount = 0;
                u32 creationTime = 0;
                u32 lastModifiedTime = 0;

                // TODO: I think the endianness of this is Big, not Little
                writer.write<u32>(0);
                reader.skip(4);
                writer.writeBytes(reader.ptr(), 8);
                reader.skip(8);
                writer.writeBytes(reader.ptr(), 32);
                reader.skip(32);
                writer.write<u32>(1);
                reader.skip(4);

                writer.writeBytes(reinterpret_cast<c_u8*>(productCode.data()), productCode.size() + 1);
                writer.skip(16 - productCode.size() + 1);
                reader.skip(16);
                writer.write<u32>(0);
                reader.skip(4);
                writer.writeBytes(reinterpret_cast<c_u8*>(productCode.data()), productCode.size() + 1);
                writer.skip(16 - productCode.size() + 1);
                reader.skip(16);

                writer.write<u32>(0);
                reader.skip(4);
                writer.write<u32>(modificationCount);
                reader.skip(4);
                writer.writePad(18, 0);
                reader.skip(18);
                writer.write<u32>(reader.read<u32>());
                writer.write<u32>(creationTime);
                reader.skip(4);
                writer.write<u32>(0);
                reader.skip(4);
                writer.write<u32>(lastModifiedTime);
                reader.skip(4);

                std::string params_str(reinterpret_cast<const char*>(params.data()), params.size());
                sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "PARAMS", params_str);

                std::string savedata_blocks("\x00\x02\x00\x00\x00\x00\x00\x00", 8);
                sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "SAVEDATA_BLOCKS",     savedata_blocks);

                sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "SAVEDATA_DIRECTORY",  folderN);
                sfo.addParam(eSFO_FMT::INT,          "SAVEDATA_LIST_PARAM", "0");
                sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "SUBTITLE",            wStringToString(saveProject.m_displayMetadata.worldName));
                sfo.addParam(eSFO_FMT::UTF8_NORMAL,  "TITLE_ID",            productCode);
            }
        }



        sfo.setMagic(eSFO_MAGIC::PS3_HDD);
        sfo.saveToFile(sfoPath.string());
    }


    int PS4::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {

        // sce_sys/icon0.png
        //      228*128, bit-depth[24]
        // sce_sys/keystone
        // sce_sys/param.sfo
        // sce_sys/sce_icon0png1
        //      116736 bytes of null


        std::string dateTimeStr = getCurrentDateTimeString();
        std::string strPCode = PS4Mapper.toString(
                theSettings.m_productCodes.getPS4());
        auto makePS4Folder = [&](int digit) {
            return strPCode + "-" + dateTimeStr + "." + std::to_string(digit);
        };


        const fs::path root = theSettings.getInFolderPath();

        // folder0
        {
            auto folder0Name = makePS4Folder(0);
            auto folder0 = root / folder0Name;
            fs::create_directories(folder0);

            placeSceSysFiles(saveProject, folder0, strPCode, folder0Name, theSettings);


            // GAMEDATA
            theSettings.m_fileNameOut = "GAMEDATA";
            fs::path gameDataPath = folder0 / theSettings.m_fileNameOut;
            Buffer inflatedData = FileListing::writeListing(saveProject, theSettings);
            Buffer deflatedData;
            int status1 = deflateListing(gameDataPath, inflatedData, deflatedData);
            if (status1 != 0) return printf_err(status1, "failed to compress fileListing\n");
            theSettings.setOutFilePath(gameDataPath);
            cmn::log(cmn::eLog::info, "Savefile size: {}\n", deflatedData.size());


            // FILE INFO
            fs::path fileInfoPath = folder0 / "THUMB";
            Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);
            try {
                DataWriter::writeFile(fileInfoPath, fileInfoData.span());
            } catch(const std::exception& error) {
                return printf_err(-1, "failed to write fileInfo to \"%s\"\n",
                                  fileInfoPath.string().c_str());
            }
        }




        // folders 1-N
        {
            static constexpr u32 BYTES_PER_SAVE = 62'914'560;

            std::vector<std::vector<LCEFile*>> splitSavesPtrVec;
            splitSavesPtrVec.emplace_back();

            // build buckets of  region files
            u32 currentSize = 0;
            for (auto& file : saveProject.view_of(lce::FILETYPE::NEW_REGION_ANY)) {
                c_u32 fileSize = file.detectSize();
                if (currentSize + fileSize >= BYTES_PER_SAVE) {
                    splitSavesPtrVec.emplace_back();
                    currentSize = 0;
                }
                splitSavesPtrVec.back().push_back(&file);
                currentSize += fileSize;
            }

            // move files to correct folders
            int folderNum = 1;
            for (std::vector<LCEFile*>& splitSaveVec : splitSavesPtrVec) {
                auto folderNName = makePS4Folder(folderNum);
                auto folderN = root / folderNName;
                fs::create_directories(folderN);
                placeSceSysFiles(saveProject, folderN, strPCode, folderNName, theSettings);

                for (LCEFile* region : splitSaveVec) {
                    const fs::path oldPath = region->path();
                    try {
                        fs::rename(oldPath, folderN / region->getFileName());
                    } catch (const fs::filesystem_error& e) {
                        std::cerr << "Error moving file: " << e.what() << "\n";
                    }
                }

                folderNum++;
            }

        }

        // If we made it here, it's a miracle
        return SUCCESS;
    }


    int PS4::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        deflatedData.allocate(compressBound(inflatedData.size()));

        if (compress(deflatedData.data(), reinterpret_cast<uLongf*>(deflatedData.size_ptr()),
                     inflatedData.data(), inflatedData.size()) != Z_OK) {
            return COMPRESS;
        }

        DataWriter writer(deflatedData.size() + 8, Endian::Little);
        writer.write<u64>(inflatedData.size());
        writer.writeBytes(deflatedData.data(), deflatedData.size());
        try {
            writer.save(gameDataPath.string().c_str());
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR,
                              "failed to write savefile to \"%s\"\n",
                              gameDataPath.string().c_str());
        }

        return SUCCESS;
    }


    int PS4::writeExternalFolders(SaveProject& saveProject, WriteSettings& theSettings) const {
        return SUCCESS;
    }


    std::optional<fs::path> PS4::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "THUMB";
    }


}