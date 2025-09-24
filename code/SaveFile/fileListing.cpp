#include "fileListing.hpp"

#include "common/nbt.hpp"
#include "common/fmt.hpp"

#include "SaveProject.hpp"
#include "common/utils.hpp"
#include "writeSettings.hpp"


namespace editor {

    ListingHeader::ListingHeader(DataReader& reader) {

        indexOffset = reader.read<u32>();
        fileCount = reader.read<u32>();
        oldestVersion = reader.read<u16>();
        latestVersion = reader.read<u16>();

        if (latestVersion <= 1) {
            footerEntrySize = 136;
            fileCount /= 136;
        }
    }


    void ListingHeader::moveReaderToFileHeader(DataReader& reader, uint32_t fileIndex) const {
        reader.seek(indexOffset + fileIndex * footerEntrySize);
    }


    // We love AI code
    bool FileListing::isValid(DataReader& reader) {
        // Preserve caller state
        const u64 savedPos = reader.tell();
        auto restore = [&] { reader.seek(savedPos); };

        // ---- Parse raw header (no helper structs; handle old/new formats uniformly) ----
        reader.seek(0);
        static constexpr u32 FILELISTING_HEADER_SIZE = 12; // 4(indexOffset)+4(rawCount)+2(oldest)+2(latest)

        if (reader.size() < FILELISTING_HEADER_SIZE) { restore(); return false; }

        const u32 indexOffsetRaw    = reader.read<u32>();
        const u32 rawFileCountField = reader.read<u32>(); // bytes for v<=1, count for v>=2
        const u16 oldestVersion     = reader.read<u16>();
        const u16 latestVersion     = reader.read<u16>();

        // ---- Version sanity ----
        if (oldestVersion > 13 || latestVersion > 13 || latestVersion < oldestVersion) {
            restore(); return false;
        }

        // ---- Normalize count / footer entry size and verify the "* 136" semantics for old listings ----
        const bool isOld = (latestVersion <= 1);
        const u32 footerEntrySize = isOld ? 136u : 144u;

        u32 normalizedCount = 0;
        if (isOld) {
            // In old listings, the header stores "footer byte size" (= count * 136)
            if (rawFileCountField == 0u || (rawFileCountField % 136u) != 0u) {
                restore(); return false;
            }
            normalizedCount = rawFileCountField / 136u;
        } else {
            // New listings store an actual count
            normalizedCount = rawFileCountField;
        }

        if (normalizedCount > 32768u) { restore(); return false; }

        // ---- Buffer/region math (use 64-bit for safety) ----
        const u64 bufSize   = static_cast<u64>(reader.size());
        const u64 idxOffset = static_cast<u64>(indexOffsetRaw);
        const u64 footBytes = static_cast<u64>(normalizedCount) * static_cast<u64>(footerEntrySize);

        // indexOffset must be within buffer and after header
        if (idxOffset < FILELISTING_HEADER_SIZE || idxOffset > bufSize) {
            restore(); return false;
        }
        // Entire footer must fit
        if (idxOffset + footBytes > bufSize) {
            restore(); return false;
        }
        // For zero files, enforce canonical layout (header immediately followed by footer)
        if (normalizedCount == 0u && idxOffset != FILELISTING_HEADER_SIZE) {
            restore(); return false;
        }

        // ---- Deep footer validation: ensure each payload range is within [12, indexOffset) ----
        struct Range { u64 off, end; }; // [off, end)
        std::vector<Range> ranges;
        ranges.reserve(normalizedCount);

        // Fixed-size name field: 64 UTF-16LE chars (2 bytes each) = 128 bytes
        static constexpr u32 kNameChars = 64;
        static constexpr u32 kNameBytes = kNameChars * 2; // 128
        const u32 kRemainingAfterName  = footerEntrySize - kNameBytes; // 8 (old) or 16 (new)

        for (u32 i = 0; i < normalizedCount; ++i) {
            const u64 entryPos = idxOffset + static_cast<u64>(i) * static_cast<u64>(footerEntrySize);

            // Whole entry must fit (paranoia guard)
            if (entryPos + footerEntrySize > bufSize) {
                restore(); return false;
            }

            // ---- Read fixed-size footer entry ----
            reader.seek(entryPos);

            // Always consume exactly 128 bytes for the UTF-16 name field.
            // (Do NOT use a function that stops on NUL; this field is fixed-length.)
            if (!reader.canRead(kNameBytes)) { restore(); return false; }
            reader.seek(reader.tell() + kNameBytes);

            // Now only the remaining per-entry bytes need to be available:
            // old: size(4)+ofs(4) => 8 bytes; new: size(4)+ofs(4)+timestamp(8) => 16 bytes.
            if (!reader.canRead(kRemainingAfterName)) { restore(); return false; }

            const u32 fileSize32 = reader.read<u32>();
            const u32 fileOfs32  = reader.read<u32>();
            if (!isOld) { (void)reader.read<u64>(); } // timestamp in v>=2

            const u64 fileOffset = static_cast<u64>(fileOfs32);
            const u64 fileSize   = static_cast<u64>(fileSize32);

            // ---- Validate the data range for this file ----
            if (fileOffset < FILELISTING_HEADER_SIZE || fileOffset >= idxOffset) {
                restore(); return false; // must be in data region [12, idxOffset)
            }

            const u64 fileEnd = fileOffset + fileSize;
            if (fileEnd < fileOffset) { restore(); return false; } // overflow
            if (fileEnd > idxOffset)  { restore(); return false; } // would overlap footer
            if (fileEnd > bufSize)    { restore(); return false; } // hard cap

            ranges.push_back({fileOffset, fileEnd});
        }

        restore();
        return true;
    }



