#include "Vita.hpp"

#include "include/sfo/sfo.hpp"
#include "common/utils.hpp"
#include "common/rle/rle_vita.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int Vita::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        return SUCCESS;
    }


    int Vita::inflateListing(SaveProject& saveProject) {
        Buffer fileData;
        DataReader reader;

        try {
            fileData = DataReader::readFile(m_filePath);
            reader = DataReader(fileData.span(), Endian::Little);
            if (reader.size() < 12) {
                return printf_err(FILE_ERROR, ERROR_5);
            }
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        Buffer dst;
        if(u32 dst_size = reader.peek_at<u32>(4);
            !dst.allocate(dst_size)) {
            return printf_err(MALLOC_FAILED, ERROR_1, dst_size);
        }

        codec::RLEVITA_DECOMPRESS(reader.data() + 8, reader.size() - 8, dst.data(), dst.size());

        int status = FileListing::readListing(saveProject, dst, m_console);
        if (status != 0) {
            return -1;
        }

        return SUCCESS;
    }


    int Vita::deflateToSave(SaveProject& saveProject, WriteSettings& theSettings) const {
        int status;
        fs::path rootPath = theSettings.getInFolderPath();

        // FIND PRODUCT CODE
        auto productCode = theSettings.m_productCodes.getVITA();
        std::string strProductCode = VITAMapper.toString(productCode);
        std::string strCurrentTime = getCurrentDateTimeString();
        std::string folderName = strProductCode + "--" + strCurrentTime;
        rootPath /= folderName;
        fs::create_directories(rootPath);

        // GAMEDATA
        fs::path gameDataPath = rootPath / "GAMEDATA.bin";
        Buffer deflatedData = FileListing::writeListing(saveProject, theSettings);
        Buffer inflatedData;

        status = deflateListing(gameDataPath, deflatedData, inflatedData);
        if (status != 0) return printf_err(status,
                                           "failed to compress fileListing\n");
        theSettings.setOutFilePath(gameDataPath);
        printf("[*] gamedata final size: %u\n", deflatedData.size());


        // FILE INFO
        fs::path fileInfoPath = rootPath / "THUMBDATA.bin";
        Buffer fileInfoData = saveProject.m_displayMetadata.write(m_console);

        try {
            DataWriter::writeFile(fileInfoPath, fileInfoData.span());
        } catch(const std::exception& error) {
            return printf_err(status,
                              "failed to write fileInfo to \"%s\"\n",
                              fileInfoPath.string().c_str());
        }

        // UPDATE CACHE.BIN
        // const fs::path cacheBinPath = saveProject.m_stateSettings.m_psVitaCachePath;
        // if (!cacheBinPath.empty()) {
        //     if (fs::exists(cacheBinPath)) {

        //     }
        // }


        return SUCCESS;
    }


    int Vita::deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const {
        deflatedData.allocate(inflatedData.size() + 2);

        *deflatedData.size_ptr() = codec::RLEVITA_COMPRESS(
                inflatedData.data(), inflatedData.size(),
                deflatedData.data(), deflatedData.size());


        // 4-bytes of '0'
        // 4-bytes of total decompressed fileListing size
        // N-bytes fileListing data
        DataWriter writer(inflatedData.size() + 8, Endian::Little);
        writer.write<u32>(0);
        writer.write<u32>(inflatedData.size());
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


    std::optional<fs::path> Vita::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "THUMBDATA.BIN";
    }


}

