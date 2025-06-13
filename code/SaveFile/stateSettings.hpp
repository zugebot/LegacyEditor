#pragma once

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"


namespace editor {


    class MU StateSettings {
        fs::path m_filePath;
        lce::CONSOLE m_console = lce::CONSOLE::NONE;
        bool m_isX360Bin = false;
        bool m_newGen = false;
        bool m_shouldDecompress = true;

    public:
        StateSettings() = default;
        MU explicit StateSettings(fs::path theFilePath)
            : m_filePath(std::move(theFilePath)) {}

        void reset() {
            m_filePath = "";
            m_isX360Bin = false;
            m_newGen = false;
            m_console = lce::CONSOLE::NONE;
        }

        MU ND lce::CONSOLE console() const { return m_console; }
        MU ND fs::path filePath() const { return m_filePath; }
        MU ND bool isXbox360Bin() const { return m_isX360Bin; }
        MU ND bool isNewGen() const { return m_newGen; }
        MU ND bool shouldDecompress() const { return m_shouldDecompress; }


        MU void setConsole(const lce::CONSOLE theConsole) { m_console = theConsole; }
        MU void setFilePath(fs::path theFilePath) { m_filePath = std::move(theFilePath); }
        MU void setXbox360Bin(c_bool theBool) { m_isX360Bin = theBool; }
        MU void setNewGen(c_bool theBool) { m_newGen = theBool; }
        MU void setShouldDecompress(c_bool theBool) { m_shouldDecompress = theBool; }


    };

}