#pragma once

#include "LegacyEditor/utils/processor.hpp"


namespace editor {

    class File;

    namespace map {


        MU void saveMapToPng(const File* map,
                             const std::string& path,
                             const std::string& filename = "map_0.png");
    }
}

