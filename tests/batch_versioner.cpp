#include <iostream>

#include "include/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"

#include "code/Impl/level.hpp"
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


int main(MU int argc, char* argv[]) {

    fs::path dirMain(argv[0]);
    fs::path inDir = "E:\\Xbox360\\Minecraft-Xbox360-Worlds";

    std::vector<editor::summary::SaveSummary> saveSummaries;
    std::vector<std::tuple<std::string, fs::path>> versionFiles;

    for (c_auto& folder: fs::directory_iterator(inDir)) {
        if (!is_directory(folder)) { continue; }

        std::string name = folder.path().filename().string();
        std::string tuLabel = name;
        if (auto pos = name.find(' '); pos != std::string::npos)
            tuLabel = name.substr(0, pos);

        for (c_auto& file: fs::directory_iterator(folder)) {
            if (file.path().string().ends_with(".bin")) {
                versionFiles.emplace_back(tuLabel, file.path() / "savegame.dat");
            }
        }
    }

    std::sort(versionFiles.begin(), versionFiles.end(),
              [](const auto& a, const auto& b) {
                  return editor::parseLabel(std::get<0>(a)) <
                         editor::parseLabel(std::get<0>(b)); });

    for (auto& versionFile : versionFiles) {
        auto& vf_tuLabel   = std::get<0>(versionFile);
        auto& vf_savegame  = std::get<1>(versionFile);

        editor::summary::SaveSummary summary = editor::summary::createSummary(vf_savegame);
        if (summary.m_isValid) {
            summary.m_summary.insert("_TU", makeString(vf_tuLabel));
            saveSummaries.emplace_back(summary);
        }
    }

    std::ofstream tableOut("level_summary.txt");
    if (!tableOut)
        throw std::runtime_error("Cannot open level_summary.txt for writing");

    print_table(saveSummaries, tableOut);
    tableOut.close();

    return 0;
}