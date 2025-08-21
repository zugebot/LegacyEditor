#include "ConsoleParser.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "common/Config.hpp"


namespace editor {

    Buffer ConsoleParser::readRaw(fs::path& inFilePath) {
        Buffer dest;

        try {
            dest = DataReader::readFile(inFilePath);
        } catch (const std::exception& e) {
            dest = Buffer();
            printf_err(FILE_ERROR, ERROR_4, inFilePath.string().c_str());
        }

        return dest;
    }


    // TODO: does not properly suit Xbox360 .bin/.dat's or Vita's Cache.bin.
    void ConsoleParser::readFileInfo(SaveProject& saveProject) const {

        auto filePath = getFileInfoPath(saveProject);
        fs::path cachePathVita = m_filePath.parent_path().parent_path() / "CACHE.BIN";

        // if (filePath.has_value() && fs::exists(filePath.value())) {

        //     if (m_console == lce::CONSOLE::VITA && fs::exists(cachePathVita)) {
        //         std::string folderName = m_filePath.parent_path().filename().string();
        //         saveProject.m_displayMetadata.readCacheFile(cachePathVita, folderName);

        //     } else {
        //         Buffer buffer = DataReader::readFile(filePath.value());
        //         saveProject.m_displayMetadata.read(buffer, m_console);
        //     }
        // } else {
        //     if (saveProject.m_stateSettings.isXbox360Bin()) {

        //     } else {
        //         printf("[!] DisplayMetadata file not found, setting defaulted data.\n");
        //         defaultFileInfo(saveProject);
        //     }
        // }


        if (filePath.has_value() && fs::exists(filePath.value())) {
            Buffer buffer = DataReader::readFile(filePath.value());
            saveProject.m_displayMetadata.read(buffer, m_console);

        } else if (m_console == lce::CONSOLE::VITA && fs::exists(cachePathVita)) {
            std::string folderName = m_filePath.parent_path().filename().string();
            saveProject.m_displayMetadata.readCacheFile(cachePathVita, folderName);

        } else if (m_console == lce::CONSOLE::XBOX360 && !saveProject.m_stateSettings.isXbox360Bin()) {
            defaultFileInfo(saveProject);
        } else {
            printf("[!] DisplayMetadata file not found, setting defaulted data.\n");
            defaultFileInfo(saveProject);
        }
    }


    void ConsoleParser::defaultFileInfo(SaveProject& saveProject) {
        if (!saveProject.m_displayMetadata.isLoaded) {
            saveProject.m_displayMetadata.defaultSettings();
            saveProject.m_displayMetadata.loadFileAsThumbnail(
                    fs::path(EXE_CURRENT_PATH) / "assets/LegacyEditor/world-icon.png");
        }

        if (saveProject.m_displayMetadata.worldName.empty()) {
            saveProject.m_displayMetadata.worldName = L"New World";
        }
    }
}