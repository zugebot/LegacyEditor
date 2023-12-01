#include "ChunkManager.hpp"

#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"


void ChunkManager::ensure_decompress(CONSOLE console) {
    if (!isCompressed || console == CONSOLE::NONE || data == nullptr || sectors == 0) {
        // printf("cannot Chunk.ensure_decompress the chunk if its already decompressed\n");
        // printf("passed CONSOLE::NONE to Chunk.ensure_decompress, results will not work\n");
        // printf("chunk data is nullptr, cannot Chunk.ensure_decompress nothing\n");
        // printf("cannot decompress data of chunk that has not been loaded");
        return;
    }
    isCompressed = false;

    u32 dec_size_copy = dec_size;
    Data decompData(dec_size);

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

    if (rleFlag) {
        allocate(dec_size);
        RLE_decompress(decompData.start(), decompData.size, start(), dec_size_copy);
        decompData.deallocate();
    } else {
        data = decompData.data;
        size = dec_size_copy;
        decompData.reset();
    }
}


void ChunkManager::ensure_compressed(CONSOLE console) {
    if (isCompressed || console == CONSOLE::NONE || data == nullptr || sectors == 0) {
        // printf("cannot Chunk.ensure_compress if the chunk is already compressed\n");
        // printf("passed CONSOLE::NONE to Chunk.ensure_compress, results will not work\n");
        // printf("chunk data is nullptr, cannot Chunk.ensure_compress nothing\n");
        return;
    }

    isCompressed = true;
    dec_size = size;

    if (rleFlag) {
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






