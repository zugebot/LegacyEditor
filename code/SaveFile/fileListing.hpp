#pragma once

#include <ranges>

#include "include/lce/processor.hpp"
#include "common/data/ghc/fs_std.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "common/error_status.hpp"


namespace editor {
    class SaveProject;
    class WriteSettings;

    class FileListing {
    public:
        FileListing() = default;

        ND static int readListing(SaveProject& saveProject, const Buffer& bufferIn, lce::CONSOLE consoleIn);
        ND static Buffer writeListing(SaveProject& saveProject, WriteSettings& writeSettings);


    };
}

