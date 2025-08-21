#include <iostream>

#include "include/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"
#include "include/nlohmann/json.hpp"

#include "common/fmt.hpp"
#include "common/timer.hpp"
#include "common/windows/force_utf8.hpp"

#include "code/convert/convertWorld.hpp"
#include "code/include.hpp"

#include "code/Impl/CacheBinManager.hpp"
#include "common/Config.hpp"
#include "tinf/tinf.h"
#include "zlib-1.2.12/zlib.h"

#include <windows.h>


using namespace cmn;


inline void consumeEnter(const char* prompt = nullptr) {
    if (prompt) std::cout << prompt;
    std::cin.clear();

    // If there is already a newline sitting in the buffer from a prior `>>`, eat it.
    if (std::cin.rdbuf()->in_avail() > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Now actually wait for the user to press Enter.
    std::cin.get();
}

std::string EXE_CURRENT_PATH;


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


void handleRemovalOption(const std::string& prompt, bool& setting) {
    log(eLog::input, "{} (y/n): ", prompt);
    char choice = 'y';
    std::cin >> choice;
    consumeEnter();
    setting = (choice == 'y' || choice == 'Y');
}


static const editor::sch::Schematic& selectSchemaVersionInteractive() {
    struct Opt {
        const char* label;
        const editor::sch::Schematic* schema;
    };
    static const Opt opts[] = {
            {"AquaticTU69",  &editor::sch::AquaticTU69},
            {"Potions",      &editor::sch::Potions},
            {"ElytraLatest", &editor::sch::ElytraLatest},
    };

    log(eLog::input, "Select a schema version:\n");
    for (int i = 0; i < static_cast<int>(std::size(opts)); ++i) {
        std::cout << "  [" << (i + 1) << "] " << opts[i].label << "\n";
    }
    std::cout << std::flush;

    int choice = getNumberFromUser("Enter index: ", 1, static_cast<int>(std::size(opts)));
    return *opts[choice - 1].schema;
}


int main(int argc, char* argv[]) {
    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();
#ifdef _WIN32
    force_utf8_console();
#endif

    /*
    std::string m_name = "correct.dat";
    fs::path m_filePath = R"(C:\Users\jerrin\AppData\Roaming\yuzu\nand\user\save\0000000000000000\D5FA36C00E9BAF9DB378592F82847B9E\01006BD001E06000)";


    editor::WriteSettings settings;
    settings.setInFolderPath(m_filePath);

    editor::SaveProject proj1;
    proj1.read(m_filePath / m_name);
    settings.setConsole(lce::CONSOLE::WIIU);
    proj1.write(settings);

    {
        std::string fileIn = m_name;
        Buffer src;
        DataReader reader;

        try {
            src = DataReader::readFile(m_filePath / fileIn);
            reader = DataReader(src, Endian::Little);
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        u32 final_size = reader.peek_at<u32>(4);

        Buffer dest;
        if (!dest.allocate(final_size)) {
            return printf_err(MALLOC_FAILED, ERROR_1, final_size);
        }

        int status = tinf_zlib_uncompress(dest.data(), dest.size_ptr(), src.data() + 8, src.size() - 8);
        if (status != 0) {
            return DECOMPRESS;
        }
        DataWriter::writeFile(m_filePath / (fileIn + "_decomp"), dest.span());
    }

    Sleep(5000);

    editor::SaveProject proj2;
    settings.setConsole(lce::CONSOLE::SWITCH);
    proj2.read(m_filePath / settings.m_fileNameOut);
    proj2.write(settings);


    {
        std::string fileIn = settings.m_fileNameOut;
        Buffer src;
        DataReader reader;

        try {
            src = DataReader::readFile(m_filePath / fileIn);
            reader = DataReader(src, Endian::Little);
        } catch (const std::exception& e) {
            return printf_err(FILE_ERROR, ERROR_4, m_filePath.string().c_str());
        }

        u32 final_size = reader.peek_at<u32>(4);

        Buffer dest;
        if (!dest.allocate(final_size)) {
            return printf_err(MALLOC_FAILED, ERROR_1, final_size);
        }

        int status = tinf_zlib_uncompress(dest.data(), dest.size_ptr(), src.data() + 8, src.size() - 8);
        if (status != 0) {
            return DECOMPRESS;
        }
        DataWriter::writeFile(m_filePath / (fileIn + "_decomp"), dest.span());
    }*/
    /*
    // status = editor::FileListing::readListing(saveProject, dest, m_console);
    // if (status != 0) {
    //     return -1;
    // }
    // saveProject.setNewGen(true);
    // if (fs::path thumb = m_filePath.parent_path() / "THUMB";
    //     saveProject.m_stateSettings.console() == lce::CONSOLE::SWITCH
    //     && fs::exists(thumb)) {
    //     saveProject.m_stateSettings.setConsole(lce::CONSOLE::PS4);
    // }


    // return 0;



    // fs::path cachePath = "E:\\Emulators\\Vita3K\\LOL\\ux0\\user\\00\\savedata\\PCSE00491\\CACHE.bin";
    // CacheBinManager manager;
    // manager.load(cachePath);
    */
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
        "Supports writing  [ -------  ---  RPCS3, PSVITA, +/-  WiiU/Cemu  Switch, ---------- ]\n\n");

    ConverterConfig config;
    config.read("conversion.json");

    std::vector<std::string> saveFileArgs;

    if (config.conversionInput.autoInput) {
        log(eLog::input, "Reading auto input from \"configuration.json\"\n");
        const auto& key = config.conversionInput.autoKey;
        auto it = config.conversionInput.autoPath.find(key);
        if (it == config.conversionInput.autoPath.end()) {
            log(eLog::error, "Invalid input conversionInput.autoPath[{}]\n", key);
            return -1;
        }
        saveFileArgs.push_back(it->second);

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


    const editor::sch::Schematic* chosenSchema = &editor::sch::AquaticTU69;

    if (config.conversionOutput.autoOutput) {
        log(eLog::input, "Using default schema (AquaticTU69) from config.\n");
    } else {
        chosenSchema = &selectSchemaVersionInteractive();
        log(eLog::success, "Schema selected.\n");
    }

    editor::WriteSettings writeSettings(*chosenSchema);
    bool isShadPs4 = false;


    if (config.conversionOutput.autoOutput) {
        consoleOutput = lce::strToConsole(config.conversionOutput.autoConsole);
        if (consoleOutput == lce::CONSOLE::NONE) {
            log(eLog::error, "Invalid conversionOutput.autoConsole\n");
            return -1;
        }

        log(eLog::input, "Using auto output: console = \"{}\"\n", config.conversionOutput.autoConsole);

        const auto& vars = config.conversionOutput.variables;
        writeSettings.removePlayers = vars.removePlayers;
        writeSettings.removeMaps = vars.removeMaps;
        writeSettings.removeStructures = vars.removeStructures;
        writeSettings.removeRegionsOverworld = vars.removeRegionsOverworld;
        writeSettings.removeRegionsNether = vars.removeRegionsNether;
        writeSettings.removeRegionsEnd = vars.removeRegionsEnd;
        writeSettings.removeEntities = vars.removeEntitiesDat;


        if (consoleOutput == lce::CONSOLE::RPCS3 || consoleOutput == lce::CONSOLE::PS3) {
            const std::string optStr = config.conversionOutput.autoPs3ProductCode;
            const auto optEnum = editor::PS3Mapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setPS3(optEnum.value());
                log(eLog::input, "Using auto output: PS3 P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPs3ProductCode\"\n");
            }
        } else if (consoleOutput == lce::CONSOLE::VITA) {
            const std::string optStr = config.conversionOutput.autoPsVProductCode;
            const auto optEnum = editor::VITAMapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setVITA(optEnum.value());
                log(eLog::input, "Using auto output: PsVita P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPsVProductCode\"\n");
            }
        } else if (consoleOutput == lce::CONSOLE::PS4) {

            std::cout << "do you want to convert to shadPs4? (y/n)\n";
            std::string userInput2;
            std::cin >> userInput2;
            if (userInput2 == "y") {
                isShadPs4 = true;
            }
            std::cout << "\n";


            std::cout << "You must provide the file path to a PARAM.SFO file that comes from a save file,\n"
                         "from BOTH the same console AND account. You can find it any \"sce_sys\" folder.\n"
                         "Please enter the full path to that file here. I am lazy and DO NOT have code that\n"
                         "checks that the file path is valid OR that the file itself is valid. Don't put \" in it.\n"
                         "Example:\n"
                         "C:\\Users\\jerrin\\jerrins_and_tiaras\\PS4-CUSA00744-1CUSA00744-210322225338.1\\savedata0\\sce_sys\\param.sfo\n"
                         "\n"
                         "Path: ";

            std::string userInput;
            std::cin >> userInput;

            writeSettings.m_paramSfoToReplace = userInput;


            const std::string optStr = config.conversionOutput.autoPs4ProductCode;
            const auto optEnum = editor::PS4Mapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setPS4(optEnum.value());
                log(eLog::input, "Using auto output: Ps4 P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPs4ProductCode\"\n");
            }


        }


    } else {

        log(eLog::input, "Saves Detected:\n");
        int count = 0;
        for (const auto& arg: saveFileArgs) {
            count++;
            auto consoleDetected = editor::SaveProject::detectConsole(arg);
            std::cout << "[" << count << "] [" + lce::consoleToStr(consoleDetected) << "] "
                      << arg << "\n";
        }
        std::cout << "\n"
                  << std::flush;

        log(eLog::input, "Name the console you want your saves converted to: ");
        std::string userInput;
        std::cin >> userInput;
        consumeEnter();
        consoleOutput = lce::strToConsole(userInput);
        if (consoleOutput == lce::CONSOLE::NONE) {
            log(eLog::error, "Invalid console name, exiting\n");
            consumeEnter();
            return -1;
        }

        if (consoleOutput == lce::CONSOLE::RPCS3 || consoleOutput == lce::CONSOLE::PS3) {
            editor::ePS3ProductCode code = selectProductCode(editor::PS3Mapper, "PS3");
            writeSettings.m_productCodes.setPS3(code);
        } else if (consoleOutput == lce::CONSOLE::VITA) {
            auto code = selectProductCode(editor::VITAMapper, "VITA");
            writeSettings.m_productCodes.setVITA(code);
        }

        handleRemovalOption("Do you want to remove all [map (item)] data   ", writeSettings.removeMaps);
        handleRemovalOption("Do you want to remove all [player    ] data   ", writeSettings.removePlayers);
        handleRemovalOption("Do you want to remove all [structure ] data   ", writeSettings.removeStructures);
        handleRemovalOption("Do you want to remove all [overworld ] regions", writeSettings.removeRegionsOverworld);
        handleRemovalOption("Do you want to remove all [nether    ] regions", writeSettings.removeRegionsNether);
        handleRemovalOption("Do you want to remove all [end       ] regions", writeSettings.removeRegionsEnd);
    }


    const std::string consoleStr = consoleToStr(consoleOutput);
    const fs::path outputPath = config.getOutputPath(consoleStr);
    if (!outputPath.empty() && !fs::exists(outputPath)) {
        fs::create_directories(outputPath);
    }
    writeSettings.m_schematic.setConsole(consoleOutput);
    writeSettings.setInFolderPath(outputPath);


    // editor::SaveProject toSteal;
    // toSteal.read(R"(E:\Emulators\cemu_1.27.1\mlc01\usr\save\00050000\101dbe00\user\80000001\all_loot_chests)");


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

        // TODO: really shit fucking garbage bad code
        saveProject.m_stateSettings.setShadPS4(isShadPs4);

        if (saveProject.read(filePath.string()) != 0) {
            log(eLog::error, "Failed to load file: {}\n", filePath.make_preferred().string());
            continue;
        }
        log(eLog::time, "Time to load: {} sec\n", readTimer.getSeconds());

        // (void) saveProject.dumpToFolder("");


        const int statusProcess = editor::preprocess(saveProject, saveProject.m_stateSettings, writeSettings);
        if (statusProcess != 0) {
            log(eLog::error,
                "Preprocessing {} failed for file: {}\n",
                consoleStr, filePath.string());
            continue;
        }

        saveProject.printDetails();

        editor::convert(saveProject, writeSettings);


        
        saveProject.m_displayMetadata.extraData = "0000000"; // "78000A8"; // 125829288;

        std::cout << consoleToStr(saveProject.m_stateSettings.console()) << std::endl;
        (void) saveProject.dumpToFolder("");


        saveProject.printDetails();


        Timer writeTimer;
        const int statusOut = saveProject.write(writeSettings);
        if (statusOut != 0) {
            log(eLog::error, "Converting to {} failed for file: {}\n", consoleStr, filePath.string());
            continue;
        }
        log(eLog::time, "Time to write: {} sec\n", writeTimer.getSeconds());

        saveProject.cleanup();


#ifdef DEBUG
        std::cout << "[*] level.dat: ";
        if (auto* level = saveProject.m_fileListing.findFile(lce::FILETYPE::LEVEL))
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
    std::cin.clear();
    consumeEnter();
    return 0;
}
