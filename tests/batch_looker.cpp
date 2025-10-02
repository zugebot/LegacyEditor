#include <iostream>

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "common/fmt.hpp"
#include "common/timer.hpp"
#include "common/windows/force_utf8.hpp"

#include "code/convert/convertWorld.hpp"
#include "code/include.hpp"
#include "common/ConverterConfig.hpp"

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
    const char* last = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    return (ec == std::errc{} && ptr == last);
}

static bool parse_yes_no(std::string_view s, bool& out) {
    s = trim(s);
    if (s.empty()) return false;
    // accept y/n/yes/no/1/0/true/false (case-insensitive)
    std::string tmp(s);
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
    if (tmp == "y" || tmp == "yes" || tmp == "1" || tmp == "true") {
        out = true;
        return true;
    }
    if (tmp == "n" || tmp == "no" || tmp == "0" || tmp == "false") {
        out = false;
        return true;
    }
    return false;
}


inline void consumeEnter(const char* prompt = nullptr) {
    if (prompt) { std::cout << prompt << std::flush; }
    // Eat any leftover chars on the current line, then wait for a fresh Enter.
    std::string dummy;
    if (std::cin.peek() != '\n') {
        std::getline(std::cin, dummy); ///< finish current line if partial
    }
    std::getline(std::cin, dummy); ///< wait for user to press Enter
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
        if (parse_yes_no(line, yn)) {
            setting = yn;
            return;
        }
        log(eLog::error, "Please enter 'y' or 'n'.\n");
    }
}



int main(int argc, char* argv[]) {
    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();
#ifdef _WIN32
    force_utf8_console();
#endif

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
            log(eLog::info, "You can drag & drop the GAMEDATA/SAVEGAME/.dat/.bin on the executable or pass as arguments.\n");


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
                    return s;
                }
            };
            saveFileArgs.emplace_back(readExistingPath("File Path:"));

        } else {
            for (int i = 1; i < argc; i++) {
                saveFileArgs.emplace_back(argv[i]);
            }
        }
    }


    // saves detected
    log("Saves Detected:\n");
    int count = 0;
    for (auto it = saveFileArgs.begin(); it != saveFileArgs.end();) {
        try {
            const auto& arg = *it;
            const auto consoleDetected = editor::SaveProject::detectConsole(arg);

            ++count; // count only valid entries we print
            std::cout << "  [" << count << ", " << lce::consoleToStr(consoleDetected) << "] "
                      << arg << "\n";

            ++it; // advance normally on success
        } catch (const std::exception& e) {
            std::cout << "  [" << count << "] Error: " << e.what() << "\n    (not converting: " << *it << ")\n";
            it = saveFileArgs.erase(it);
        }
    }
    if (saveFileArgs.empty()) {
        std::cout << "\n";
        log(eLog::input, "Press ENTER to exit.\n");
        consumeEnter();
        return -1;
    }

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



        try {
            Timer convertTimer;


            // ...

            log(eLog::time, "Time to convert: {} sec\n", convertTimer.getSeconds());
        } catch (std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
            break;
        }

       MU int filesDeleted = saveProject.cleanup();
    }

    std::cout << "\n";
    log(eLog::input, "Press ENTER to exit.\n");
    consumeEnter();
    return 0;
}
