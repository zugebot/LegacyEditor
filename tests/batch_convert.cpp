#include <iostream>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "common/fmt.hpp"
#include "common/timer.hpp"
#include "common/windows/force_utf8.hpp"

#include "code/convert/convertWorld.hpp"
#include "code/include.hpp"
#include "common/Config.hpp"

#include "code/Impl/CacheBinManager.hpp"
#include "tinf/tinf.h"
#include "zlib-1.2.12/zlib.h"



std::string EXE_CURRENT_PATH;
using namespace cmn;



static inline std::string_view trim(std::string_view s) {
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string_view::npos) return {};
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static bool getline_prompt(std::string& out, const std::string& prompt) {
    log(eLog::input, "{}", prompt);
    if (!std::getline(std::cin, out)) {
        // EOF (Ctrl+Z/Ctrl+D) or stream closed
        log(eLog::error, "Input aborted.\n");
        return false;
    }
    return true;
}

static bool parse_int_strict(std::string_view s, int& value) {
    s = trim(s);
    if (s.empty()) return false;
    // from_chars doesn’t skip whitespace; that’s why we trimmed.
    const char* first = s.data();
    const char* last  = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    return (ec == std::errc{} && ptr == last);
}

static bool parse_yes_no(std::string_view s, bool& out) {
    s = trim(s);
    if (s.empty()) return false;
    // accept y/n/yes/no/1/0/true/false (case-insensitive)
    std::string tmp(s);
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c){ return std::tolower(c); });
    if (tmp == "y" || tmp == "yes" || tmp == "1" || tmp == "true") { out = true;  return true; }
    if (tmp == "n" || tmp == "no"  || tmp == "0" || tmp == "false"){ out = false; return true; }
    return false;
}


inline void consumeEnter(const char* prompt = nullptr) {
    if (prompt) { std::cout << prompt << std::flush; }
    // Eat any leftover chars on the current line, then wait for a fresh Enter.
    std::string dummy;
    if (std::cin.peek() != '\n') {
        std::getline(std::cin, dummy); // finish current line if partial
    }
    std::getline(std::cin, dummy);     // wait for user to press Enter
}




static int getNumberFromUser(const std::string& prompt, int min, int max) {
    for (;;) {
        std::string line;
        if (!getline_prompt(line, prompt)) std::exit(1);
        int v = 0;
        if (parse_int_strict(line, v) && v >= min && v <= max) {
            return v;
        }
        log(eLog::error, "Invalid selection. Try again.\n");
    }
}


void handleRemovalOption(const std::string& prompt, bool& setting) {
    for (;;) {
        std::string line;
        if (!getline_prompt(line, cmn::fmt("{} (y/n): ", prompt))) std::exit(1);
        bool yn = false;
        if (parse_yes_no(line, yn)) { setting = yn; return; }
        log(eLog::error, "Please enter 'y' or 'n'.\n");
    }
}



