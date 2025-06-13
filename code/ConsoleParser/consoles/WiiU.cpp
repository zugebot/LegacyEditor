#include "WiiU.hpp"

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


    int WiiU::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        return SUCCESS;
    }


    int WiiU::inflateListing(SaveProject& saveProject) {
        Buffer fileData;
        DataReader reader;

        try {
            fileData = DataReader::readFile(m_filePath);
            reader = DataReader(fileData.span(), Endian::Big);
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }
        if (reader.size() < 12) {
            return printf_err(FILE_ERROR, ERROR_5);
        }

        Buffer dst;
        if(u32 dst_size = reader.read<u64>();
            !dst.allocate(dst_size)) {
            return printf_err(MALLOC_FAILED, ERROR_1, dst_size);
        }

        int status = tinf_zlib_uncompress(dst.data(), dst.size_ptr(),
                                          reader.data() + 8, reader.size() - 8);
        if (status != 0) {
            return DECOMPRESS;
        }

        status = FileListing::readListing(saveProject, dst, m_console);
        if (status != 0) {
            return -1;
        }

        return SUCCESS;
    }


    int WiiU::deflateToSave(SaveProject& saveProject, WriteSettings& theSettings) const {
        int status;

        const fs::path rootPath = theSettings.getInFolderPath();

        // GAMEDATA
        fs::path gameDataPath = rootPath / getCurrentDateTimeString();
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


    int WiiU::deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const {
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

}


