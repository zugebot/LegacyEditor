#include "Switch.hpp"

#include "lce/processor.hpp"
#include "tinf/tinf.h"
#include "common/utils.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "common/fmt.hpp"
#include "zlib-1.2.12/zlib.h"


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


    int Switch::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        int status;

        const fs::path rootPath = theSettings.getInFolderPath();

        // GAMEDATA
        theSettings.m_fileNameOut = getCurrentDateTimeString() + ".dat";
        fs::path gameDataPath = rootPath / theSettings.m_fileNameOut;
        Buffer inflatedData = FileListing::writeListing(saveProject, theSettings);

        Buffer deflatedData;

        status = deflateListing(gameDataPath, inflatedData, deflatedData);
        if (status != 0)
            return printf_err(status, "failed to compress fileListing\n");
        theSettings.setOutFilePath(gameDataPath);

        cmn::log(cmn::eLog::info, "Savefile size: {}\n", deflatedData.size());



        // FILE INFO
        fs::path fileInfoPath = gameDataPath;
        fileInfoPath.replace_extension(".ext");

        Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);
        try {
            DataWriter::writeFile(fileInfoPath, fileInfoData.span());
        } catch(const std::exception& error) {
            return printf_err(status,
                              "failed to write fileInfo to \"%s\"\n",
                              fileInfoPath.string().c_str());
        }

        writeExternalFolders(saveProject, theSettings);

        return SUCCESS;
    }


    int Switch::deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const {
        deflatedData.allocate(compressBound(inflatedData.size()));

        if (compress(deflatedData.data(), reinterpret_cast<uLongf*>(deflatedData.size_ptr()),
                     inflatedData.data(), inflatedData.size()) != Z_OK) {
            return COMPRESS;
        }

        DataWriter writer(deflatedData.size() + 8, Endian::Little);
        writer.writeSwitchU64(inflatedData.size());
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


    int Switch::writeExternalFolders(SaveProject& saveProject, WriteSettings& theSettings) const {

        c_auto rootPath = theSettings.getOutFilePath();
        const fs::path newFilePath = rootPath.parent_path() / rootPath.filename().replace_extension(".sub");
        if (!fs::exists(newFilePath)) {
            fs::create_directories(newFilePath);
        }

        for (auto& region : saveProject.view_of(SaveProject::s_NEW_REGION_ANY)) {
            const fs::path oldPath = region.path();
            try {
                fs::rename(oldPath, newFilePath / region.getFileName());
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error moving file: " << e.what() << "\n";
            }
        }

        return SUCCESS;
    }


    std::optional<fs::path> Switch::getFileInfoPath(SaveProject& saveProject) const {
        fs::path filePath = m_filePath;
        filePath.replace_extension(".ext");
        return filePath;
    }


}
