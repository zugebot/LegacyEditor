#include "EntityFile.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"

namespace editor {

    EntityFile::EntityList EntityFile::readEntityList(const LCEFile& file) {
        EntityList result;

        Buffer buffer = file.getBuffer();
        if (buffer.empty()) return result;

        DataReader reader(buffer.span(), Endian::Big);
        int count = reader.read<i32>();

        for (int i = 0; i < count; ++i) {
            int x = reader.read<i32>();
            int z = reader.read<i32>();
            NBTBase entity = NBTBase::read(reader);
            entity = entity[""]["Entities"];
            result.emplace_back(EntityStruct{Pos2D(x, z), std::move(entity)});
        }

        return result;
    }

    EntityFile::EntityMap EntityFile::toEntityMap(const EntityList& list) {
        EntityMap map;
        for (const auto& [coord, nbt] : list) {
            auto nbtCopy = nbt.copy();
            map.emplace(coord, std::move(nbtCopy));
        }
        return map;
    }

    void EntityFile::writeEntityList(LCEFile& file, const EntityList& list) {
        DataWriter writer(256, Endian::Big);

        writer.write<i32>(static_cast<i32>(list.size()));
        for (const auto& [coord, nbt] : list) {
            writer.write<i32>(coord.x);
            writer.write<i32>(coord.z);
            NBTBase final =
                makeCompound( {
                     { "",
                        makeCompound( {
                            { "Entities", nbt }
                        } )
                     }
                }
            );
            final.write(writer);
        }

        file.setBuffer(writer.take());
    }

}
