#pragma once

#include "../../common/data/DataReader.hpp"
#include "../../common/data/DataWriter.hpp"

#include "SaveLayout.hpp"
#include "code/ConsoleParser/helpers/headerUnion.hpp"

namespace editor {
    class SaveProject;
    class WriteSettings;

    class ConsoleParser {
    public:
        lce::CONSOLE m_console;
        fs::path m_filePath;

        virtual ~ConsoleParser() = default;

        ND virtual SaveLayout discoverSaveLayout(const fs::path& rootFolder) = 0;
        ND virtual int inflateFromLayout(SaveProject& theSave, const fs::path& inFilePath) = 0;

        ND virtual int deflateToSave(SaveProject& saveProject, WriteSettings& theSettings) const = 0;
        virtual void supplyRequiredDefaults(SaveProject& saveProject) const = 0;

    protected:
        static Buffer readRaw(fs::path& inFilePath);

        ND virtual int inflateListing(SaveProject& saveProject) = 0;
        ND virtual int deflateListing(const fs::path& gameDataPath, Buffer& inflatedData, Buffer& deflatedData) const = 0;

        ND virtual std::optional<fs::path> getFileInfoPath(SaveProject& saveProject) const = 0;
        void readFileInfo(SaveProject& saveProject) const;
        static void defaultFileInfo(SaveProject& saveProject);
    };
}
