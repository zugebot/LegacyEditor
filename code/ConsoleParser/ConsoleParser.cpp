#include "ConsoleParser.hpp"

#include "code/SaveProject/SaveProject.hpp"


namespace editor {


    void ConsoleParser::readFileInfo(SaveProject* saveProject) const {
        fs::path filePath = m_filePath.parent_path();
        fs::path cachePathVita = m_filePath.parent_path().parent_path();
        cachePathVita /= "CACHE.BIN";

        switch (m_console) {
            case lce::CONSOLE::PS3:
            case lce::CONSOLE::RPCS3:
            case lce::CONSOLE::PS4:
                filePath /= "THUMB";
                break;
            case lce::CONSOLE::VITA:
                filePath /= "THUMBDATA.BIN";
                break;
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::SWITCH: {
                filePath = m_filePath;
                filePath.replace_extension(".ext");
                break;
            }
            case lce::CONSOLE::XBOX360:
                goto XBOX360_SKIP_READING_FILEINFO;
            case lce::CONSOLE::NONE:
            default:
                return;
        }

        if (fs::exists(filePath)) {
            Buffer buffer = DataReader::readFile(filePath);
            saveProject->m_displayMetadata.read(buffer, m_console);

        } else if (m_console == lce::CONSOLE::VITA && fs::exists(cachePathVita)) {
            std::string folderName = m_filePath.parent_path().filename().string();
            saveProject->m_displayMetadata.readCacheFile(cachePathVita, folderName);

        } else {
            printf("[!] DisplayMetadata file not found, setting defaulted data.\n");
        }

    XBOX360_SKIP_READING_FILEINFO:
        defaultFileInfo(saveProject);
    }


    void ConsoleParser::defaultFileInfo(SaveProject* saveProject) {
        if (!saveProject->m_displayMetadata.isLoaded) {
            saveProject->m_displayMetadata.defaultSettings();
            saveProject->m_displayMetadata.loadFileAsThumbnail("assets/LegacyEditor/world-icon.png");
        }

        if (saveProject->m_displayMetadata.worldName.empty()) {
            saveProject->m_displayMetadata.worldName = L"New World";
        }
    }
}