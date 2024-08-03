#pragma once

#include <utility>

#include "include/ghc/fs_std.hpp"

#include "lce/enums.hpp"
#include "lce/processor.hpp"


namespace editor {


    /**
     * .
     */
    class MU ReadSettings {
        fs::path myFilePath;
        bool myIsXbox360BIN = false;

    public:
        ReadSettings() = default;
        explicit ReadSettings(fs::path theFilePath) : myFilePath(std::move(theFilePath)) {}

        MU void setFilePath(fs::path theFilePath) { myFilePath = std::move(theFilePath); }
        MU ND fs::path getFilePath() const { return myFilePath; }

        MU void setIsXbox360BIN(bool val) { myIsXbox360BIN = val; }
        MU ND bool getIsXbox360BIN() const { return myIsXbox360BIN; }
    };

}