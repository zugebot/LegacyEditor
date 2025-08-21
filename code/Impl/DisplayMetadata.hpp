#pragma once

#include "include/lce/processor.hpp"
#include "include/lce/enums.hpp"

#include "common/data/ghc/fs_std.hpp"

#include "../../common/data/DataReader.hpp"
#include "../../common/data/DataWriter.hpp"
#include "../../common/data/buffer.hpp"
#include "include/lce/include/picture.hpp"


namespace editor {

    class DisplayMetadata {
    public:
        std::wstring worldName;
        Buffer thumbnail;
        // TODO: should not be here
        Picture icon0png;
        i64 seed{};
        i64 loads{};
        i64 hostOptions{};
        i64 texturePack{};
        std::string extraData{};
        i64 exploredChunks{};
        bool isLoaded = false;

        void defaultSettings();

        bool read(Buffer& buffer, lce::CONSOLE c);
        bool readHeader(DataReader& reader, lce::CONSOLE c);
        bool readPNG(DataReader& reader, lce::CONSOLE c);

        Buffer write(lce::CONSOLE c);
        void writeHeader(DataWriter& writer, lce::CONSOLE c) const;
        void writePNG(DataWriter& writer, lce::CONSOLE c);


        int readCacheFile(const fs::path& inFilePath, const std::string& folderName);
        MU void loadFileAsThumbnail(const fs::path& inFilePath);



    private:
    };

}
