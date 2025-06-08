#include <iostream>

#include "include/ghc/fs_std.hpp"
#include "include/nlohmann/json.hpp"
#include "include/lce/processor.hpp"

#include "common/windows/force_utf8.hpp"
#include "common/fmt.hpp"
#include "common/timer.hpp"

#include "code/scripts.hpp"
#include "code/include.hpp"

#include "code/DisplayMetadata/CacheBinManager.hpp"


#include "include/grf/grfReader.hpp"


using namespace cmn;


void consumeEnter() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


static int getNumberFromUser(const std::string& prompt, int min, int max) {
    int selection;
    while (true) {
        log(eLog::input, "{}", prompt);
        std::cin >> selection;
        if (selection >= min && selection <= max) {
            return selection;
        }
        log(eLog::error, "Invalid selection. Try again.\n");
    }
}


template<typename EnumType>
EnumType selectProductCode(const editor::EnumMapper<EnumType>& mapper, const std::string& consoleName) {
    log("Select a \"{}\" product code:\n", consoleName);
    mapper.printOptions();
    int choice = getNumberFromUser("Enter index: ", 1,
                                   static_cast<int>(mapper.size() - 1));

    auto selectedCode = mapper.selectOption(choice);
    if (selectedCode.has_value()) {
        auto info = mapper.getInfo(selectedCode.value());
        log(eLog::success, "Selected: {}\n", info->name);
        return selectedCode.value();
    }

    return EnumType::NONE;
}


fs::path getOutputPath(const nlohmann::json& jsonConfig, const std::string& consoleStr, const fs::path& defaultPath) {

    auto outputConfig = jsonConfig.value("conversionOutput", nlohmann::json::object());
    auto outputPath = outputConfig.value("path", nlohmann::json::object());

    if (outputPath.contains(consoleStr)) {
        const auto& consoleConfig = outputPath[consoleStr];
        bool useDefault = consoleConfig.value("useDefaultPath", true);
        if (useDefault) {
            return defaultPath;
        } else {
            return consoleConfig.value("conversionPath", defaultPath.string());
        }
    }

    return defaultPath;
}


void handleRemovalOption(const std::string& prompt, bool& setting) {
    log(eLog::input, "{} (y/n): ", prompt);
    char choice = 'y';
    std::cin >> choice;
    consumeEnter();
    setting = (choice == 'y' || choice == 'Y');
    // log(setting ? eLog::warning : eLog::info,
    //          setting ? "Data will be removed.\n"
    //                  : "Data will NOT be removed.\n");
}


