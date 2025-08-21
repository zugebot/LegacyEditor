#include "Xbox360BIN.hpp"

#include "common/binfile/BINSupport.hpp"
#include "common/codec/XDecompress.hpp"
#include "common/utils.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/SaveFile/fileListing.hpp"
#include "code/SaveFile/writeSettings.hpp"


namespace editor {

    // TODO: IDK if it should but it is for now, get fileInfo out of it, fix memory leaks
    // TODO: I do not want to touch this (6/5/2025)
    int Xbox360BIN::inflateFromLayout(SaveProject& saveProject, const fs::path& theFilePath) {
        m_filePath = theFilePath;

        FILE *f_in = fopen(m_filePath.string().c_str(), "rb");
        if (f_in == nullptr) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        fseek(f_in, 0, SEEK_END);
        c_u64 input_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);
        if (input_size < 12) {
            return printf_err(FILE_ERROR, ERROR_5);
        }
        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, 12, f_in);

        Buffer bin(input_size);

        fseek(f_in, 0, SEEK_SET);
        fread(bin.data(), 1, bin.size(), f_in);

        DataReader binFile(bin.data(), bin.size());
        StfsPackage stfsInfo(binFile);
        stfsInfo.parse();
        StfsFileListing listing = stfsInfo.getFileListing();

        StfsFileEntry* entry = findSavegameFileEntry(listing);
        if (entry == nullptr) {
            std::cout << "Something really bad happened trying to parse xbox360.bin file?\n";
            bin.clear();
            return {};
        } else {
        }



        // stuff I need to figure out
        MU auto createdTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
        BINHeader* meta = stfsInfo.getMetaData();
        if (!meta->thumbnailImage.empty()) {
            saveProject.m_displayMetadata.read(meta->thumbnailImage, lce::CONSOLE::XBOX360);
        }
        saveProject.m_displayMetadata.worldName = stfsInfo.getMetaData()->displayName;

        const Buffer src = stfsInfo.extractFile(entry);
        DataReader reader(src.data(), src.size());
        c_u32 srcSize = reader.read<u32>() - 8;
        c_u32 inflatedSize = reader.read<u64>();
        bin.clear();


        Buffer dest;
        if (!dest.allocate(inflatedSize)) {
            return MALLOC_FAILED;
        }

        codec::XmemErr err = codec::XDecompress(
                dest.data(), dest.size_ptr(),
                reader.ptr(), srcSize);
        if (err != codec::XmemErr::Ok) {
            return DECOMPRESS;
        }

        int status = FileListing::readListing(saveProject, dest, m_console);
        if (status != 0) {
            return -1;
        }

        return SUCCESS;
    }


    int Xbox360BIN::inflateListing(MU SaveProject& saveProject) {
        return NOT_IMPLEMENTED;
    }


    ND int Xbox360BIN::deflateToSave(MU SaveProject& saveProject, MU WriteSettings& theSettings) const {
        printf("Xbox360BIN.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Xbox360BIN::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }


    std::optional<fs::path> Xbox360BIN::getFileInfoPath(SaveProject& saveProject) const {
        return std::nullopt;
    }
}