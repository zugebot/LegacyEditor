#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <utility>

#include "LegacyEditor/utils/dataManager.hpp"


// this works with the .ext file in cemu / vita, and in xbox .bin
struct WorldOptions {
    i64 displaySeed = 0;
    u32 numLoads = 0;
    u32 hostOptions = 0;
    u32 texturePack = 0;
    u32 extraData = 0;
    u32 numExploredChunks = 0;
    std::string baseSaveName;
};


class FileInfo {
private:
    typedef std::optional<std::chrono::system_clock::time_point> timePoint_t;

public:
    DataManager saveFileData{};
    DataManager thumbnailImage{};
    std::wstring saveName;
    timePoint_t createdTime = std::nullopt;
    WorldOptions options;

    FileInfo() = default;
    /*
    FileInfo(DataInManager saveFileDataIn, std::wstring saveNameIn, DataInManager thumbnailImageIn,
             timePoint_t createdTimeIn, WorldOptions optionsIn)
        : saveFileData(saveFileDataIn), saveName(std::move(saveNameIn)), thumbnailImage(thumbnailImageIn),
          createdTime(createdTimeIn), options(std::move(std::move(optionsIn))) {}
          */
};
