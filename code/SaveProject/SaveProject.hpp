#pragma once

#include "code/ConsoleParser/helpers/detectConsole.hpp"
#include "code/ConsoleParser/helpers/makeParserForConsole.hpp"
#include "code/DisplayMetadata/DisplayMetadata.hpp"
#include "code/FileListing/fileListing.hpp"

namespace editor {

    class SaveProject {
    public:
        FileListing m_fileListing;
        DisplayMetadata m_displayMetadata;
        StateSettings m_stateSettings;


        int read(const fs::path& theFilePath);
        int write(WriteSettings& theWriteSettings);


        MU void printDetails() const;
        MU void printFileList() const;
        MU ND int dumpToFolder(const std::string& detail) const;
    };

} // namespace editor
