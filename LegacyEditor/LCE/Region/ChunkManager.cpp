#include "ChunkManager.hpp"

#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"

#include "LegacyEditor/LCE/Region/Chunk/v10.hpp"
#include "LegacyEditor/LCE/Region/Chunk/v11.hpp"
#include "LegacyEditor/LCE/Region/Chunk/v12.hpp"




MU void ChunkManager::readChunk(DIM dim) {
    DataManager managerIn = DataManager(this->data, this->size);
    managerIn.seekStart();
    chunkData->lastVersion = managerIn.readInt16();
    
    switch(chunkData->lastVersion) {
        case 0x0a00: {
            chunkData->lastVersion = 0x000A;
            chunk::ChunkV10 v10Chunk;
            v10Chunk.readChunk(chunkData, &managerIn, dim);
            break;
        }
        case 0x0008:
        case 0x0009:
        case 0x000B: {
            chunk::ChunkV11 v11Chunk;
            v11Chunk.readChunk(chunkData, &managerIn, dim);
            break;
        }
        case 0x000C: {
            chunk::ChunkV12 v12Chunk;
            v12Chunk.readChunk(chunkData, &managerIn, dim);
            break;
        }
    }
}


MU void ChunkManager::writeChunk(DIM dim) {
    Data outBuffer(CHUNK_BUFFER_SIZE);
    memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
    auto managerOut = DataManager(outBuffer);

    managerOut.seekStart();
    switch(chunkData->lastVersion) {
        case 0x0a00: {
            chunk::ChunkV10 v10Chunk;
            v10Chunk.writeChunk(chunkData, &managerOut, dim);
            break;
        }
        case 0x0008:
        case 0x0009:
        case 0x000B: {
            managerOut.writeInt16(chunkData->lastVersion);
            chunk::ChunkV11 v11Chunk;
            v11Chunk.writeChunk(chunkData, &managerOut, dim);
            break;
        }
        case 0x000C: {
            managerOut.writeInt16(chunkData->lastVersion);
            chunk::ChunkV12 v12Chunk;
            v12Chunk.writeChunk(chunkData, &managerOut, dim);
            break;
        }
    }

    Data outData(managerOut.getPosition());
    memcpy(outData.data, outBuffer.data, outData.size);
    outBuffer.deallocate();
    deallocate();

    data = outData.data;
    size = outData.size;
    setDecSize(size);
}


void ChunkManager::ensure_decompress(CONSOLE console) {
    if (!getCompressed() || console == CONSOLE::NONE || data == nullptr || size == 0) {
        // printf("cannot Chunk.ensure_decompress the chunk if its already decompressed\n");
        // printf("passed CONSOLE::NONE to Chunk.ensure_decompress, results will not work\n");
        // printf("chunk data is nullptr, cannot Chunk.ensure_decompress nothing\n");
        // printf("cannot decompress data of chunk that has not been loaded");
        return;
    }
    setCompressed(false);
    
    u32 dec_size_copy = getDecSize();
    Data decompData(getDecSize());

    switch (console) {
        case CONSOLE::XBOX360:
            dec_size_copy = XDecompress(decompData.start(), &decompData.size, data, size);
            break;
        case CONSOLE::PS3:
            tinf_uncompress(decompData.start(), &decompData.size, data, size);
            break;
        case CONSOLE::WIIU:
        case CONSOLE::VITA:
            tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
            break;
        default:
            break;
    }

    deallocate();

    if (getRLE()) {
        allocate(getDecSize());
        RLE_decompress(decompData.start(), decompData.size, start(), dec_size_copy);
        decompData.deallocate();
    } else {
        data = decompData.data;
        size = dec_size_copy;
        decompData.reset();
    }
}


void ChunkManager::ensure_compressed(CONSOLE console) {
    if (getCompressed() || console == CONSOLE::NONE || data == nullptr || size == 0) {
        // printf("cannot Chunk.ensure_compress if the chunk is already compressed\n");
        // printf("passed CONSOLE::NONE to Chunk.ensure_compress, results will not work\n");
        // printf("chunk data is nullptr, cannot Chunk.ensure_compress nothing\n");
        return;
    }
    setCompressed(true);
    setDecSize(size);

    if (getRLE()) {
        Data rleBuffer(size);
        RLE_compress(data, size, rleBuffer.data, rleBuffer.size);
        deallocate();
        data = rleBuffer.data;
        size = rleBuffer.size;
        rleBuffer.reset();
    }

    // allocate memory and recompress
    u8* comp_ptr = new u8[size];
    uLongf comp_size = size;

    // TODO: Does it work for vita?
    switch (console) {
        case CONSOLE::XBOX360:
            // TODO: leaks memory
            // XCompress(comp_ptr, comp_size, data_ptr, data_size);
            break;
        case CONSOLE::PS3:
            // TODO: leaks memory
            // tinf_compress(comp_ptr, comp_size, data_ptr, data_size);
            break;

        case CONSOLE::RPCS3: {
            int status = ::def(data, comp_ptr, size, (uLongf*)&comp_size, -15);
            if (status != 0) {
                printf("error has occurred compressing chunk\n");
            }

            deallocate();
            data = comp_ptr;
            size = comp_size;
            comp_size = 0;
            break;
        }

        case CONSOLE::WIIU:
        case CONSOLE::VITA: {
            int status = ::compress(comp_ptr, &comp_size, data, size);
            if (status != 0) {
                printf("error has occurred compressing chunk\n");
            }

            deallocate();
            data = comp_ptr;
            size = comp_size;
            comp_size = 0;
            break;
        }
        default:
            break;
    }

}
