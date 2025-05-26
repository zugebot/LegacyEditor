#pragma once

#include "include/lce/processor.hpp"


namespace editor {

    enum class ePS3ProductCode : uint8_t {
        NONE,
        NPUB31419, ///< USA (HDD)
        NPEB01899, ///< Europe (HDD)
        NPJB00549, ///< Japan (HDD)
        BLUS31426, ///< USA (Disc)
        BLES01976, ///< Europe (Disc)
    };

    enum class eVITAProductCode : uint8_t {
        NONE,
        PCSE00491, ///< USA (HDD)
        PCSB00560, ///< Europe (HDD)
        PCSG00302, ///< Japan (HDD)
    };

    enum class ePS4ProductCode : uint8_t {
        NONE,
        CUSA00744, /// USA
        CUSA00265, /// Europe
        CUSA00283, /// Japan
    };

    struct ProductCodeInfo {
        std::string_view name;
        std::string_view region;
        std::string_view storageType;
    };

    template <typename EnumType>
    class EnumMapper {
    public:
        EnumMapper(std::initializer_list<std::pair<EnumType, ProductCodeInfo>> list) {
            for (const auto& [key, info] : list) {
                enumToInfo[key] = info;
                nameToEnum[info.name] = key;
            }
        }

        ND size_t size() const {
            return enumToInfo.size();
        }

        std::optional<ProductCodeInfo> getInfo(EnumType code) const {
            auto it = enumToInfo.find(code);
            return it != enumToInfo.end() ? std::optional<ProductCodeInfo>{it->second} : std::nullopt;
        }

        void printOptions() const {
            int index = 1;
            for (const auto& [code, info] : enumToInfo) {
                if (code != EnumType::NONE) {
                    std::cout << "  [" << index << "] " << info.name
                              << " | " << info.region
                              << " | " << info.storageType << "\n";
                    ++index;
                }
            }
        }

        ND std::optional<EnumType> selectOption(int index) const {
            int currentIndex = 1;
            for (const auto& [code, info] : enumToInfo) {
                if (code != EnumType::NONE) {
                    if (currentIndex == index) {
                        return code;
                    }
                    currentIndex++;
                }
            }
            return std::nullopt;
        }

        std::optional<EnumType> fromString(std::string_view name) const {
            auto it = nameToEnum.find(name);
            return it != nameToEnum.end() ? std::optional<EnumType>{it->second}
                                          : std::nullopt;
        }

        std::string toString(EnumType code) const {
            auto it = enumToInfo.find(code);
            if (it != enumToInfo.end()) return std::string(it->second.name);
            return "NONE";
        }

    private:
        std::unordered_map<EnumType, ProductCodeInfo> enumToInfo;
        std::unordered_map<std::string_view, EnumType> nameToEnum;
    };

    inline const EnumMapper<ePS3ProductCode> PS3Mapper = {
            {ePS3ProductCode::NONE, {"NONE", "N/A", "N/A"}},
            {ePS3ProductCode::NPEB01899, {"NPEB01899", "Europe", "HDD"}},
            {ePS3ProductCode::NPUB31419, {"NPUB31419", "USA", "HDD"}},
            {ePS3ProductCode::NPJB00549, {"NPJB00549", "Japan", "HDD"}},
            {ePS3ProductCode::BLES01976, {"BLES01976", "Europe", "Disc"}},
            {ePS3ProductCode::BLUS31426, {"BLUS31426", "USA", "Disc"}}
    };

    inline const EnumMapper<eVITAProductCode> VITAMapper = {
            {eVITAProductCode::NONE, {"NONE", "N/A", "N/A"}},
            {eVITAProductCode::PCSE00491, {"PCSE00491", "USA", "HDD"}},
            {eVITAProductCode::PCSB00560, {"PCSB00560", "Europe", "HDD"}},
            {eVITAProductCode::PCSG00302, {"PCSG00302", "Japan", "HDD"}}
    };

    inline const EnumMapper<ePS4ProductCode> PS4Mapper = {
            {ePS4ProductCode::NONE, {"NONE", "N/A", "N/A"}},
            {ePS4ProductCode::CUSA00744, {"CUSA00744", "USA", "HDD"}},
            {ePS4ProductCode::CUSA00283, {"CUSA00283", "Japan", "HDD"}},
            {ePS4ProductCode::CUSA00265, {"CUSA00265", "Europe", "HDD"}}
    };


    class ProductCodes {
        ePS3ProductCode ps3 = ePS3ProductCode::NONE;
        eVITAProductCode vita = eVITAProductCode::NONE;
        ePS4ProductCode ps4 = ePS4ProductCode::NONE;

    public:
        void setPS3(const ePS3ProductCode code) { ps3 = code; }
        void setVITA(const eVITAProductCode code) { vita = code; }
        void setPS4(const ePS4ProductCode code) { ps4 = code; }

        MU ND ePS3ProductCode getPS3() const { return ps3; }
        MU ND eVITAProductCode getVITA() const { return vita; }
        MU ND ePS4ProductCode getPS4() const { return ps4; }

        bool isVarSetPS3() const { return ps3 != ePS3ProductCode::NONE; }
        bool isVarSetVITA() const { return vita != eVITAProductCode::NONE; }
        bool isVarSetPS4() const { return ps4 != ePS4ProductCode::NONE; }
    };

}