    std::vector<ListingFile> FileListing::createListItems(DataReader& reader, ListingHeader& header) {
        static constexpr u32 WSTRING_SIZE = 64;

        std::vector<ListingFile> listItems;

        MU u32 totalSize = 0;
        for (u32 fileIndex = 0; fileIndex < header.fileCount; fileIndex++) {
            ListingFile& file = listItems.emplace_back();

            header.moveReaderToFileHeader(reader, fileIndex);


            std::string tempFileName = reader.readWAsString(WSTRING_SIZE);
            std::string parsedFileName;
            for (int i = 0; i < tempFileName.size(); i++) {
                char ch = *(tempFileName.data() + i);
                if (ch == '\001') {
                    i += 2;
                }
                parsedFileName += ch;
            }
            file.fileName = parsedFileName;

            file.fileSize = reader.read<u32>();
            file.fileIndex = reader.read<u32>();
            file.timestamp = 0;
            if (header.latestVersion > 1) {
                file.timestamp = reader.read<u64>();
            }
        }

        return listItems;
    }


    int FileListing::readListing(SaveProject& saveProject, const Buffer& bufferIn, lce::CONSOLE consoleIn) {
        saveProject.m_allFiles.clear();

        Endian end = getConsoleEndian(consoleIn);
        DataReader reader(bufferIn.data(), bufferIn.size(), end);

        // garbage to dynamically determine if it's little or big endian
        reader.seek(0);
        reader.setEndian(Endian::Big);
        bool isValidBig = editor::FileListing::isValid(reader);

        reader.seek(0);
        reader.setEndian(Endian::Little);
        bool isValidLittle = editor::FileListing::isValid(reader);

        if (isValidBig) {
            reader.setEndian(Endian::Big);
            if (saveProject.m_stateSettings.console() == lce::CONSOLE::NEWGENMCS) {
                saveProject.m_stateSettings.setConsole(lce::CONSOLE::NEWGENMCS_BIG);
                consoleIn = lce::CONSOLE::NEWGENMCS_BIG;
            }
        } else if (isValidLittle) {
            reader.setEndian(Endian::Little);
        } else {
            return INVALID_SAVE;
        }

        reader.seek(0);
        ListingHeader header(reader);
        std::vector<ListingFile> files = createListItems(reader, header);


        for (auto& file : files) {
            reader.seek(file.fileIndex);

            fs::path filePath = saveProject.m_tempFolder / file.fileName;
            if (fs::path folderPath = filePath.parent_path();
                !folderPath.empty() && !fs::exists(folderPath)) {
                fs::create_directories(folderPath);
            }
            auto readSpan = reader.readSpan(file.fileSize);
            DataWriter::writeFile(filePath, readSpan);

            bool forceOldRegion = consoleIn != lce::CONSOLE::NEWGENMCS && consoleIn != lce::CONSOLE::NEWGENMCS_BIG;
            auto& front = saveProject.m_allFiles.emplace_back(consoleIn, file.timestamp,
                                                saveProject.m_tempFolder,
                                                saveProject.m_tempFolder, file.fileName, forceOldRegion);
            if (consoleIn == lce::CONSOLE::NEWGENMCS || consoleIn == lce::CONSOLE::NEWGENMCS_BIG) {
                front.setIsMCSRegion(true);
            }
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

        return writer.take();
    }



}