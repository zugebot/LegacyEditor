#pragma once

#include <utility>

#include "lce/enums.hpp"
#include "lce/processor.hpp"

#include "include/ghc/fs_std.hpp"




namespace editor::productcode {

    enum class PS3 : u8 {
        NONE,
        NPEB01899, // Europe (HDD)
        NPUB31419, // USA (HDD)
        NPJB00549, // Japan (HDD)
        BLES01976, // Europe (Disc)
        BLUS31426, // USA (Disc)
    };


    enum class VITA : u8 {
        NONE,
        PCSE00491, // USA (HDD)
        PCSB00560, // Europe (HDD)
        PCSG00302, // Japan (HDD)
    };


    enum class PS4 : u8 {
        NONE,
        CUSA00744, // USA
        CUSA00283, // Japan
        CUSA00265, // Europe
    };

}




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

        struct ProductCodes {
            productcode::PS3 PS3;
            productcode::PS4 PS4;
            productcode::VITA Vita;

            ProductCodes() {
                PS3 = productcode::PS3::NONE;
                PS4 = productcode::PS4::NONE;
                Vita = productcode::VITA::NONE;
            }
        } myProductCodes;
    public:


        ConvSettings() : myConsole(lce::CONSOLE::NONE) {
            myProductCodes = ProductCodes();
        }

        MU explicit ConvSettings(lce::CONSOLE theConsole)
            : myConsole(theConsole) {
            myProductCodes = ProductCodes();
        }

        MU ConvSettings(lce::CONSOLE theConsole, fs::path theFilePath)
            : myConsole(theConsole), myFilePath(std::move(theFilePath)) {
            myProductCodes = ProductCodes();
        }

        MU ND lce::CONSOLE getConsole() const {
            return myConsole;
        }

        MU ND fs::path getFilePath() const {
            return myFilePath;
        }


        MU ND bool areSettingsValid() const {

            if (myConsole == lce::CONSOLE::PS3
                && myProductCodes.PS3 == productcode::PS3::NONE) {
                return false;
            }

            if (myConsole == lce::CONSOLE::PS4
                && myProductCodes.PS4 == productcode::PS4::NONE) {
                return false;
            }

            if (myConsole == lce::CONSOLE::VITA
                && myProductCodes.Vita == productcode::VITA::NONE) {
                return false;
            }

            return true;
        }

    };




}


