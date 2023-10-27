#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <utility>

#include "LegacyEditor/utils/managers/dataInManager.hpp"
#include "LegacyEditor/utils/managers/dataOutManager.hpp"


struct WorldOptions {
    int64_t displaySeed = 0;
    uint32_t numLoads = 0;
    uint32_t hostOptions = 0;
    uint32_t texturePack = 0;
    uint32_t extraData = 0;
    uint32_t numExploredChunks = 0;
    std::string baseSaveName;
};


class FileInfo {
private:
    typedef std::optional<std::chrono::system_clock::time_point> timePoint_t;

public:
    DataInManager saveFileData = DataInManager();
    DataInManager thumbnailImage = DataInManager();
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
