#include "NewGenConsoleParser.hpp"

#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "common/RLE/rle_nsxps4.hpp"
#include "common/utils.hpp"
#include "include/tinf/tinf.h"

namespace editor {

    int NewGenConsoleParser::inflateListing(SaveProject& saveProject) {
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
        if (status != 0) {
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

    /**
     * \brief Ps4 / Switch / Xbox1
     * \param inDirPath the directory containing the GAMEDATA files.
     * \return
     */
    int NewGenConsoleParser::readExternalFolder(SaveProject& saveProject, const fs::path& inDirPath) {
        MU int fileIndex = -1;
        for (c_auto& file: fs::directory_iterator(inDirPath)) {

            if (is_directory(file)) { continue; }

            // initiate filename and filepath
            std::string filePathStr = file.path().string();
            std::string fileNameStr = file.path().filename().string();
            if (!fileNameStr.starts_with("GAMEDATA_000")) {
                continue;
            }

            // open the file
            auto byteVec = DataReader::readFile(filePathStr);
            DataReader reader(byteVec.data(), byteVec.size(), Endian::Little);
            if (reader.size() == 0) { continue; }

            fileIndex++;

            // c_u32 fileSize = reader.read<u32>();
            // Buffer buffer(fileSize);
            // codec::RLE_NSX_OR_PS4_DECOMPRESS(reader.ptr(), reader.size() - 4,
            //                                  buffer.data(), buffer.size());


            // TODO: get m_timestamp from file itself / make one up
            u32 timestamp = 0;
            saveProject.emplaceFile(
                    saveProject.m_stateSettings.console(),
                    timestamp,
                    inDirPath,
                    "",
                    fileNameStr
                    );
        }

        return SUCCESS;
    }

}