template<typename EnumType>
EnumType selectProductCode(const editor::EnumMapper<EnumType>& mapper,
                           const std::string& consoleName) {
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


static lce::CONSOLE selectOutputConsoleInteractive()
{
    struct Option {
        lce::CONSOLE id;
        const char*  label;  // nice display name
        const char*  hint;   // optional note
    };

    // Keep this list in sync with “Supports writing …”
    static const Option kWritableOptions[] = {
            { lce::CONSOLE::WIIU,    "WiiU/Cemu", nullptr },
            { lce::CONSOLE::VITA,    "PS Vita",   nullptr },
            { lce::CONSOLE::RPCS3,   "Rpcs3",     nullptr },
            { lce::CONSOLE::PS4,     "Ps4",       nullptr },
            { lce::CONSOLE::SHADPS4, "ShadPs4",   nullptr },
            { lce::CONSOLE::SWITCH,  "Switch",    nullptr },
            // If/when native PS4 write is supported, uncomment this:
            // { lce::CONSOLE::PS4,      "PS4",       "Native PS4 format"   },
    };

    log("Select a target console (supported for writing):\n");
    for (int i = 0; i < static_cast<int>(std::size(kWritableOptions)); ++i) {
        const auto& o = kWritableOptions[i];
        std::cout << "  [" << (i + 1) << "] " << o.label;
                  // << " (" << lce::consoleToStr(o.id) << ')';
        if (o.hint) std::cout << " - " << o.hint;
        std::cout << "\n";
    }
    std::cout << std::flush;

    const int choice = getNumberFromUser("Enter index: ", 1,
                                         static_cast<int>(std::size(kWritableOptions)));

    const auto picked = kWritableOptions[choice - 1];
    log(eLog::success, "Selected: {}\n\n", picked.label);
    return picked.id;
}


static const editor::sch::Schematic& selectSchemaVersionInteractive() {
    struct Opt {
        const editor::sch::Schematic* schema;
        const char* label;
    };
    static const Opt opts[] = {
            {&editor::sch::Potions,      "(TU12)"},
            {&editor::sch::ElytraLatest, "(TU68)"},
            {&editor::sch::AquaticTU69,  "(TU69)"},
    };

    log("Select a schema version:\n");
    for (int i = 0; i < static_cast<int>(std::size(opts)); ++i) {
        std::cout << "  [" << (i + 1) << "] " <<
                opts[i].label << ", " <<
                opts[i].schema->display_name << "\n";
    }
    std::cout << std::flush;

    int choice = getNumberFromUser("Enter index: ", 1, static_cast<int>(std::size(opts)));

    log(eLog::success, "Selected: {}\n\n", opts[choice - 1].schema->display_name);

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

    std::cout << "\n";
    log(eLog::detail,
        "Find the project here! https://github.com/zugebot/LegacyEditor\n");
    log(eLog::detail,
        "Version: 1.3.1\n");
    log(eLog::detail,
        "Supports reading  [ Xbox360, PS3, RPCS3, PSVITA, PS4, ShadPs4, WiiU/Cemu, Switch, Windurango ]\n");
    log(eLog::detail,
        "Supports writing  [ -------  ---  RPCS3, PSVITA, ---, ShadPs4, WiiU/Cemu  Switch, ---------- ]\n\n");

    ConverterConfig config;
    config.read("conversion.json");

    std::vector<std::string> saveFileArgs;

    if (config.conversionInput.autoInput) {
        log("Reading auto input from \"configuration.json\"\n");
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
    std::reference_wrapper<const editor::sch::Schematic>
            chosenSchema = std::cref(editor::sch::AquaticTU69);
    editor::WriteSettings writeSettings(chosenSchema);



    if (config.conversionOutput.autoOutput) {

        // schema
        log(eLog::input, "Using default schema (AquaticTU69) from config.\n");

        // console
        consoleOutput = lce::strToConsole(config.conversionOutput.autoConsole);
        if (consoleOutput == lce::CONSOLE::NONE) {
            log(eLog::error, "Invalid conversionOutput.autoConsole\n");
            return -1;
        }
        log(eLog::input, "Using auto output: console = \"{}\"\n", config.conversionOutput.autoConsole);


        // console specific questions
        if (consoleOutput == lce::CONSOLE::RPCS3 || consoleOutput == lce::CONSOLE::PS3) {
            const std::string optStr = config.conversionOutput.autoPs3ProductCode;
            const auto optEnum = editor::PS3Mapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setPS3(optEnum.value());
                log(eLog::input, "Using auto output: PS3 P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPs3ProductCode\"\n");
                return -1;
            }

        } else if (consoleOutput == lce::CONSOLE::VITA) {
            const std::string optStr = config.conversionOutput.autoPsVProductCode;
            const auto optEnum = editor::VITAMapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setVITA(optEnum.value());
                log(eLog::input, "Using auto output: PsVita P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPsVProductCode\"\n");
                return -1;
            }
        } else if (consoleOutput == lce::CONSOLE::PS4 ||
                   consoleOutput == lce::CONSOLE::SHADPS4) {

            // might be important to ensure it's a valid param.sfo
            writeSettings.m_paramSfoToReplace = config.conversionInput.autoSampleParamSFO;
            if (writeSettings.m_paramSfoToReplace.empty()) {
                log(eLog::error, "Invalid input \"conversionOutput.autoSampleParamSFO\"\n");
                return -1;
            }

            const std::string optStr = config.conversionOutput.autoPs4ProductCode;
            const auto optEnum = editor::PS4Mapper.fromString(optStr);
            if (optEnum) {
                writeSettings.m_productCodes.setPS4(optEnum.value());
                log(eLog::input, "Using auto output: Ps4 P.C.=\"{}\"\n", optStr);
            } else {
                log(eLog::error, "Invalid input \"conversionOutput.autoPs4ProductCode\"\n");
            }


        }

        // assign variables
        const auto& vars = config.conversionOutput.variables;
        writeSettings.removePlayers = vars.removePlayers;
        writeSettings.removeMaps = vars.removeMaps;
        writeSettings.removeStructures = vars.removeStructures;
        writeSettings.removeRegionsOverworld = vars.removeRegionsOverworld;
        writeSettings.removeRegionsNether = vars.removeRegionsNether;
        writeSettings.removeRegionsEnd = vars.removeRegionsEnd;
        writeSettings.removeEntities = vars.removeEntitiesDat;

    // user input
    } else {

        // saves detected
        log("Saves Detected:\n");
        int count = 0;
        for (const auto& arg: saveFileArgs) {
            count++;
            auto consoleDetected = editor::SaveProject::detectConsole(arg);
            std::cout << "  [" << count << ", " + lce::consoleToStr(consoleDetected) << "] "
                      << arg << "\n";
        }
        std::cout << "\n" << std::flush;

        // console
        consoleOutput = selectOutputConsoleInteractive();

        // schema
        chosenSchema = selectSchemaVersionInteractive();


        writeSettings.setSchema(chosenSchema);

        // console specific questions
        if (consoleOutput == lce::CONSOLE::RPCS3 || consoleOutput == lce::CONSOLE::PS3) {
            editor::ePS3ProductCode code = selectProductCode(editor::PS3Mapper, "PS3");
            writeSettings.m_productCodes.setPS3(code);

        } else if (consoleOutput == lce::CONSOLE::VITA) {
            auto code = selectProductCode(editor::VITAMapper, "VITA");
            writeSettings.m_productCodes.setVITA(code);

        } else if (consoleOutput == lce::CONSOLE::PS4 ||
                   consoleOutput == lce::CONSOLE::SHADPS4) {
            editor::ePS4ProductCode code = selectProductCode(editor::PS4Mapper, "PS4");
            writeSettings.m_productCodes.setPS4(code);


            if (consoleOutput == lce::CONSOLE::PS4) {
                auto readExistingPath = [&](const std::string& prompt) -> std::string {
                    for (;;) {
                        std::string pathIn;
                        if (!getline_prompt(pathIn, prompt)) std::exit(1);
                        std::string s(trim(pathIn));
                        // strip paired surrounding quotes if present
                        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
                            s = s.substr(1, s.size() - 2);
                        }
                        // empty path
                        if (s.empty()) {
                            log(eLog::error, "Empty path. Try again.\n");
                            continue;
                        }
                        // file does not exist
                        if (!fs::exists(s)) {
                            log(eLog::error, "File does not exist:\n{}\n", fs::path(s).make_preferred().string());
                            continue;
                        }
                        // sfo does not have a "PARAMS" attribute
                        try {
                            SFOManager sfo(s);
                            auto attr = sfo.getAttribute("PARAMS");
                            if (!attr.has_value()) {
                                log(eLog::error, "File does not contain the \"PARAMS\" attribute:\n{}\n",
                                    fs::path(s).make_preferred().string());
                                continue;
                            }
                        }
                        // catch (const SFOManager::parse_error& e) {
                        //     log(eLog::error, "param.sfo parse error:\n{}\nReason: {}\n",
                        //         fs::path(s).make_preferred().string(), e.what());
                        //     continue;
                        // }
                        catch (const std::exception& e) { // fallback for anything std::exception-based
                            log(eLog::error, "Failed to read param.sfo:\n{}\nReason: {}\n",
                                fs::path(s).make_preferred().string(), e.what());
                            continue;
                        }

                        return s;
                    }
                };

                std::cout << "\n";
                log("You must provide the file path to a PARAM.SFO file that comes from a save file,\n"
                    "    from BOTH the same console AND account. You can find it any \"sce_sys\" folder.\n"
                    "    Please enter the full path to that file here. Don't put \" in it.\n");
                log("Example:\n"
                    "C:\\Users\\jerrin\\jerrins_and_tiaras\\PS4-CUSA00744-1CUSA00744-210322225338.1\\savedata0\\sce_sys\\param.sfo\n\n");

                writeSettings.m_paramSfoToReplace = readExistingPath("Path: ");
                std::cout << "\n";
            }


        }

        // assign variables
        log("Some conversion questions:\n");
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

        // saveProject.printDetails();

        (void) saveProject.dumpToFolder("before_ps4");

        std::set<lce::FILETYPE> items = {lce::FILETYPE::ENTITY_OVERWORLD};
        for (auto file : saveProject.view_of(items)) {
            Buffer buf = file.getBuffer();
            DataWriter::writeFile("C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\out\\entities_working.dat", buf.span());
        }


        const int statusProcess = editor::preprocess(saveProject, saveProject.m_stateSettings, writeSettings);
        if (statusProcess != 0) {
            log(eLog::error,
                "Preprocessing {} failed for file: {}\n",
                consoleStr, filePath.string());
            continue;
        }

        saveProject.printDetails();

        editor::convert(saveProject, writeSettings);

        saveProject.printDetails();

        for (auto file : saveProject.view_of(items)) {
            Buffer buf = file.getBuffer();
            DataWriter::writeFile("C:\\Users\\jerrin\\CLionProjects\\LegacyEditor\\build\\out\\entities_broken.dat", buf.span());
        }


        
        saveProject.m_displayMetadata.extraData = "0000000"; // "78000A8"; // 125829288;
        saveProject.m_displayMetadata.worldName = L"Entities?";

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
    log(eLog::input, "Press ENTER to exit.\n");
    consumeEnter();
    return 0;
}
