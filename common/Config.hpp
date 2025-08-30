#pragma once


#include "fmt.hpp"
#include "common/data/ghc/fs_std.hpp"
#include "nlohmann/json.hpp"
#include <fstream>


using namespace cmn;

class ConverterConfig {
public:

    static fs::path CURRENT_PATH;

    struct OutPath {
        bool useDefaultPath{};
        std::string conversionPath;
    };

    bool debug;

    struct {
        bool autoInput;
        std::string autoKey;
        std::string autoSampleParamSFO;
        std::map<std::string, std::string> autoShortener;
        std::map<std::string, std::string> autoPath;
    } conversionInput;

    struct {
        bool autoOutput;
        std::string autoConsole;
        std::string autoPsVProductCode;
        std::string autoPs3ProductCode;
        std::string autoPs4ProductCode;
        struct {
            bool removePlayers;
            bool removeMaps;
            bool removeStructures;
            bool removeRegionsOverworld;
            bool removeRegionsNether;
            bool removeRegionsEnd;
            bool removeEntitiesDat;
        } variables;
        std::map<std::string, OutPath> paths;
    } conversionOutput;

    void read(const std::string& fileName) {
        fs::path configPath = fs::path(EXE_CURRENT_PATH) / fileName;

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

        // Apply shorteners to autoPath values
        auto applyShorteners = [&](const std::string& input) -> std::string {
            std::string result = input;
            for (const auto& [key, val] : conversionInput.autoShortener) {
                std::string token = "{" + key + "}";
                size_t pos = 0;
                while ((pos = result.find(token, pos)) != std::string::npos) {
                    result.replace(pos, token.length(), val);
                    pos += val.length();
                }
            }
            return result;
        };

        if (input.contains("autoPath")) {
            for (const auto& [k, v] : input["autoPath"].items()) {
                std::string path = v.get<std::string>();
                conversionInput.autoPath[k] = applyShorteners(path);
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
                outPath.useDefaultPath = pathEntry.value("useDefaultPath", true);
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
                                 {"xbox360", {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"ps3",     {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"rpcs3",   {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"vita",    {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"ps4",     {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"shadps4", {{"useDefaultPath", true}, {"conversionPath", ""}}},
                                 {"wiiu",    {{"useDefaultPath", false}, {"conversionPath", "{cemu_ely_}"}}},
                                 {"switch",  {{"useDefaultPath", false}, {"conversionPath", "{yuzu}"}}}
                         }}
        };

        std::ofstream out(configPath);
        out << jsonConfig.dump(4);
        log(eLog::success, "Created default config at {}\n", configPath.string());
    }



    ND fs::path getOutputPath(const std::string& consoleStr) const {
        fs::path defaultPath = fs::path(EXE_CURRENT_PATH) / "out";

        auto it = conversionOutput.paths.find(consoleStr);
        if (it != conversionOutput.paths.end()) {
            const OutPath& pathConfig = it->second;
            if (!pathConfig.useDefaultPath && !pathConfig.conversionPath.empty()) {
                return pathConfig.conversionPath;
            }
        }
        return defaultPath;
    }
};
