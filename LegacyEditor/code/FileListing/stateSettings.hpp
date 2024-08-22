#pragma once

#include <utility>

#include "include/ghc/fs_std.hpp"

#include "lce/enums.hpp"
#include "lce/processor.hpp"


namespace editor {


    /**
     * .
     */
    class MU StateSettings {
        fs::path myFilePath;
        lce::CONSOLE myConsole = lce::CONSOLE::NONE;
        i32 myOldestVersion{};
        i32 myCurrentVersion{};

        bool isXbox360BIN = false;
        bool hasSepRegions = false;

    public:
        StateSettings() = default;
        MU explicit StateSettings(fs::path theFilePath) : myFilePath(std::move(theFilePath)) {}

        void reset() {
            myFilePath = "";
            myOldestVersion = 0;
            myCurrentVersion = 0;
            isXbox360BIN = false;
            hasSepRegions = false;
            myConsole = lce::CONSOLE::NONE;
        }

        MU void setConsole(lce::CONSOLE theConsole) { myConsole = theConsole; }
        MU ND lce::CONSOLE getConsole() const { return myConsole; }

        MU void setOldestVersion(i32 theVersion) { myOldestVersion = theVersion; }
        MU ND i32 getOldestVersion() const { return myOldestVersion; }

        MU void setCurrentVersion(i32 theVersion) { myCurrentVersion = theVersion; }
        MU ND i32 getCurrentVersion() const { return myCurrentVersion; }

        MU void setFilePath(fs::path theFilePath) { myFilePath = std::move(theFilePath); }
        MU ND fs::path getFilePath() const { return myFilePath; }

        MU void setIsXbox360BIN(bool theBool) { isXbox360BIN = theBool; }
        MU ND bool getIsXbox360BIN() const { return isXbox360BIN; }

        MU void setHasSepRegions(bool theBool) { hasSepRegions = theBool; }
        MU ND bool getHasSepRegions() const { return hasSepRegions; }


    };

}