#pragma once

#include "include/ghc/fs_std.hpp"

#include "include/lce/enums.hpp"
#include "include/lce/processor.hpp"

#include "code/ConsoleParser/productcodes.hpp"

#include "code/convert/Schematic.hpp"


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
        fs::path m_inFolderPath;
        fs::path m_outFilePath;

    public:
        std::string m_fileNameOut;
        ProductCodes m_productCodes;
        sch::Schematic m_schematic;
        fs::path m_paramSfoToReplace;

        bool removePlayers = true;
        bool removeDataMapping = true;
        bool removeMaps = true;
        bool removeStructures = true;
        bool removeRegionsOverworld = false;
        bool removeRegionsNether = false;
        bool removeRegionsEnd = false;
        bool removeEntities = true;


        WriteSettings() = delete;

        MU explicit WriteSettings(sch::Schematic sch, lce::CONSOLE theConsole = lce::CONSOLE::NONE)
            : m_schematic(sch) {
            m_schematic.setConsole(theConsole);
        }

        MU WriteSettings(sch::Schematic sch, const lce::CONSOLE theConsole, fs::path theFilePath)
            : m_schematic(sch), m_inFolderPath(std::move(theFilePath)) {
            m_schematic.setConsole(theConsole);
        }
        
        
        MU WriteSettings(sch::Schematic sch, const lce::CONSOLE theConsole, const ePS3ProductCode thePCode, fs::path theFilePath)
            : m_schematic(sch), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setPS3(thePCode);
            m_schematic.setConsole(theConsole);
        }
        
        MU WriteSettings(sch::Schematic sch, const lce::CONSOLE theConsole, const eVITAProductCode thePCode, fs::path theFilePath)
            : m_schematic(sch), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setVITA(thePCode);
            m_schematic.setConsole(theConsole);
        }
        
        MU WriteSettings(sch::Schematic sch, const lce::CONSOLE theConsole, const ePS4ProductCode thePCode, fs::path theFilePath)
            : m_schematic(sch), m_inFolderPath(std::move(theFilePath)) {
            m_productCodes.setPS4(thePCode);
            m_schematic.setConsole(theConsole);
        }

        MU ND fs::path getInFolderPath() const { return m_inFolderPath; }

        MU ND fs::path getOutFilePath() const { return m_outFilePath; }

        MU void setInFolderPath(const fs::path& theFolderPath) { m_inFolderPath = theFolderPath; }

        MU void setOutFilePath(const fs::path& theOutFilePath) { m_outFilePath = theOutFilePath; }

        MU void setSfoPath(const fs::path& sfo) { m_paramSfoToReplace = sfo; }

        MU ND bool areSettingsValid() const {
            if (m_schematic.save_console == lce::CONSOLE::PS3 && !m_productCodes.isVarSetPS3()) return false;
            if (m_schematic.save_console == lce::CONSOLE::PS4 && !m_productCodes.isVarSetPS4()) return false;
            if (m_schematic.save_console == lce::CONSOLE::VITA && !m_productCodes.isVarSetVITA()) return false;
            return true;
        }
    };
}
