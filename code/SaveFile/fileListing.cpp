#include "fileListing.hpp"

#include "common/nbt.hpp"
#include "common/fmt.hpp"

#include "writeSettings.hpp"
#include "SaveProject.hpp"


namespace editor {

    int FileListing::readListing(SaveProject& saveProject, const Buffer& bufferIn, lce::CONSOLE consoleIn) {
        static constexpr u32 WSTRING_SIZE = 64;
        MU static constexpr u32 FILELISTING_HEADER_SIZE = 12;

        DataReader reader(bufferIn.data(), bufferIn.size(), getConsoleEndian(consoleIn));

        // DataWriter::writeFile("listingIn.dat", bufferIn.span());

        c_u32 indexOffset = reader.read<u32>();
        u32 fileCount = reader.read<u32>();
        saveProject.setOldestVersion(reader.read<u16>());
        saveProject.setLatestVersion(reader.read<u16>());

        u32 FOOTER_ENTRY_SIZE = 144;
        if (saveProject.latestVersion() <= 1) {
            FOOTER_ENTRY_SIZE = 136;
            fileCount /= 136;
        }

        saveProject.m_allFiles.clear();

        MU u32 totalSize = 0;
        for (u32 fileIndex = 0; fileIndex < fileCount; fileIndex++) {

            reader.seek(indexOffset + fileIndex * FOOTER_ENTRY_SIZE);

            std::string fileName = reader.readWAsString(WSTRING_SIZE);

            std::string parsedFileName;
            for (int i = 0; i < fileName.size(); i++) {
                char ch = *(fileName.data() + i);
                if (ch == '\001') {
                    i += 2;
                }
                parsedFileName += ch;
            }
            fileName = parsedFileName;

            u32 fileSize = reader.read<u32>();
            c_u32 index = reader.read<u32>();
            u64 timestamp = 0;
            if (saveProject.latestVersion() > 1) {
                timestamp = reader.read<u64>();
            }
            totalSize += fileSize;

            reader.seek(index);

            fs::path filePath = saveProject.m_tempFolder / fileName;
            if (fs::path folderPath = filePath.parent_path();
                !folderPath.empty() && !fs::exists(folderPath)) {
                fs::create_directories(folderPath);
            }
            auto readSpan = reader.readSpan(fileSize);
            DataWriter::writeFile(filePath, readSpan);

            saveProject.m_allFiles.emplace_back(consoleIn, timestamp,
                                                saveProject.m_tempFolder,
                                                saveProject.m_tempFolder, fileName);
        }

        return SUCCESS;
    }


    Buffer FileListing::writeListing(SaveProject& saveProject, WriteSettings& writeSettings) {
        static constexpr u32 WSTRING_SIZE = 64;
        static constexpr u32 FILELISTING_HEADER_SIZE = 12;
        static const std::set<lce::FILETYPE> TYPES_TO_WRITE = {
                lce::FILETYPE::STRUCTURE,
                lce::FILETYPE::VILLAGE,
                lce::FILETYPE::DATA_MAPPING,
                lce::FILETYPE::MAP,
                lce::FILETYPE::OLD_REGION_NETHER,
                lce::FILETYPE::OLD_REGION_OVERWORLD,
                lce::FILETYPE::OLD_REGION_END,
                lce::FILETYPE::PLAYER,
                lce::FILETYPE::LEVEL,
                lce::FILETYPE::GRF,
                lce::FILETYPE::ENTITY_NETHER,
                lce::FILETYPE::ENTITY_OVERWORLD,
                lce::FILETYPE::ENTITY_END,
        };
        u32 FOOTER_ENTRY_SIZE = (saveProject.latestVersion() > 1) ? 144 : 136;
        u32 MULTIPLIER = (saveProject.latestVersion() > 1) ? 1 : 136;

        auto fileRange = saveProject.view_of(TYPES_TO_WRITE);
        auto consoleOut = writeSettings.m_schematic.save_console;

        // step 1: get the file count and size of all sub-files
        struct FileStruct {
            const LCEFile& file;
            Buffer buffer;
            u32 offset{};
        };

        std::list<FileStruct> fileStructs;

        u32 fileInfoOffset = FILELISTING_HEADER_SIZE;
        for (const editor::LCEFile& file: fileRange) {
            fileStructs.emplace_back(
                    file,
                    DataReader::readFile(file.path()),
                    fileInfoOffset
            );
            fileInfoOffset += fileStructs.back().buffer.size();
        }

        // step 2: find total binary size and create its data buffer
        c_u32 totalFileSize = fileInfoOffset + FOOTER_ENTRY_SIZE * fileStructs.size();
        DataWriter writer(totalFileSize, lce::getConsoleEndian(consoleOut));

        // step 3: write header
        writer.write<u32>(fileInfoOffset);
        writer.write<u32>(fileStructs.size() * MULTIPLIER);
        writer.write<u16>(saveProject.oldestVersion());
        writer.write<u16>(saveProject.latestVersion());

        // step 4: write each files data
        for (const auto& fileStruct : fileStructs) {
            writer.writeBytes(fileStruct.buffer.data(), fileStruct.buffer.size());
        }

        // step 5: write file metadata
        for (const auto& fileStruct : fileStructs) {
            std::string fileIterName = fileStruct.file.constructFileName();

            writer.writeWStringFromString(fileIterName, WSTRING_SIZE);

            writer.write<u32>(fileStruct.buffer.size());
            writer.write<u32>(fileStruct.offset);
            if (saveProject.latestVersion() > 1) {
                writer.write<u64>(fileStruct.file.m_timestamp);
            }
        }

        // DataWriter::writeFile("listingOut.dat", writer.span());

        return writer.take();
    }

}