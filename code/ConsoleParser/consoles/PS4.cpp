#include "PS4.hpp"

#include "include/sfo/sfo.hpp"
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

        if (!saveProject.m_stateSettings.isMCS()) {
            readExternalFolders(saveProject);
        }

        return SUCCESS;
    }


    std::vector<fs::path> PS4::findExternalFolder(SaveProject& saveProject) {
        // go from "root/00000001/savedata0/GAMEDATA" to "root/00000001/savedata0"
        const fs::path mainDirPath = saveProject.m_stateSettings.filePath().parent_path();


        SFOManager mainSFO;

        // get sfo data from "root/00000001/savedata0/sce_sys/param.sfo"
        {
            const fs::path sfoFilePath = mainDirPath / "sce_sys" / "param.sfo";
            if (!fs::exists(sfoFilePath)) {
                printf("input folder does not have a sce_sys/param.sfo, returning early");
                return {""};
            }
            mainSFO.loadFile(sfoFilePath.string());
        }

        {
            const fs::path icon0Path = mainDirPath / "sce_sys" / "icon0.png";
            if (!saveProject.m_displayMetadata.icon0png.isValid() && fs::exists(icon0Path)) {
                saveProject.m_displayMetadata.icon0png.loadFromFile(icon0Path.string().c_str());
            }
        }

        // get the "CUSA00744-240620222358.0"-alike str from the main "param.sfo"
        {
            const std::wstring subtitle = stringToWstring(mainSFO.getStringAttribute("SUBTITLE").value_or("New World"));
            saveProject.m_displayMetadata.worldName = subtitle;
        }



        std::vector<std::string> mainAttrParts;

        // invalid SFO
        {
            std::optional<std::string> mainAttr = mainSFO.getStringAttribute("SAVEDATA_DIRECTORY");
            if (!mainAttr) {
                return {""};
            } else {
                mainAttrParts = split(mainAttr.value(), '.');
                if (mainAttrParts.size() != 2) {
                    printf("param.sfo does not seem to be formatted correctly.");
                    return {""};
                }
            }
        }

        // the vector of directories to add regions from
        std::vector<fs::path> directoriesToReturn{};


        // it goes "{ROOT}/PS4-CUSA#####-#CUSA#####-#########/savedata0/FILES"
        // all initial folders sit inside the "{ROOT}" folder
        if (mainDirPath.filename() == "savedata0") {

            // go from "root/00000001/savedata0" to "root"
            const fs::path toCheckDirPath = mainDirPath.parent_path().parent_path();
            // checks each folder in "root"
            for (c_auto &entry: fs::directory_iterator(toCheckDirPath)) {
                fs::path tempCheckDirPath = entry.path() / "savedata0";

                // find valid paths
                // this might not be necessary
                {
                    if (!fs::is_directory(entry.status())) { continue; } // skip entries that are not directories
                    if (mainDirPath.parent_path() == entry.path()) { continue; } // skips checking the input folder
                    if (entry.path().filename().string().contains("OPTIONS")) { continue; }
                    if (entry.path().filename().string().contains("CACHE")) { continue; }
                    if (!fs::exists(tempCheckDirPath)) { continue; }
                }


                // fetch info from param.sfo
                {
                    SFOManager tempSFO;
                    if (fs::path tempSFOFilePath = entry.path() / "savedata0" / "sce_sys" / "param.sfo";
                        !fs::exists(tempSFOFilePath)) {
                        continue;
                    } else {
                        tempSFO.loadFile(tempSFOFilePath.string());
                    }

                    // get the "CUSA00744-240620222358.0"-alike str from the temp "param.sfo"
                    std::optional<std::string> tempAttr = tempSFO.getStringAttribute("SAVEDATA_DIRECTORY");
                    // invalid SFO
                    if (!tempAttr) {
                        cmn::log(cmn::eLog::error, "Skipping folder "
                            + entry.path().string() + " and returning early, should this be a continue?\n");
                        return {""};
                    }
                    auto tempAttrParts = split(tempAttr.value(), '.');
                    if (tempAttrParts.size() != 2) { continue; }
                    // skip PS4 worlds that are not the same as the one being looked for
                    if (mainAttrParts[0] != tempAttrParts[0]) { continue; }
                }

                directoriesToReturn.push_back(tempCheckDirPath);
            }


        // otherwise, just check every folder right outside
        } else {

            // go from "root/00000001/savedata0" to "root"
            const fs::path toCheckDirPath = mainDirPath.parent_path();
            // checks each folder in "root"
            for (c_auto &entry: fs::directory_iterator(toCheckDirPath)) {
                const fs::path& tempCheckDirPath = entry.path();

                // find valid paths
                // this might not be necessary
                {
                    if (!fs::is_directory(entry.status())) { continue; } // skip entries that are not directories
                    if (mainDirPath == entry.path()) { continue; } // skips checking the input folder
                    if (entry.path().filename().string().contains("OPTIONS")) { continue; }
                    if (entry.path().filename().string().contains("CACHE")) { continue; }
                    if (!fs::exists(tempCheckDirPath)) { continue; }
                }


                // fetch info from param.sfo
                {
                    fs::path tempSFOFilePath = entry.path() / "sce_sys" / "param.sfo";
                    if (!fs::exists(tempSFOFilePath)) { continue; }
                    // get the "CUSA00744-240620222358.0"-alike str from the temp "param.sfo"
                    SFOManager tempSFO(tempSFOFilePath.string());
                    std::optional<std::string> tempAttr = tempSFO.getStringAttribute("SAVEDATA_DIRECTORY");
                    // invalid SFO
                    if (!tempAttr) { return {""}; }
                    auto tempAttrParts = split(tempAttr.value(), '.');
                    if (tempAttrParts.size() != 2) { continue; }
                    // skip PS4 worlds that are not the same as the one being looked for
                    if (mainAttrParts[0] != tempAttrParts[0]) {
                        continue;
                    }
                }

                directoriesToReturn.push_back(tempCheckDirPath);
            }
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


    static void writeIcon0Png(SaveProject& saveProject,
                              const fs::path& folder_sce_sys,
                              const fs::path& fileInfoPath) {
        fs::path icon0png = folder_sce_sys / "icon0.png";

        if (saveProject.m_displayMetadata.icon0png.m_data == nullptr) {
            Picture fileInfoPng;
            fileInfoPng.loadFromFile(fileInfoPath.string().c_str());
            saveProject.m_displayMetadata.icon0png.allocate(228, 128, 3);
            saveProject.m_displayMetadata.icon0png.fillColor(0, 0, 0);
            saveProject.m_displayMetadata.icon0png.placeAndStretchSubImage(&fileInfoPng, 0, 0, 228, 128);
        }
        saveProject.m_displayMetadata.icon0png.saveWithName(icon0png.string());
    }

    static void writeKeystone(const fs::path& folder_sce_sys, const std::string& pc) {
        static constexpr unsigned char CUSA00744_keystone[96] = {
                0x6B, 0x65, 0x79, 0x73, 0x74, 0x6F, 0x6E, 0x65, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x5C, 0x8D, 0x01, 0x55, 0xD5, 0x42, 0xB0, 0x36, 0xD8, 0xF8, 0x45, 0x9A, 0x32, 0x9E, 0x1C, 0xC5,
                0xAC, 0x06, 0x22, 0x79, 0xAF, 0xBE, 0xA2, 0x96, 0x8C, 0xDE, 0x53, 0x84, 0x03, 0x41, 0x33, 0xC2,
                0xD1, 0xDC, 0x9E, 0xBE, 0x43, 0xEB, 0x9C, 0x56, 0x37, 0xB1, 0x21, 0x90, 0xEE, 0xE6, 0xD6, 0x77,
                0x1D, 0x53, 0xFF, 0x0B, 0xDC, 0x13, 0x86, 0x88, 0x52, 0xA2, 0x73, 0xCA, 0xFB, 0x11, 0xB0, 0x41,
        };

        static constexpr unsigned char CUSA00265_keystone[96] = {
                0x6B, 0x65, 0x79, 0x73, 0x74, 0x6F, 0x6E, 0x65, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x5C, 0x8D, 0x01, 0x55, 0xD5, 0x42, 0xB0, 0x36, 0xD8, 0xF8, 0x45, 0x9A, 0x32, 0x9E, 0x1C, 0xC5,
                0xAC, 0x06, 0x22, 0x79, 0xAF, 0xBE, 0xA2, 0x96, 0x8C, 0xDE, 0x53, 0x84, 0x03, 0x41, 0x33, 0xC2,
                0xD1, 0xDC, 0x9E, 0xBE, 0x43, 0xEB, 0x9C, 0x56, 0x37, 0xB1, 0x21, 0x90, 0xEE, 0xE6, 0xD6, 0x77,
                0x1D, 0x53, 0xFF, 0x0B, 0xDC, 0x13, 0x86, 0x88, 0x52, 0xA2, 0x73, 0xCA, 0xFB, 0x11, 0xB0, 0x41
        };

        /*
         * I have no idea what this actually is,
         * I believe that the PS bots are replacing the keystone when they shouldn't be.
        static constexpr unsigned char CUSA00265_keystone[96] = {
                0x6B, 0x65, 0x79, 0x73, 0x74, 0x6F, 0x6E, 0x65, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x29, 0x4A, 0x5E, 0xD0, 0x6D, 0xB1, 0x70, 0x61, 0x8F, 0x2E, 0xED, 0x8C, 0x42, 0x4B, 0x9D, 0x82,
                0x88, 0x79, 0xC0, 0x80, 0xCC, 0x66, 0xFB, 0xC4, 0x86, 0x4F, 0x69, 0xE9, 0x74, 0xDE, 0xB8, 0x56,
                0xFA, 0x0D, 0x0C, 0x2E, 0xBD, 0x6A, 0x00, 0x80, 0x63, 0x71, 0x3D, 0xE8, 0x81, 0x0D, 0x7E, 0x10,
                0xB7, 0x32, 0x14, 0x3B, 0x91, 0xCD, 0x2E, 0x4F, 0xEA, 0x2D, 0x20, 0x53, 0x10, 0x6E, 0xB7, 0x5D,
        };
        */


        // unknown, so just supplying USA / Europe keystone bytes
        static constexpr unsigned char CUSA00283_keystone[96] = {
            0x6B, 0x65, 0x79, 0x73, 0x74, 0x6F, 0x6E, 0x65, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x5C, 0x8D, 0x01, 0x55, 0xD5, 0x42, 0xB0, 0x36, 0xD8, 0xF8, 0x45, 0x9A, 0x32, 0x9E, 0x1C, 0xC5,
            0xAC, 0x06, 0x22, 0x79, 0xAF, 0xBE, 0xA2, 0x96, 0x8C, 0xDE, 0x53, 0x84, 0x03, 0x41, 0x33, 0xC2,
            0xD1, 0xDC, 0x9E, 0xBE, 0x43, 0xEB, 0x9C, 0x56, 0x37, 0xB1, 0x21, 0x90, 0xEE, 0xE6, 0xD6, 0x77,
            0x1D, 0x53, 0xFF, 0x0B, 0xDC, 0x13, 0x86, 0x88, 0x52, 0xA2, 0x73, 0xCA, 0xFB, 0x11, 0xB0, 0x41
        };

        fs::path keystonePath = folder_sce_sys / "keystone";
        const u8* chosenKeystone = nullptr;

        if (pc == "CUSA00744") {
            chosenKeystone = &CUSA00744_keystone[0];
        }
        if (pc == "CUSA00265") {
            chosenKeystone = &CUSA00265_keystone[0];
        }
        if (pc == "CUSA00283") {
            chosenKeystone = &CUSA00283_keystone[0];
        }

        if (!chosenKeystone) {
            cmn::log(cmn::eLog::error, "\"{}\" is not a valid PS4 Product Code", pc);
            return;
        }

        Buffer keystoneBuf(96);
        std::memcpy(keystoneBuf.data(), chosenKeystone, 96);
        DataWriter::writeFile(keystonePath, keystoneBuf.span());
    }

    static void writeSceIcon0png1(const fs::path& folder_sce_sys) {
        fs::path sce_icon0png1Path = folder_sce_sys / "sce_icon0png1";
        Buffer sce_icon0png1Buf(116736);
        DataWriter::writeFile(sce_icon0png1Path, sce_icon0png1Buf.span());
    }

    static void writeParamSfo(SaveProject& saveProject,
                              const fs::path& folder_sce_sys,
                              const std::string& pc,
                              const int intN,
                              const std::string& folderN,
                              WriteSettings& theSettings) {
        fs::path sfoPath = folder_sce_sys / "param.sfo";

        if (!fs::exists(theSettings.m_paramSfoToReplace)) {
            throw std::runtime_error("Input param.sfo file not found, exiting\n");
        }
        SFOManager other(theSettings.m_paramSfoToReplace.string());
        SFOManager sfo;
        sfo.setMagic(eSFO_MAGIC::PS3_HDD);

        if (saveProject.m_stateSettings.console() == lce::CONSOLE::SHADPS4) {
            // Simplified SFO for SHADPS4?
            sfo.setLexicographic(false);
            std::string data(8, '\0');
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", data);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "MAINTITLE", "Minecraft: PlayStation®4 Edition");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SUBTITLE", wStringToString(saveProject.m_displayMetadata.worldName));
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", "");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_DIRECTORY", folderN);
            sfo.addParam(eSFO_FMT::INT, "SAVEDATA_LIST_PARAM", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "TITLE_ID", pc);
            std::string savedata_blocks("\x00\x02\x00\x00\x00\x00\x00\x00", 8);
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "SAVEDATA_BLOCKS", savedata_blocks);

        } else if (saveProject.m_stateSettings.console() == lce::CONSOLE::PS4) {
            // Regular PS4 flow
            auto accountAttr = other.getAttribute("ACCOUNT_ID");
            if (!accountAttr) throw std::runtime_error("Input param.sfo missing ACCOUNT_ID");
            auto accountVal = std::get<std::vector<uint8_t>>(accountAttr.value().myValue);
            std::string accountIdStr = std::string((char*)accountVal.data(), accountVal.size());
            // std::cout << "AccountID: \"" << accountIdStr << "\" Size: " << accountIdStr.size() << "\n";
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", accountIdStr);

            sfo.addParam(eSFO_FMT::INT, "ATTRIBUTE", "0");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "CATEGORY", "sd");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", "");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "FORMAT", "obs");
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "MAINTITLE", "Minecraft: PlayStation®4 Edition");

            auto paramsAttr = other.getAttribute("PARAMS");
            if (!paramsAttr) throw std::runtime_error("Input param.sfo missing PARAMS");

            auto array = std::get<std::vector<uint8_t>>(paramsAttr->myValue);
            DataReader reader(array.data(), array.size(), Endian::Little);

            std::vector<uint8_t> params(1024);
            DataWriter writer(params.data(), params.size(), Endian::Little);

            // Get current UNIX time
            auto now = std::chrono::system_clock::now();
            u32 lastModifiedTime = std::chrono::duration_cast<std::chrono::seconds>(
                                           now.time_since_epoch()).count();

            u32 flagUnknown1 = 0x0A; // observed range 0x2–0xA

            writer.write<u32>(0); reader.skip(4);
            writer.writeBytes(reader.ptr(), 4); reader.skip(4);
            writer.writeBytes(reader.ptr(), 32); reader.skip(32);

            writer.write<u32>(1); reader.skip(4);
            writer.writeBytes((c_u8*)pc.data(), pc.size()); writer.skip(16 - pc.size()); reader.skip(16);
            writer.writeBytes((c_u8*)pc.data(), pc.size()); writer.skip(16 - pc.size()); reader.skip(16);

            writer.write<u32>(reader.read<u32>() + 1);  // modificationCount
            writer.skip(12); reader.skip(12);

            writer.write<u32>(flagUnknown1); reader.skip(4);
            writer.write<u32>(reader.read<u32>()); // creationTime

            writer.write<u32>(0); reader.skip(4);
            writer.write<u32>(lastModifiedTime); reader.skip(4);

            std::string params_str((const char*)params.data(), params.size());
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "PARAMS", params_str);

            std::string savedata_blocks("\x00\x02\x00\x00\x00\x00\x00\x00", 8);
            sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "SAVEDATA_BLOCKS", savedata_blocks);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_DIRECTORY", folderN);
            sfo.addParam(eSFO_FMT::INT, "SAVEDATA_LIST_PARAM", "0");
            std::string fileName = wStringToString(saveProject.m_displayMetadata.worldName);
            if (intN > 0) {
                fileName += " - World Data " + std::to_string(intN);
            }
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SUBTITLE", fileName);
            sfo.addParam(eSFO_FMT::UTF8_NORMAL, "TITLE_ID", pc);
        } else {
            throw std::runtime_error("PS4::writeParamSfo was given lce::CONSOLE::" +
                                     lce::consoleToStr(saveProject.m_stateSettings.console()));
        }

        sfo.saveToFile(sfoPath.string());
    }

    // sce_sys/icon0.png
    //      228*128, bit-depth[24]
    // sce_sys/keystone
    // sce_sys/param.sfo
    // sce_sys/sce_icon0png1
    //      116736 bytes of null
    static void placeSceSysFiles(SaveProject& saveProject,
                                 const fs::path& root,
                                 const std::string& pc,
                                 const int intN,
                                 const std::string& folderN,
                                 const fs::path& fileInfoPath,
                                 WriteSettings& theSettings) {
        fs::path folder_sce_sys = root / "sce_sys";
        fs::create_directories(folder_sce_sys);

        writeIcon0Png(saveProject, folder_sce_sys, fileInfoPath);
        writeKeystone(folder_sce_sys, pc);
        writeSceIcon0png1(folder_sce_sys);
        writeParamSfo(saveProject, folder_sce_sys, pc, intN, folderN, theSettings);
    }



    int PS4::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {

        std::string dateTimeStr = getCurrentDateTimeString();

        std::string strPCode = PS4Mapper.toString(theSettings.m_productCodes.getPS4());
        auto makePS4Folder = [&](int digit) {
            return strPCode + "-" + dateTimeStr + "." + std::to_string(digit);
        };

        const fs::path root = theSettings.getInFolderPath();

        fs::path fileInfoPath;

        // folder0
        {
            // FOLDER CREATION
            auto folder0Name = makePS4Folder(0/*, folderType*/);
            auto folder0 = root / folder0Name;
            fs::create_directories(folder0);

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
            fileInfoPath = folder0 / "THUMB";
            Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);
            try {
                DataWriter::writeFile(fileInfoPath, fileInfoData.span());
            } catch(const std::exception& error) {
                return printf_err(-1, "failed to write fileInfo to \"%s\"\n",
                                  fileInfoPath.string().c_str());
            }

            // SCE_SYS_FILES
            placeSceSysFiles(saveProject, folder0, strPCode, 0, folder0Name, fileInfoPath, theSettings);
        }


        // folders 1-N
        {
            // SPLIT SAVES INTO BUCKETS
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
                auto folderNName = makePS4Folder(folderNum/*, folderType*/);
                auto folderN = root / folderNName;
                fs::create_directories(folderN);
                placeSceSysFiles(saveProject, folderN, strPCode, folderNum, folderNName, fileInfoPath, theSettings);

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
        writer.write<u32>(0);
        writer.write<u32>(inflatedData.size());
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