#include "ChunkManager.hpp"


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
            tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
            break;
        default:
            break;
    }

    if (data != nullptr) {
        delete[] data;
        data = nullptr;
        size = 0;
    }

    if (rleFlag) {
        Data rle(dec_size);
        RLE_decompress(decompData.start(), decompData.size, rle.start(), dec_size_copy);
        data = rle.data;
        size = dec_size;
        return;

    } else {
        data = decompData.data;
        size = dec_size_copy;
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
        Data rle(size);
        RLE_compress(data, size, rle.data, rle.size);

        if (data != nullptr) {
            delete[] data;
            data = nullptr;
            size = 0;
        }

        data = rle.data;
        size = rle.size;
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
        case CONSOLE::WIIU: {
            ::compress(comp_ptr, &comp_size, data, size);

            if (data != nullptr) {
                delete[] data;
                data = nullptr;
                size = 0;
            }

            data = comp_ptr;
            size = comp_size;
            break;
        }
        default:
            break;
    }


}






