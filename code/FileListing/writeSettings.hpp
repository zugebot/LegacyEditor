#pragma once

#include <utility>

#include "include/ghc/fs_std.hpp"

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"

#include "productcodes.hpp"


namespace editor {


    /**
     * The goal of the class is to enforce the user to give correct conversion settings,
     * such as:
     * 1. the console to output to,
     * 2. the folder to put the files in,
     * 3. product codes for different playstation consoles,
     * etc.
     */
    class MU WriteSettings {
        lce::CONSOLE myConsole;
        fs::path myInFolderPath;
        fs::path myOutFilePath;


    public:
        ProductCodes myProductCodes;
        /// im lazy, write getter + setter
        bool shouldRemovePlayers = true;
        /// im lazy, write getter + setter
        bool shouldRemoveDataMapping = true;
        /// im lazy, write getter + setter
        bool shouldRemoveMaps = true;


        WriteSettings() : myConsole(lce::CONSOLE::NONE) {}

        MU explicit WriteSettings(lce::CONSOLE theConsole)
            : myConsole(theConsole) {}

        MU WriteSettings(const lce::CONSOLE theConsole, fs::path theFilePath)
            : myConsole(theConsole), myInFolderPath(std::move(theFilePath)) {}
        
        
        MU WriteSettings(const lce::CONSOLE theConsole, const ePS3ProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myInFolderPath(std::move(theFilePath)) {
            myProductCodes.setPS3(thePCode);
        }
        
        MU WriteSettings(const lce::CONSOLE theConsole, const eVITAProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myInFolderPath(std::move(theFilePath)) {
            myProductCodes.setVITA(thePCode);
        }
        
        MU WriteSettings(const lce::CONSOLE theConsole, const ePS4ProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myInFolderPath(std::move(theFilePath)) {
            myProductCodes.setPS4(thePCode);
        }
        
        MU ND lce::CONSOLE getConsole() const { return myConsole; }

        MU ND fs::path getInFolderPath() const { return myInFolderPath; }

        MU ND fs::path getOutFilePath() const { return myOutFilePath; }

        MU void setOutFilePath(const fs::path& theOutFilePath) { myOutFilePath = theOutFilePath; }

        MU ND bool areSettingsValid() const {
            if (myConsole == lce::CONSOLE::PS3 && !myProductCodes.isVarSetPS3()) return false;
            if (myConsole == lce::CONSOLE::PS4 && !myProductCodes.isVarSetPS4()) return false;
            if (myConsole == lce::CONSOLE::VITA && !myProductCodes.isVarSetVITA()) return false;
            return true;
        }
    };




}


