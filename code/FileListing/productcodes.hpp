#pragma once

#include "include/lce/processor.hpp"


namespace editor {

    /// An enum of PS3 product codes.
    enum class ePS3ProductCode : u8 {
        NONE,
        /// Europe (HDD)
        NPEB01899,
        /// USA (HDD)
        NPUB31419,
        /// Japan (HDD)
        NPJB00549,
        /// Europe (Disc)
        BLES01976,
        /// USA (Disc)
        BLUS31426,
    };

    MU static ePS3ProductCode PS3ProductCodeArray[6] = {
            ePS3ProductCode::NONE,
            ePS3ProductCode::NPEB01899,
            ePS3ProductCode::NPUB31419,
            ePS3ProductCode::NPJB00549,
            ePS3ProductCode::BLES01976,
            ePS3ProductCode::BLUS31426
    };

    /// An enum of PSVita product codes.
    enum class eVITAProductCode : u8 {
        NONE,
        /// USA (HDD)
        PCSE00491,
        /// Europe (HDD)
        PCSB00560,
        /// Japan (HDD)
        PCSG00302,
    };

    MU static eVITAProductCode PSVITAProductCodeArray[6] = {
            eVITAProductCode::NONE,
            eVITAProductCode::PCSE00491,
            eVITAProductCode::PCSB00560,
            eVITAProductCode::PCSG00302,
    };

    /// An enum of PS4 product codes.
    enum class ePS4ProductCode : u8 {
        NONE,
        /// USA
        CUSA00744,
        /// Japan
        CUSA00283,
        /// Europe
        CUSA00265,
    };


    class MU ProductCodes {
        MU ePS3ProductCode myPS3;
        MU eVITAProductCode myVita;
        MU ePS4ProductCode myPS4;

    public:
        ProductCodes() : myPS3(ePS3ProductCode::NONE), myVita(eVITAProductCode::NONE), myPS4(ePS4ProductCode::NONE) {}

        // getters and setters

        MU void setPS3(ePS3ProductCode thePCode) { myPS3 = thePCode; }
        MU void setVITA(eVITAProductCode thePCode) { myVita = thePCode; }
        MU void setPS4(ePS4ProductCode thePCode) { myPS4 = thePCode; }

        MU ND ePS3ProductCode getPS3() const { return myPS3; }
        MU ND eVITAProductCode getVITA() const { return myVita; }
        MU ND ePS4ProductCode getPS4() const { return myPS4; }

        MU ND bool isVarSetPS3() const { return myPS3 != ePS3ProductCode::NONE; }
        MU ND bool isVarSetVITA() const { return myVita != eVITAProductCode::NONE; }
        MU ND bool isVarSetPS4() const { return myPS4 != ePS4ProductCode::NONE; }

        // Helper functions

        MU static std::string toString(const ePS3ProductCode code) {
            switch (code) {
                case ePS3ProductCode::NPEB01899: return "NPEB01899";
                case ePS3ProductCode::NPUB31419: return "NPUB31419";
                case ePS3ProductCode::NPJB00549: return "NPJB00549";
                case ePS3ProductCode::BLES01976: return "BLES01976";
                case ePS3ProductCode::BLUS31426: return "BLUS31426";
                default: return "NONE";
            }
        }

        MU static ePS3ProductCode toPS3PCode(const std::string& str) {
            if (str == "NPEB01899") return ePS3ProductCode::NPEB01899;
            if (str == "NPUB31419") return ePS3ProductCode::NPUB31419;
            if (str == "NPJB00549") return ePS3ProductCode::NPJB00549;
            if (str == "BLES01976") return ePS3ProductCode::BLES01976;
            if (str == "BLUS31426") return ePS3ProductCode::BLUS31426;
            return ePS3ProductCode::NONE;
        }

        MU static std::string toString(const eVITAProductCode code) {
            switch (code) {
                case eVITAProductCode::PCSE00491: return "PCSE00491";
                case eVITAProductCode::PCSB00560: return "PCSB00560";
                case eVITAProductCode::PCSG00302: return "PCSG00302";
                default: return "NONE";
            }
        }

        MU static eVITAProductCode toVITAPCode(const std::string& str) {
            if (str == "PCSE00491") return eVITAProductCode::PCSE00491;
            if (str == "PCSB00560") return eVITAProductCode::PCSB00560;
            if (str == "PCSG00302") return eVITAProductCode::PCSG00302;
            return eVITAProductCode::NONE;
        }

        MU static std::string toString(const ePS4ProductCode code) {
            switch (code) {
                case ePS4ProductCode::CUSA00744: return "CUSA00744";
                case ePS4ProductCode::CUSA00283: return "CUSA00283";
                case ePS4ProductCode::CUSA00265: return "CUSA00265";
                default: return "NONE";
            }
        }

        MU static ePS4ProductCode toPS4PCode(const std::string& str) {
            if (str == "CUSA00744") return ePS4ProductCode::CUSA00744;
            if (str == "CUSA00283") return ePS4ProductCode::CUSA00283;
            if (str == "CUSA00265") return ePS4ProductCode::CUSA00265;
            return ePS4ProductCode::NONE;
        }

    };

}