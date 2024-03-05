#pragma once

#include "RegionManager.hpp"

namespace editor {
class File;

/**
 * should be part of FileListing?
 * should be passed all the regions
 * has functions getBlock(), setBlock()
 * if it needs a specific region, it reads it
 * if it needs a specific chunk, it reads it
 * call a function at the end of its usage to clean up loaded chunks
 * should regionManager be rewritten to act more like chunkManager?
 */
class WorldManager {
public:
    WorldManager() = default;


};

} // editor

