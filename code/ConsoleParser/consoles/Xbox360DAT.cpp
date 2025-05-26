#include "Xbox360DAT.hpp"

#include "common/codec/XDecompress.hpp"
#include "common/utils.hpp"

#include "code/FileListing/fileListing.hpp"
#include "code/SaveProject/SaveProject.hpp"


namespace editor {


    int Xbox360DAT::inflateFromLayout(const fs::path& theFilePath, SaveProject* saveProject) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        return SUCCESS;
    }


    int Xbox360DAT::inflateListing(MU SaveProject* saveProject) {
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

        u32 src_size = reader.read<u32>() - 8;
        reader.read<i32>();
        u32 file_size = reader.read<u32>();

        Buffer inflatedData;
       if(!inflatedData.allocate(file_size)) {
           return printf_err(MALLOC_FAILED, ERROR_1, file_size);
       }

       codec::XmemErr error = codec::XDecompress(
               reader.ptr(), src_size, inflatedData.data(), inflatedData.size_ptr());
        if (error != codec::XmemErr::Ok) {
            return printf_err(DECOMPRESS, "%s (%s)", ERROR_3, to_string(error));
        }

        if (inflatedData.empty()) {
            return printf_err(DECOMPRESS, "%s", ERROR_3);
        }

        int status = saveProject->m_fileListing.readListing(inflatedData, m_console);

        return status;
    }


    int Xbox360DAT::deflateToSave(MU SaveProject* saveProject, MU WriteSettings& theSettings) const {
        printf("Xbox360DAT.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Xbox360DAT::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }
}