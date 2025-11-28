#pragma once


#include "common/data/ghc/fs_std.hpp"
#include "data/GetExecutablePath.hpp"
#include "fmt.hpp"
#include "nlohmann/json.hpp"
#include <fstream>


using namespace cmn;

class LookerConfig {
public:

    static fs::path CURRENT_PATH;
    static constexpr std::string BASE_JSON = "looker.json";


    struct block {
        int id;
        std::vector<short> dataTags;

        bool filterOffDataTag;
        bool filterOffHasNBT;
    };

    struct item {
        std::string id;
        std::vector<short> damages;
        u8 count;

        bool filterOffDamage;
        bool filterOffCount;
    };

    struct {
        std::vector<block> blocks;
        std::vector<item> items;
    } self;



    void read(const std::string& fileName) {
        const fs::path exe_path = ExecutablePath::getExecutableDir();
        const fs::path configPath = exe_path / fileName;

        nlohmann::json jsonConfig;

        if (!fs::exists(configPath)) {
            log(eLog::warning, "No conversion.json found. Creating default...\n");
            createDefaultConfigFile(configPath);
            return;
        }

        std::ifstream in(configPath);
        try {
            in >> jsonConfig;
        } catch (const std::exception& e) {
            log(eLog::error, "Error reading conversion.json: {}\n", e.what());
            return;
        }

        debug = jsonConfig["debug"].get<bool>();
        const auto& input = jsonConfig["conversionInput"];
        const auto& output = jsonConfig["conversionOutput"];

        // Load input section
        conversionInput.autoInput          = input.value("autoInput", false);
        conversionInput.autoKey            = input.value("autoKey", "");
        conversionInput.autoSampleParamSFO = input.value("autoSampleParamSFO", "");

        // Load autoShortener
        if (input.contains("autoShortener")) {
            for (const auto& [k, v] : input["autoShortener"].items()) {
                conversionInput.autoShortener[k] = v.get<std::string>();
            }
        }


        // Load output section
        conversionOutput.autoOutput        = output.value("autoOutput", false);
        conversionOutput.autoConsole       = output.value("autoConsole", "");
        conversionOutput.autoPsVProductCode = output.value("autoPsVProductCode", "");
        conversionOutput.autoPs3ProductCode = output.value("autoPs3ProductCode", "");
        conversionOutput.autoPs4ProductCode = output.value("autoPs4ProductCode", "");

        // Load variables
        const auto& vars = output["variables"];
        conversionOutput.variables.removePlayers        = vars.value("removePlayers", false);
        conversionOutput.variables.removeMaps           = vars.value("removeMaps", false);
        conversionOutput.variables.removeStructures     = vars.value("removeStructures", false);
        conversionOutput.variables.removeRegionsOverworld = vars.value("removeRegionsOverworld", false);
        conversionOutput.variables.removeRegionsNether  = vars.value("removeRegionsNether", false);
        conversionOutput.variables.removeRegionsEnd     = vars.value("removeRegionsEnd", false);
        conversionOutput.variables.removeEntitiesDat    = vars.value("removeEntitiesDat", false);

        // Load conversion paths
        if (output.contains("path")) {
            for (const auto& [console, pathEntry] : output["path"].items()) {
                ConverterConfig::OutPath outPath;
                outPath.useCustomPath = pathEntry.value("useCustomPath", false);
                outPath.conversionPath = applyShorteners(pathEntry.value("conversionPath", ""));
                conversionOutput.paths[console] = outPath;
            }
        }
    }


    static void createDefaultConfigFile(const fs::path& configPath) {
        nlohmann::json jsonConfig;

        jsonConfig["conversionInput"] = {
                {"autoInput", true},
                {"autoKey", "pirates"},
                {"autoSampleParamSFO", ""},
                {"autoShortener", nlohmann::json::object()},
                {"autoPath", nlohmann::json::object()}
        };

        jsonConfig["conversionOutput"] = {
                {"autoOutput", true},
                {"autoConsole", "switch"},
                {"autoPsVProductCode", "PCSB00560"},
                {"autoPs3ProductCode", "NPUB31419"},
                {"autoPs4ProductCode", "CUSA00744"},
                {"variables", {
                                 {"removePlayers", true},
                                 {"removeMaps", true},
                                 {"removeStructures", true},
                                 {"removeRegionsOverworld", false},
                                 {"removeRegionsNether", true},
                                 {"removeRegionsEnd", true},
                                 {"removeEntitiesDat", true}
                              }},
                {"path", {
                                 {"xbox360", {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"ps3",     {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"rpcs3",   {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"vita",    {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"ps4",     {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"shadps4", {{"useDefaultPath", true},  {"conversionPath", ""}}},
                                 {"wiiu",    {{"useDefaultPath", false}, {"conversionPath", "{cemu_ely_}"}}},
                                 {"switch",  {{"useDefaultPath", false}, {"conversionPath", "{yuzu}"}}}
                         }}
        };

        std::ofstream out(configPath);
        out << jsonConfig.dump(4);
        log(eLog::success, "Created default config at {}\n", configPath.string());
    }



    ND fs::path getOutputPath(const std::string& consoleStr) const {
        const fs::path exe_path = ExecutablePath::getExecutableDir();
        fs::path defaultPath = exe_path / "out";

        auto it = conversionOutput.paths.find(consoleStr);
        if (it != conversionOutput.paths.end()) {
            const OutPath& pathConfig = it->second;
            if (pathConfig.useCustomPath && !pathConfig.conversionPath.empty()) {
                return pathConfig.conversionPath;
            }
        }
        return defaultPath;
    }
};
