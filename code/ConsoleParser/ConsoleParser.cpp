#include "ConsoleParser.hpp"

#include "code/SaveFile/SaveProject.hpp"


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


    void ConsoleParser::readFileInfo(SaveProject& saveProject) const {

        auto filePath = getFileInfoPath(saveProject);
        fs::path cachePathVita = m_filePath.parent_path().parent_path() / "CACHE.BIN";

        if (filePath.has_value() && fs::exists(filePath.value())) {
            Buffer buffer = DataReader::readFile(filePath.value());
            saveProject.m_displayMetadata.read(buffer, m_console);

        } else if (m_console == lce::CONSOLE::VITA && fs::exists(cachePathVita)) {
            std::string folderName = m_filePath.parent_path().filename().string();
            saveProject.m_displayMetadata.readCacheFile(cachePathVita, folderName);

        } else if (m_console == lce::CONSOLE::XBOX360) {
            defaultFileInfo(saveProject);
        } else {
            printf("[!] DisplayMetadata file not found, setting defaulted data.\n");
        }
    }


    void ConsoleParser::defaultFileInfo(SaveProject& saveProject) {
        if (!saveProject.m_displayMetadata.isLoaded) {
            saveProject.m_displayMetadata.defaultSettings();
            saveProject.m_displayMetadata.loadFileAsThumbnail("assets/LegacyEditor/world-icon.png");
        }

        if (saveProject.m_displayMetadata.worldName.empty()) {
            saveProject.m_displayMetadata.worldName = L"New World";
        }
    }
}