#pragma once

#include "LegacyEditor/code/FileListing/fileListing.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/utils/dataManager.hpp"

#include "headerUnion.hpp"

namespace editor {
    class FileListing;
}

class ConsoleParser {
    static constexpr u32 WSTRING_SIZE = 64;
    static constexpr u32 FILELISTING_HEADER_SIZE = 12;


public:
    lce::CONSOLE myConsole;
    fs::path myFilePath;

    virtual ~ConsoleParser() = default;

    ND virtual int read(editor::FileListing* theListing, const fs::path& inFilePath) = 0;
    ND virtual int write(editor::FileListing* theListing, editor::WriteSettings& theSettings) const = 0;

protected:
    mutable editor::FileListing* myListingPtr;

    ND virtual int inflateListing() = 0;
    ND virtual int deflateListing(const fs::path& gameDataPath, Data& inflatedData, Data& deflatedData) const = 0;


    ND int readListing(const Data &dataIn);
    ND Data writeListing(const lce::CONSOLE consoleOut) const;

    virtual void readFileInfo() const;
    // writeFileInfo...

    /// This function is used by Switch and PS4.
    int readExternalFolder(const fs::path& inDirPath);
    // writeExternalFolder...

};
