#include "NewGenMCS.hpp"

#include "include/zlib-1.2.12/zlib.h"
#include "include/tinf/tinf.h"

#include "common/utils.hpp"
#include "common/data/DataWriter.hpp"
#include "common/fmt.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int NewGenMCS::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        return SUCCESS;
    }


    int NewGenMCS::inflateListing(SaveProject& saveProject) {
        Buffer dst;

        if (!saveProject.m_stateSettings.isCompressed()) {
            dst = readRaw(m_filePath);
        } else {
            Buffer src;
            DataReader reader;

            try {
                src = DataReader::readFile(m_filePath);
                reader = DataReader(src.span(), Endian::Little);
                if (reader.size() < 12) {
                    return printf_err(FILE_ERROR, ERROR_5);
                }
            } catch (const std::exception& e) {
                return printf_err(FILE_ERROR, ERROR_4,
                                  m_filePath.string().c_str());
            }

            reader.read<u32>();
            u32 dst_size = reader.read<u32>();

            // garbage nonsense
            if (dst_size > 629'145'600) {
                saveProject.m_stateSettings.setConsole(lce::CONSOLE::NEWGENMCS_BIG);
                reader.setEndian(Endian::Big);
                dst_size = reader.peek_at<u32>(4);
                reader.seek(8);
            }

            size_t fileSize = reader.size();

            bool isOldFormat = (fileSize == dst_size + 8);

            size_t compLen = fileSize - 8;
            if (isOldFormat) {
                // scans for the first 0 after the zlib payload
                auto* data = reader.data() + 8;
                auto* ptr = data + compLen;
                while (ptr > data && *(--ptr) == 0) {}
                compLen = static_cast<size_t>(ptr - data) + 1;
            }

            if (!dst.allocate(dst_size)) {
                return printf_err(MALLOC_FAILED, ERROR_1, dst_size);
            }

            unsigned int outLen = dst.size();
            int status = tinf_zlib_uncompress(
                    dst.data(),
                    &outLen,
                    reader.data() + 8,
                    static_cast<unsigned int>(compLen));
            if (status != TINF_OK) {
                return DECOMPRESS;
            }
        }

        int status = FileListing::readListing(saveProject, dst, saveProject.m_stateSettings.console());
        return status == 0 ? SUCCESS : -1;
    }


    int NewGenMCS::deflateToSave(SaveProject& saveProject, WriteSettings& theSettings) const {
        int status;

        const fs::path rootPath = theSettings.getInFolderPath();

        // GAMEDATA
        theSettings.m_fileNameOut = getCurrentDateTimeString();
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
        fileInfoPath += ".ext";

        Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);
        try {
            DataWriter::writeFile(fileInfoPath, fileInfoData.span());
        } catch(const std::exception& error) {
            return printf_err(status,
                              "failed to write fileInfo to \"%s\"\n",
                              fileInfoPath.string().c_str());
        }

        return SUCCESS;
    }


    int NewGenMCS::deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const {
        deflatedData.allocate(compressBound(inflatedData.size()));

        if (compress(deflatedData.data(), reinterpret_cast<uLongf*>(deflatedData.size_ptr()),
                     inflatedData.data(), inflatedData.size()) != Z_OK) {
            return COMPRESS;
        }

        DataWriter writer(inflatedData.size() + 8);
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


    std::optional<fs::path> NewGenMCS::getFileInfoPath(SaveProject& saveProject) const {
        return std::nullopt;
    }

}


