#include "ChunkManager.hpp"

#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/libs/tinf/tinf.h"
#include "LegacyEditor/libs/zlib-1.2.12/zlib.h"

#include "LegacyEditor/LCE/Chunk/v10.hpp"
#include "LegacyEditor/LCE/Chunk/v11.hpp"
#include "LegacyEditor/LCE/Chunk/v12.hpp"


namespace editor {

    

    MU void ChunkManager::readChunk(MU CONSOLE console) const {
        auto managerIn = DataManager(data, size);
        managerIn.seekStart();

        chunkData->lastVersion = managerIn.readInt16();

        switch(chunkData->lastVersion) {
            case HEADER_NBT: {
                chunkData->lastVersion = V_NBT;
                chunk::ChunkV10().readChunk(chunkData, &managerIn);
                break;
            }
            case V_8:
            case V_9:
            case V_11: {
                chunk::ChunkV11().readChunk(chunkData, &managerIn);
                break;
            }
            case V_12: {
                chunk::ChunkV12().readChunk(chunkData, &managerIn);
                break;
            }
            default:;
        }
    }


    MU void ChunkManager::writeChunk(MU CONSOLE console) {
        Data outBuffer(CHUNK_BUFFER_SIZE);
        memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
        auto managerOut = DataManager(outBuffer);

        managerOut.seekStart();
        switch (chunkData->lastVersion) {
            case HEADER_NBT: {
                chunk::ChunkV10().writeChunk(chunkData, &managerOut);
                break;
            }
            case V_8:
            case V_9:
            case V_11: {
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV11().writeChunk(chunkData, &managerOut);
                break;
            }
            case V_12: {
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV12().writeChunk(chunkData, &managerOut);
                break;
            }
            default:;
        }

        const Data outData(managerOut.getPosition());
        memcpy(outData.data, outBuffer.data, outData.size);
        outBuffer.deallocate();
        deallocate();

        data = outData.data;
        size = outData.size;
        setDecSize(size);
    }


    // TODO: rewrite to return status
    void ChunkManager::ensureDecompress(const CONSOLE console) {
        if (getCompressed() == 0U || console == CONSOLE::NONE || data == nullptr || size == 0) {
            return;
        }
        setCompressed(0U);

        u32 dec_size_copy = getDecSize();
        Data decompData(getDecSize());

        int result;
        switch (console) {
            case CONSOLE::XBOX360:
                dec_size_copy = XDecompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::RPCS3:
            case CONSOLE::PS3:
                result = tinf_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::SWITCH:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
                result = tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            default:
                break;
        }

        deallocate();

        if (getRLE() != 0U) {
            allocate(getDecSize());
            RLE_decompress(decompData.start(), decompData.size, start(), dec_size_copy);
            decompData.deallocate();
        } else {
            data = decompData.data;
            size = dec_size_copy;
            decompData.reset();
        }
    }


    // TODO: rewrite to return status
    void ChunkManager::ensureCompressed(const CONSOLE console) {
        if (getCompressed() != 0U || console == CONSOLE::NONE || data == nullptr || size == 0) {
            return;
        }
        setCompressed(1U);
        setDecSize(size);

        if (getRLE() != 0U) {
            Data rleBuffer(size);
            RLE_compress(data, size, rleBuffer.data, rleBuffer.size);
            deallocate();
            data = rleBuffer.data;
            size = rleBuffer.size;
            rleBuffer.reset();
        }

        // allocate memory and recompress
        auto *const comp_ptr = new u8[size];
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
                if (const int status = compress(comp_ptr, &comp_size, data, size); status != 0) {
                    printf("error has occurred compressing chunk\n");
                }

                deallocate();
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            }

            case CONSOLE::SWITCH:
            case CONSOLE::WIIU:
            case CONSOLE::VITA: {
                if (const int status = compress(comp_ptr, &comp_size, data, size); status != 0) {
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

}