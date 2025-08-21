#include "Xbox360DAT.hpp"

#include "common/codec/XDecompress.hpp"
#include "common/utils.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {


    int Xbox360DAT::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        int status = inflateListing(saveProject);
        if (status != 0) {
            printf("failed to extract listing\n");
            return status;
        }

        readFileInfo(saveProject);

        return SUCCESS;
    }


    int Xbox360DAT::inflateListing(MU SaveProject& saveProject) {
        Buffer dest;

        if (!saveProject.m_stateSettings.isCompressed()) {
            dest = readRaw(m_filePath);
        } else {
            Buffer src;
            DataReader reader;

            try {
                src = DataReader::readFile(m_filePath);
                reader = DataReader(src.span(), Endian::Big);
                if (reader.size() < 12) {
                    return printf_err(FILE_ERROR, ERROR_5);
                }
            } catch (const std::exception& e) {
                return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
            }

            u32 src_size = reader.read<u32>() - 8;
            reader.read<i32>();
            u32 file_size = reader.read<u32>();

            if (!dest.allocate(file_size)) {
                return printf_err(MALLOC_FAILED, ERROR_1, file_size);
            }

            codec::XmemErr error = codec::XDecompress(dest.data(), dest.size_ptr(),
                                                      reader.ptr(), src_size);
            if (error != codec::XmemErr::Ok) {
                return printf_err(DECOMPRESS, "%s (%s)", ERROR_3, to_string(error));
            }

            if (dest.empty()) {
                return printf_err(DECOMPRESS, "%s", ERROR_3);
            }
        }

        int status = FileListing::readListing(saveProject, dest, m_console);
        return status;
    }


    int Xbox360DAT::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        printf("Xbox360DAT.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Xbox360DAT::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }


    std::optional<fs::path> Xbox360DAT::getFileInfoPath(SaveProject& saveProject) const {
        fs::path folderPath = m_filePath.parent_path();
        return folderPath / "__thumbnail.png";
    }
}