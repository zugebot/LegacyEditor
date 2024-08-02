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
    ND virtual int write(editor::FileListing* theListing, const editor::ConvSettings& theSettings) const = 0;

protected:

    ND virtual int deflateListing(editor::FileListing* theListing) = 0;
    ND virtual int inflateListing(const fs::path& gameDataPath, const Data& deflatedData, Data& inflatedData) const = 0;


    ND int readListing(editor::FileListing* theListing, const Data &dataIn);
    ND Data writeListing(editor::FileListing* theListing, const lce::CONSOLE consoleOut) const;

    ND virtual int readFileInfo(editor::FileListing* theListing) const;
    // TODO: put writeFileInfo here

    /// This function is used by Switch and PS4.
    static int readExternalFolder(editor::FileListing* theListing, const fs::path& inDirPath);

};
