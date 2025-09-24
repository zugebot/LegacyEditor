#include "Xbox1.hpp"

#include "lce/processor.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/writeSettings.hpp"
#include "common/utils.hpp"
#include "tinf/tinf.h"


#ifndef _GNU_SOURCE
void* mempcpy(void* dest, const void* src, size_t n) {
    return (char*)memcpy(dest, src, n) + n;
}
#endif

namespace editor {

    template<bool IsWindurango>
    int Xbox1<IsWindurango>::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        if constexpr (IsWindurango) {
            try {
                fs::path wd_displayNamePath = m_filePath.parent_path() / "wd_displayname.txt";
                Buffer displayNameBuffer = DataReader::readFile(wd_displayNamePath);
                std::string displayName;
                displayName.resize(displayNameBuffer.size());
                mempcpy(displayName.data(), displayNameBuffer.data(), displayNameBuffer.size());
                saveProject.m_displayMetadata.worldName = stringToWstring(displayName);
            } catch (const std::exception& e) {
                return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
            }
        } else {
            saveProject.m_displayMetadata.worldName = L"New World";
        }

        if (!saveProject.m_stateSettings.isMCS()) {
            readExternalFolders(saveProject);
        }
        return SUCCESS;
    }


    template<bool IsWindurango>
    int Xbox1<IsWindurango>::inflateListing(SaveProject& saveProject) {
        if constexpr (IsWindurango) {
            return NewGenConsoleParser::inflateListing(saveProject);
        }

        Buffer src;
        DataReader reader;

        try {
            src = DataReader::readFile(m_filePath);
            reader = DataReader(src, Endian::Little);
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        u32 final_size = reader.peek_at<u32>(4);

        Buffer dest;
        if (!dest.allocate(final_size)) {
            return printf_err(MALLOC_FAILED, ERROR_1, final_size);
        }

        int status = tinf_zlib_uncompress(dest.data(), dest.size_ptr(), src.data() + 8, src.size() - 8);
        if (status != TINF_OK && status != TINF_ADLER_ERROR) {
            return DECOMPRESS;
        }

        // DataWriter::writeFile("C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\GAMEDATA_WINDURANGO", data.span());

        status = FileListing::readListing(saveProject, dest, m_console);
        if (status != 0) {
            return -1;
        }

        saveProject.setNewGen(true);
        if (lce::is_switch_family(saveProject.m_stateSettings.console())
            && fs::exists(m_filePath.parent_path() / "THUMB")) {
            saveProject.m_stateSettings.setConsole(lce::CONSOLE::PS4);
        }

        return SUCCESS;
    }

    template<bool IsWindurango>
    int Xbox1<IsWindurango>::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        printf("Xbox1::deflateToSave(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    template<bool IsWindurango>
    int Xbox1<IsWindurango>::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        printf("Xbox1::deflateListing(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }

    template<bool IsWindurango>
    std::vector<fs::path> Xbox1<IsWindurango>::findExternalFolder(editor::SaveProject &saveProject) {
        fs::path folder = m_filePath.parent_path();
        if (folder.empty()) {
            return {};
        }
        return { folder };
    }


    template<bool IsWindurango>
    int Xbox1<IsWindurango>::readExternalFolders(SaveProject& saveProject) {
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


    template<bool IsWindurango>
    int Xbox1<IsWindurango>::writeExternalFolders(SaveProject& saveProject, WriteSettings& theSettings) const {
        printf("Xbox1::writeExternalFolders(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }

    template<bool IsWindurango>
    std::optional<fs::path> Xbox1<IsWindurango>::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "THUMB";
    }

    template class Xbox1<false>; // XBOX1
    template class Xbox1<true>;  // WINDURANGO
}
