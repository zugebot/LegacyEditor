#include "Rpcs3.hpp"

#include "include/sfo/sfo.hpp"
#include "common/rle/rle_vita.hpp"
#include "common/utils.hpp"
#include "include/png/crc.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int RPCS3::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }
        readFileInfo(saveProject);
        readPARAM_SFO(saveProject);

        return SUCCESS;
    }


    int RPCS3::inflateListing(SaveProject& saveProject) {

        Buffer dest = readRaw(m_filePath);

        int status = FileListing::readListing(saveProject, dest, m_console);
        if (status != 0) {
            return -1;
        }

        return SUCCESS;
    }


    // TODO: make it not case-specific!
    // TODO: make it return a status!
    int RPCS3::readPARAM_SFO(SaveProject& saveProject) {
        fs::path sfoFilePath = m_filePath.parent_path();
        sfoFilePath += "/PARAM.SFO";

        // TODO: make it cache the ACCOUNT_ID for later converting
        SFOManager mainSFO(sfoFilePath.string());
        const std::wstring subtitle = stringToWstring(mainSFO.getStringAttribute("SUB_TITLE").value_or("New World"));
        saveProject.m_displayMetadata.worldName = subtitle;

        return SUCCESS;
    }


    // TODO: missing other files
    int RPCS3::deflateToSave(SaveProject& saveProject, WriteSettings& theSettings) const {
        int status;
        fs::path rootPath = theSettings.getInFolderPath();


        // FIND PRODUCT CODE
        auto productCode = theSettings.m_productCodes.getPS3();
        std::string strProductCode = PS3Mapper.toString(productCode);
        std::string strCurrentTime = getCurrentDateTimeString();
        std::string folderName = strProductCode + "--" + strCurrentTime;
        rootPath /= folderName;
        fs::create_directories(rootPath);


        // FILE INFO
        fs::path fileInfoPath = rootPath / "THUMB";
        Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);

        try {
            DataWriter::writeFile(fileInfoPath, fileInfoData.span());
        } catch (const std::exception& e) {
            return printf_err(status,
                              "failed to write fileInfo to \"%s\"\n",
                              fileInfoPath.string().c_str());
        }


        // ICON0.PNG
        fs::path icon0pngPath = rootPath / "ICON0.PNG";
        if (saveProject.m_displayMetadata.icon0png.m_data == nullptr) {
            Picture fileInfoPng;
            fileInfoPng.loadFromFile(fileInfoPath.string().c_str());
            saveProject.m_displayMetadata.icon0png.allocate(320, 176, 4);
            saveProject.m_displayMetadata.icon0png.fillColor(0, 0, 0);
            saveProject.m_displayMetadata.icon0png.placeAndStretchSubImage(&fileInfoPng, 72, 0, 176, 176);
        }
        saveProject.m_displayMetadata.icon0png.saveWithName(icon0pngPath.string());


        // GAMEDATA
        fs::path gameDataPath = rootPath / "GAMEDATA";
        Buffer inflatedData = FileListing::writeListing(saveProject, theSettings);
        Buffer deflatedData;
        status = deflateListing(gameDataPath, inflatedData, deflatedData);
        if (status != 0) return printf_err(status,
                                           "failed to compress fileListing\n");
        theSettings.setOutFilePath(gameDataPath);
        printf("[*] gamedata final size: %u\n", deflatedData.size());


        // METADATA
        fs::path metadataPath = rootPath / "METADATA";
        c_u32 crc1 = crc(deflatedData.data(), deflatedData.size());
        c_u32 crc2 = crc(fileInfoData.data(), fileInfoData.size());

        DataWriter managerMETADATA(256, Endian::Big);
        managerMETADATA.write<u32>(3);
        managerMETADATA.write<u32>(deflatedData.size());
        managerMETADATA.write<u32>(fileInfoData.size());
        managerMETADATA.write<u32>(crc1);
        managerMETADATA.write<u32>(crc2);

        try {
            DataWriter::writeFile(metadataPath, managerMETADATA.span());
        } catch (const std::exception& e) {
            return printf_err(status,
                              "failed to write metadata to \"%s\"\n",
                              metadataPath.string().c_str());
        }


        // PARAM.SFO
        fs::path sfoPath = rootPath / "PARAM.SFO";
        SFOManager sfo;
        sfo.addParam(eSFO_FMT::INT, "*GAMEDATA", "0");
        sfo.addParam(eSFO_FMT::INT, "*ICON0.PNG", "0");
        sfo.addParam(eSFO_FMT::INT, "*METADATA", "1");
        sfo.addParam(eSFO_FMT::INT, "*THUMB", "0");
        sfo.addParam(eSFO_FMT::UTF8_SPECIAL, "ACCOUNT_ID", "0000000000000000");
        sfo.addParam(eSFO_FMT::INT, "ATTRIBUTE", "0");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "CATEGORY", "SD");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "DETAIL", " ");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS", "");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "PARAMS2", "");
        sfo.addParam(eSFO_FMT::INT, "PARENTAL_LEVEL", "0");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "RPCS3_BLIST", "ICON0.PNG/METADATA/THUMB/GAMEDATA");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_DIRECTORY", folderName);
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SAVEDATA_LIST_PARAM", "0");
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "SUB_TITLE", wStringToString(saveProject.m_displayMetadata.worldName));
        std::string title = "Minecraft: PlayStation®3 Edition";
        if (productCode == ePS3ProductCode::BLES01976 ||
            productCode == ePS3ProductCode::BLUS31426) {
            title += " (" + PS3Mapper.toString(productCode) + ")";
        }
        sfo.addParam(eSFO_FMT::UTF8_NORMAL, "TITLE", title);
        sfo.setMagic(eSFO_MAGIC::PS3_HDD);
        sfo.saveToFile(sfoPath.string());


        return SUCCESS;
    }


    int RPCS3::deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const {
        deflatedData = std::move(inflatedData);

        try {
            DataWriter::writeFile(gameDataPath, deflatedData.span());
        } catch (const std::exception& e) {
            return printf_err(STATUS::FILE_ERROR,
                              "failed to write savefile to \"%s\"\n",
                              gameDataPath.string().c_str());
        }

        return SUCCESS;
    }


    std::optional<fs::path> RPCS3::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "THUMB";
    }
}
