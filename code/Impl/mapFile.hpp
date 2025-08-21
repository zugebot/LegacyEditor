#pragma once

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"


namespace editor {

    class LCEFile;

    namespace map {


        MU void saveMapToPng(const LCEFile* map,
                             const fs::path& filename = "map_0.png");
    }
}

