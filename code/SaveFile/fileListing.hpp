#pragma once

#include <ranges>

#include "include/lce/processor.hpp"
#include "common/data/ghc/fs_std.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "common/error_status.hpp"


namespace editor {
    class SaveProject;
    class WriteSettings;


    struct ListingFile {
        u32 fileIndex{};
        u32 fileSize{};

        std::string fileName;
        u64 timestamp{};
    };


    struct ListingHeader {
        u32 indexOffset{};
        u32 fileCount{};
        u16 oldestVersion{};
        u16 latestVersion{};
        u32 footerEntrySize = 144;

        explicit ListingHeader(DataReader& reader);


        void moveReaderToFileHeader(DataReader& reader, u32 fileIndex) const;
    };




    class FileListing {
    public:
        FileListing() = default;

        ND static std::vector<ListingFile> createListItems(DataReader& reader, ListingHeader& header);
        ND static bool isValid(DataReader& reader);
        ND static int readListing(SaveProject& saveProject, const Buffer& bufferIn, lce::CONSOLE consoleIn);
        ND static Buffer writeListing(SaveProject& saveProject, WriteSettings& writeSettings);


    };
}

