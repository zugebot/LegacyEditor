#pragma once

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"

#include "code/ConsoleParser/productcodes.hpp"


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
        lce::CONSOLE m_console;
        fs::path m_inFolderPath;
        fs::path m_outFilePath;


    public:
        ProductCodes m_productCodes;

        bool shouldRemovePlayers = true;

        bool shouldRemoveDataMapping = true;

        bool shouldRemoveMaps = true;

        bool shouldRemoveStructures = true;

        bool shouldRemoveRegionsOverworld = false;

        bool shouldRemoveRegionsNether = false;

        bool shouldRemoveRegionsEnd = false;

        bool shouldRemoveEntities = true;


        WriteSettings() : m_console(lce::CONSOLE::NONE) {}

        MU explicit WriteSettings(lce::CONSOLE theConsole)
            : m_console(theConsole) {}

        MU WriteSettings(const lce::CONSOLE theConsole, fs::path theFilePath)
            : m_console(theConsole), m_inFolderPath(std::move(theFilePath)) {}
        
        
        MU WriteSettings(const lce::CONSOLE theConsole, const ePS3ProductCode thePCode, fs::path theFilePath)
            : m_console(theConsole), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setPS3(thePCode);
        }
        
        MU WriteSettings(const lce::CONSOLE theConsole, const eVITAProductCode thePCode, fs::path theFilePath)
            : m_console(theConsole), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setVITA(thePCode);
        }
        
        MU WriteSettings(const lce::CONSOLE theConsole, const ePS4ProductCode thePCode, fs::path theFilePath)
            : m_console(theConsole), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setPS4(thePCode);
        }
        
        MU ND lce::CONSOLE getConsole() const { return m_console; }

        MU ND fs::path getInFolderPath() const { return m_inFolderPath; }

        MU ND fs::path getOutFilePath() const { return m_outFilePath; }

        MU void setConsole(const lce::CONSOLE console) { m_console = console; }

        MU void setInFolderPath(const fs::path& theFolderPath) { m_inFolderPath = theFolderPath; }

        MU void setOutFilePath(const fs::path& theOutFilePath) { m_outFilePath = theOutFilePath; }

        MU ND bool areSettingsValid() const {
            if (m_console == lce::CONSOLE::PS3 && !m_productCodes.isVarSetPS3()) return false;
            if (m_console == lce::CONSOLE::PS4 && !m_productCodes.isVarSetPS4()) return false;
            if (m_console == lce::CONSOLE::VITA && !m_productCodes.isVarSetVITA()) return false;
            return true;
        }
    };




}