int main(int argc, char* argv[]) {
#ifdef _WIN32
        force_utf8_console();
#endif


        // fs::path cachePath = "E:\\Emulators\\Vita3K\\LOL\\ux0\\user\\00\\savedata\\PCSE00491\\CACHE.bin";
        // CacheBinManager manager;
        // manager.load(cachePath);


        /*
    std::string entitiesFile = "C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\dump\\250524034012_ps4__0\\DIM1\\entities.dat";
    Buffer buffer = DataReader::readFile(entitiesFile);
    DataReader reader(buffer.span());

    int count = reader.read<i32>();
    std::vector<NBTList> nbtList(count);
    for (int i = 0; i < count; i++) {
        int x = reader.read<i32>();
        int z = reader.read<i32>();
        NBTBase nbt = NBTBase::read(reader);
        nbtList[i] = nbt.get<NBTCompound>().extract("Entities")
                        .value_or(makeList(eNBT::COMPOUND)).get<NBTList>();
    }
     */

        log(eLog::detail,
            "Find the project here! https://github.com/zugebot/LegacyEditor\n\n");
        log(eLog::detail,
            "Supports reading  [ Xbox360, PS3, RPCS3, PSVITA, PS4, WiiU/Cemu, Switch, Windurango ]\n");
        log(eLog::detail,
            "Supports writing  [ -------  ---  RPCS3, PSVITA, ---  WiiU/Cemu  ------, ---------- ]\n\n");

        fs::path exePath = fs::path(argv[0]).parent_path();
        fs::path defaultOutDir = exePath / "out";

        // Read conversion.json
        nlohmann::json jsonConfig;
        fs::path configPath = exePath / "conversion.json";
        if (fs::exists(configPath)) {
            std::ifstream in(configPath);
            try {
                in >> jsonConfig;
            } catch (const std::exception& e) {
                log(eLog::error, "Error reading conversion.json: {}\n", e.what());
            }
        } else {
            log(eLog::warning, "No conversion.json found. Using default paths.\n");
        }


        std::vector<std::string> saveFileArgs;


        auto inputConfig = jsonConfig.value("conversionInput", nlohmann::json::object());

        bool autoInput = inputConfig.value("autoInput", false);
        if (autoInput) {
            log(eLog::input, "Reading auto input from \"configuration.json\"\n");

            std::string idx = inputConfig.value("autoKey", "0");
            std::string path = inputConfig["autoPath"].value(idx, "");
            if (!inputConfig["autoPath"].contains(idx) || path.empty()) {

                log(eLog::error,
                    "Invalid input conversionInput.autoPath[{}]\n", idx);
                return -1;
            }
            saveFileArgs.push_back(path);

        } else {
            if (argc < 2) {
                log(eLog::error, "Must supply at least one save file to convert.\n");
                log(eLog::info, "Drag & drop the GAMEDATA/SAVEGAME/.dat/.bin on the executable or pass as arguments.\n");
                log(eLog::info, "More information can be found here: jerrin.org/links/lceditdoc/\n");
                log(eLog::info, "Or contact \"jerrinth\" on discord.\n");
                log(eLog::input, "Press ENTER to exit.\n");
                consumeEnter();
                return -1;
            }
            for (int i = 1; i < argc; i++) {
                saveFileArgs.emplace_back(argv[i]);
            }
        }

        lce::CONSOLE consoleOutput;
        editor::WriteSettings writeSettings;

        auto outputConfig = jsonConfig.value("conversionOutput", nlohmann::json::object());

        bool autoOutput = outputConfig.value("autoOutput", false);
        if (autoOutput) {
            std::string jsonConsole = outputConfig["autoConsole"];
            consoleOutput = lce::strToConsole(jsonConsole);
            if (consoleOutput == lce::CONSOLE::NONE) {
                log(eLog::error, "Invalid json conversionOutput.autoConsole\n");
                return -1;
            } else {
                log(eLog::input, "Using auto output: console=\"{}\"\n", consoleToStr(consoleOutput));
            }

            writeSettings.shouldRemoveMaps = outputConfig["variables"].value("removeMaps", true);
            writeSettings.shouldRemovePlayers = outputConfig["variables"].value("removePlayers", true);
            writeSettings.shouldRemoveStructures = outputConfig["variables"].value("removeStructures", true);
            writeSettings.shouldRemoveRegionsOverworld = outputConfig["variables"].value("removeRegionsOverworld", false);
            writeSettings.shouldRemoveRegionsNether = outputConfig["variables"].value("removeRegionsNether", false);
            writeSettings.shouldRemoveRegionsEnd = outputConfig["variables"].value("removeRegionsEnd", false);


            if (consoleOutput == lce::CONSOLE::RPCS3 || consoleOutput == lce::CONSOLE::PS3) {
                std::string optStr = outputConfig.value("autoPs3ProductCode", "");
                auto optEnum = editor::PS3Mapper.fromString(optStr);
                if (optEnum) {
                    writeSettings.m_productCodes.setPS3(optEnum.value());
                    log(eLog::input, "Using auto output: PS3 P.C.=\"{}\"\n", optStr);
                } else {
                    log(eLog::error, "Invalid input \"conversionOutput.autoPs3ProductCode\"\n");
                }
            } else if (consoleOutput == lce::CONSOLE::VITA) {
                std::string optStr = outputConfig.value("autoPsVProductCode", "");
                auto optEnum = editor::VITAMapper.fromString(optStr);
                if (optEnum) {
                    writeSettings.m_productCodes.setVITA(optEnum.value());
                    log(eLog::input, "Using auto output: PsVita P.C.=\"{}\"\n", optStr);
                } else {
                    log(eLog::error, "Invalid input \"conversionOutput.autoPsVProductCode\"\n");
                }
            }

            log(eLog::input,
                "Using auto output: removeMaps=\"{}\"\n",
                writeSettings.shouldRemoveMaps ? "true" : "false");
            log(eLog::input,
                "Using auto output: removePlayers=\"{}\"\n",
                writeSettings.shouldRemovePlayers ? "true" : "false");
            log(eLog::input,
                "Using auto output: removeStructures=\"{}\"\n",
                writeSettings.shouldRemoveStructures ? "true" : "false");
            log(eLog::input,
                "Using auto output: removeRegionsOverworld=\"{}\"\n",
                writeSettings.shouldRemoveRegionsOverworld ? "true" : "false");
            log(eLog::input,
                "Using auto output: removeRegionsNether=\"{}\"\n",
                writeSettings.shouldRemoveRegionsNether ? "true" : "false");
            log(eLog::input,
                "Using auto output: removeRegionsEnd=\"{}\"\n",
                writeSettings.shouldRemoveRegionsEnd ? "true" : "false");

        } else {
            exit(-1);
        }

        const std::string consoleStr = consoleToStr(consoleOutput);
        const fs::path outputPath = getOutputPath(jsonConfig, consoleStr, defaultOutDir);
        if (!outputPath.empty() && !fs::exists(outputPath)) {
            fs::create_directories(outputPath);
        }
        writeSettings.setConsole(consoleOutput);
        writeSettings.setInFolderPath(outputPath);

        log(eLog::info, "Output directory: {}\n", outputPath.string());



        // iterate over all the files they gave
        for (const auto& arg: saveFileArgs) {
            fs::path filePath(arg);
            std::cout << "\n";
            log(eLog::input, "Loading savefile: {}\n", filePath.string());


            if (!fs::exists(filePath)) {
                log(eLog::error, "File does not exist: {}\n", filePath.make_preferred().string());
                continue;
            }

            Timer readTimer;
            editor::SaveProject saveProject;

            if (saveProject.read(filePath.string()) != 0) {
                log(eLog::error, "Failed to load file: {}\n", filePath.make_preferred().string());
                continue;
            }
            log(eLog::time, "Time to load: {} sec\n", readTimer.getSeconds());



            using CompressionType = GameRuleFileHeader::CompressionType;
            GameRuleFileReader reader( CompressionType::Zlib );

            auto grfFile = saveProject.collectFiles(lce::FILETYPE::GRF);
            Buffer raw = grfFile.begin()->getBuffer();

            // fs::path path = "E:\\Emulators\\cemu_1.27.1\\mlc01\\usr\\title\\0005000e\\101d9d00\\content\\WiiU\\Tutorial\\GameRules.grf";
            // Buffer raw = DataReader::readFile(path);

            GameRuleFile grf = reader.FromBuffer(raw, lce::CONSOLE::PS3);





            const int statusProcess = editor::preprocess(saveProject, saveProject.m_stateSettings, writeSettings);
            if (statusProcess != 0) {
                log(eLog::error,
                    "Preprocessing {} failed for file: {}\n",
                    consoleStr, filePath.string());
                continue;
            }

            saveProject.printDetails();


            editor::convert(saveProject, writeSettings);
            saveProject.setLatestVersion(10);
            saveProject.setOldestVersion(10);

            saveProject.printDetails();


            Timer writeTimer;
            const int statusOut = saveProject.write(writeSettings);
            if (statusOut != 0) {
                log(eLog::error, "Converting to {} failed for file: {}\n", consoleStr, filePath.string());
                continue;
            }
            log(eLog::time, "Time to write: {} sec\n", writeTimer.getSeconds());

#ifdef DEBUG
            std::cout << "[*] level.dat: ";
            if (auto *level = saveProject.m_fileListing.findFile(lce::FILETYPE::LEVEL))
                DataWriter::writeFile("C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\orig_level.dat", level.m_data.span());
            DataReader reader(level.m_data.span());
            NBTBase nbt = NBTBase::read(reader);
            nbt.print();
            nbt.writeFile("C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\level.dat");
        }
#endif

        std::cout << "\n";
        log(eLog::info, "Conversion Paths:\n");
        log(eLog::output, "{}\n", filePath.make_preferred().string());
        log(eLog::input, "{}\n", writeSettings.getOutFilePath().make_preferred().string());
    }

    std::cout << "\n";
    log(eLog::input, "Press ENTER to exit...\n");
    consumeEnter();
    return 0;
}
