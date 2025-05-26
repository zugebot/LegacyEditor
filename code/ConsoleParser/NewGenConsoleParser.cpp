#include "NewGenConsoleParser.hpp"

#include "code/FileListing/fileListing.hpp"
#include "code/SaveProject/SaveProject.hpp"
#include "common/RLE/rle_nsxps4.hpp"
#include "common/utils.hpp"
#include "include/tinf/tinf.h"

namespace editor {

    int NewGenConsoleParser::inflateListing(SaveProject* saveProject) {
        Buffer data;

        FILE* f_in = fopen(m_filePath.string().c_str(), "rb");
        if (f_in == nullptr) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        fseek(f_in, 0, SEEK_END);
        u64 input_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);
        if (input_size < 12) {
            fclose(f_in);
            return printf_err(FILE_ERROR, ERROR_5);
        }

        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, 12, f_in);

        u32 final_size = headerUnion.getInt2Swap();
        if (!data.allocate(final_size)) {
            fclose(f_in);
            return printf_err(MALLOC_FAILED, ERROR_1, final_size);
        }

        input_size -= 8;
        Buffer src;
        if (!src.allocate(input_size)) {
            fclose(f_in);
            return printf_err(MALLOC_FAILED, ERROR_1, input_size);
        }

        fseek(f_in, 8, SEEK_SET);
        fread(src.data(), 1, input_size, f_in);
        fclose(f_in);

        int status = tinf_zlib_uncompress(data.data(), data.size_ptr(), src.data(), input_size);
        if (status != 0) {
            return DECOMPRESS;
        }

        status = saveProject->m_fileListing.readListing(data, m_console);
        if (status != 0) {
            return -1;
        }

        saveProject->m_fileListing.setNewGen(true);
        if (fs::path thumb = m_filePath.parent_path() / "THUMB";
            saveProject->m_stateSettings.console() == lce::CONSOLE::SWITCH
            && fs::exists(thumb)) {
            saveProject->m_stateSettings.setConsole(lce::CONSOLE::PS4);
        }

        return SUCCESS;
    }


    /**
     * \brief Ps4 / Switch / Xbox1
     * \param inDirPath the directory containing the GAMEDATA files.
     * \return
     */
    int NewGenConsoleParser::readExternalFolder(const fs::path& inDirPath, SaveProject* saveProject) {
        MU int fileIndex = -1;
        for (c_auto& file: fs::directory_iterator(inDirPath)) {

            if (is_directory(file)) { continue; }

            // initiate filename and filepath
            std::string filePathStr = file.path().string();
            std::string fileNameStr = file.path().filename().string();

            // open the file
            auto byteVec = DataReader::readFile(filePathStr);
            //< all of new-gen is little endian
            DataReader reader(byteVec.data(), byteVec.size(), Endian::Little);


            // skip empty regions (out of bounds end / nether possibly)
            if (reader.size() == 0) { continue; }

            fileIndex++;

            c_u32 fileSize = reader.read<u32>();

            Buffer buffer(fileSize);

            codec::RLE_NSX_OR_PS4_DECOMPRESS(reader.ptr(), reader.size() - 4,
                                             buffer.data(), buffer.size());


            // TODO: get m_timestamp from file itself / make one up
            u32 timestamp = 0;
            auto& lFile = saveProject->m_fileListing.emplaceFile(
                    saveProject->m_stateSettings.console(),
                    std::move(buffer),
                    timestamp);


            lFile.setFileName(fileNameStr);
            static constexpr lce::FILETYPE REGION_DIMENSIONS[3] = {
                    lce::FILETYPE::NEW_REGION_OVERWORLD,
                    lce::FILETYPE::NEW_REGION_NETHER,
                    lce::FILETYPE::NEW_REGION_END};
            if (c_auto dimChar = static_cast<char>(static_cast<int>(fileNameStr.at(12)) - 48);
                dimChar < 0 || dimChar > 2) {
                lFile.m_fileType = lce::FILETYPE::NONE;
            } else {
                lFile.m_fileType = REGION_DIMENSIONS[static_cast<int>(static_cast<u8>(dimChar))];
            }
            c_i16 rX = static_cast<i8>(strtol(fileNameStr.substr(13, 2).c_str(), nullptr, 16));
            c_i16 rZ = static_cast<i8>(strtol(fileNameStr.substr(15, 2).c_str(), nullptr, 16));
            lFile.setRegionX(rX);
            lFile.setRegionZ(rZ);
        }

        return SUCCESS;
    }

}
