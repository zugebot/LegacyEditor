#pragma once

#include <utility>

#include "include/ghc/fs_std.hpp"

#include "lce/enums.hpp"
#include "lce/processor.hpp"

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
    class MU ConvSettings {
        lce::CONSOLE myConsole;
        fs::path myFilePath;
    public:
        ProductCodes myProductCodes;


        ConvSettings() : myConsole(lce::CONSOLE::NONE) {}

        MU explicit ConvSettings(lce::CONSOLE theConsole)
            : myConsole(theConsole) {}

        MU ConvSettings(const lce::CONSOLE theConsole, fs::path theFilePath)
            : myConsole(theConsole), myFilePath(std::move(theFilePath)) {}
        
        
        MU ConvSettings(const lce::CONSOLE theConsole, const ePS3ProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myFilePath(std::move(theFilePath)) {
            myProductCodes.setPS3(thePCode);
        }
        
        MU ConvSettings(const lce::CONSOLE theConsole, const eVITAProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myFilePath(std::move(theFilePath)) {
            myProductCodes.setVITA(thePCode);
        }
        
        MU ConvSettings(const lce::CONSOLE theConsole, const ePS4ProductCode thePCode, fs::path theFilePath)
            : myConsole(theConsole), myFilePath(std::move(theFilePath)) {
            myProductCodes.setPS4(thePCode);
        }
        
        MU ND lce::CONSOLE getConsole() const { return myConsole; }

        MU ND fs::path getFilePath() const { return myFilePath; }

        MU ND bool areSettingsValid() const {
            if (myConsole == lce::CONSOLE::PS3 && myProductCodes.isVarSetPS3()) return false;
            if (myConsole == lce::CONSOLE::PS4 && myProductCodes.isVarSetPS4()) return false;
            if (myConsole == lce::CONSOLE::VITA && myProductCodes.isVarSetVITA()) return false;
            return true;
        }
    };




}


