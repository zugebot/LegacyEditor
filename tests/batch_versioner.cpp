#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/Impl/levelFile.hpp"
#include "code/SaveFile/SaveProject.hpp"
#include "code/include.hpp"
#include "common/timer.hpp"

#include "code/Chunk/helpers/scoring.hpp"
#include "code/SaveFile/helpers/summary.hpp"

#include <algorithm>
#include <ranges>


void consumeEnter() {
    std::cout << "[>] Press ENTER to exit...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}


static auto WiiUParseLabel(std::string_view s) -> std::pair<int, int> {
    if (s.size() < 3 || s[0] != '(' || s[1] != 'v') return {0, 0};

    int number = 0;
    size_t i = 2;
    while (i < s.size() && std::isdigit(s[i])) {
        number = number * 10 + (s[i] - '0');
        ++i;
    }

    return {number, 0};
}



std::string EXE_CURRENT_PATH;

int main(MU int argc, char* argv[]) {

    EXE_CURRENT_PATH = fs::path(argv[0]).parent_path().string();


    const lce::CONSOLE CONSOLE = lce::CONSOLE::XBOX360;



    fs::path dirMain(argv[0]);
    fs::path inDir;

    switch (CONSOLE) {
        case (lce::CONSOLE::XBOX360): {
            inDir = "E:\\Minecraft\\Xbox360-Worlds";
            break;
        }
        case (lce::CONSOLE::WIIU): {
            inDir = "E:\\Minecraft\\WiiU-Worlds";
            break;
        }
    }

    std::vector<editor::summary::SaveSummary> saveSummaries;
    std::vector<std::tuple<std::string, fs::path>> versionFiles;


    try {
        for (c_auto& folder: fs::directory_iterator(inDir)) {
            if (!is_directory(folder)) { continue; }

            switch (CONSOLE) {
                case (lce::CONSOLE::XBOX360): {
                    std::string name = folder.path().filename().string();
                    std::string tuLabel = name;
                    if (auto pos = name.find(' '); pos != std::string::npos)
                        tuLabel = name.substr(0, pos);

                    for (c_auto& file: fs::directory_iterator(folder)) {
                        if (file.path().string().ends_with(".bin")) {
                            versionFiles.emplace_back(tuLabel, file.path() / "savegame.dat");
                        }
                    }
                    break;
                }

                case (lce::CONSOLE::WIIU): {
                    std::string name = folder.path().filename().string();
                    std::string buildLabel = name;
                    if (auto pos = name.find(' '); pos != std::string::npos)
                        buildLabel = name.substr(0, pos);


                    std::set<fs::path> files;
                    for (const auto& dirEntry : fs::directory_iterator(folder)) {
                        files.insert(dirEntry);
                    }

                    bool found = false;
                    for (const auto& file : files) {
                        if (file.extension() == ".ext") {
                            fs::path temp = file;
                            temp.replace_extension("");
                            if (fs::exists(temp)) {
                                versionFiles.emplace_back(buildLabel, temp);
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found) {
                        std::cout << "no save found inside " << folder << "\n";
                    }

                    break;
                }
            }

        }
    } catch (std::runtime_error& e) {
        std::cout << "failed to read file or smth" << std::endl;
    }


    switch (CONSOLE) {
        case (lce::CONSOLE::XBOX360): {
            std::sort(versionFiles.begin(), versionFiles.end(),
                      [](const auto& a, const auto& b) {
                          return editor::parseLabel(std::get<0>(a)) <
                                 editor::parseLabel(std::get<0>(b)); });
            break;
        }
        case (lce::CONSOLE::WIIU): {
            std::sort(versionFiles.begin(), versionFiles.end(),
                      [](const auto& a, const auto& b) {
                          return WiiUParseLabel(std::get<0>(a)) <
                                 WiiUParseLabel(std::get<0>(b)); });
            break;
        }
    }

    for (auto& versionFile : versionFiles) {
        auto& vf_tuLabel   = std::get<0>(versionFile);
        auto& vf_savegame  = std::get<1>(versionFile);

        editor::summary::SaveSummary summary = editor::summary::createSummary(CONSOLE, vf_savegame);
        if (summary.m_isValid) {
            switch (CONSOLE) {
                case (lce::CONSOLE::XBOX360): {
                    summary.m_summary.insert("_TU", makeString(vf_tuLabel));
                    break;
                }
                case (lce::CONSOLE::WIIU): {
                    summary.m_summary.insert("_BUILD", makeString(vf_tuLabel));
                    break;
                }
            }
            saveSummaries.emplace_back(summary);
        }
    }

    std::ofstream tableOut(lce::consoleToStr(CONSOLE) + "_level_summary.txt");
    if (!tableOut)
        throw std::runtime_error("Cannot open level_summary.txt for writing");

    print_table(saveSummaries, tableOut);
    tableOut.close();

    return 0;
}