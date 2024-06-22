#include "fileListing.hpp"

#include "include/ghc/fs_std.hpp"
#include "include/sfo/sfo.hpp"

#include "LegacyEditor/utils/stringutils.hpp"
#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/RLE/rle_nsxps4.hpp"
#include "LegacyEditor/utils/dataManager.hpp"



namespace editor {


    fs::path FileListing::findExternalRegionNSXFolders() {

        // go from "231011215627.dat" to "231011215627.sub"
        fs::path mainDirPath = myFilePath;
        mainDirPath.replace_extension(".sub");
        if (!fs::is_directory(mainDirPath)) { return {}; }

        return mainDirPath;
    }


    int FileListing::writeExternalRegionsNSX(const fs::path& outDirPath) {
        printf("FileListing::writeExternalRegionsNSX: not implemented!");
        return NOT_IMPLEMENTED;
    }



}



