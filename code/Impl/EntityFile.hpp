#pragma once

#include "code/SaveFile/SaveProject.hpp"
#include "common/nbt.hpp"
#include "common/Pos2DTemplate.hpp"
#include "include/lce/processor.hpp"
#include <list>
#include <unordered_map>

namespace editor {

    struct EntityStruct {
        Pos2D coordinate{};
        NBTBase nbt;
    };

    class EntityFile {
    public:
        using EntityList = std::list<EntityStruct>;
        using EntityMap  = std::unordered_map<Pos2D, NBTBase, Pos2D::Hasher>;

        // Reads entity data from an LCEFile and returns an entity list
        MU static EntityList readEntityList(const LCEFile& file);

        // Converts a list into a map based on chunk position
        MU static EntityMap toEntityMap(const EntityList& list);

        // Writes the entity list to an LCEFile buffer
        MU static void writeEntityList(LCEFile& file, const EntityList& list);
    };

}
