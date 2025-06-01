
#include "include/lce/processor.hpp"

#include "code/Chunk/v11.hpp"


int main(int argc, char* argv[]) {

    editor::chunk::ChunkData chunkData;
    editor::chunk::ChunkV11 chunkV11(&chunkData);

    std::string path = "E:\\Xbox360\\Minecraft-Xbox360-Worlds\\TU68 1.12.1944.0\\chunk\\";
    Buffer buffer = DataReader::readFile(path + "decompressed.chunk");
    DataReader reader(buffer.span());

    chunkData.lastVersion = reader.read<u16>();
    chunkV11.readChunk(reader);

    DataWriter writer;
    writer.write<u16>(11);
    chunkV11.writeChunk(writer);
    writer.save(path + "written.chunk");

    return 0;
}
