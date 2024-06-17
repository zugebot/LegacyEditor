#include "ChunkManager.hpp"

#include "lce/processor.hpp"

#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"

#include "LegacyEditor/utils/XBOX_LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/code/Chunk/chunkData.hpp"

#include "LegacyEditor/code/Chunk/v10.hpp"
#include "LegacyEditor/code/Chunk/v11.hpp"
#include "LegacyEditor/code/Chunk/v12.hpp"
#include "LegacyEditor/code/Chunk/v13.hpp"


namespace editor {


    enum CHUNK_HEADER : i16 {
        V_8 = 0x0008,
        V_9 = 0x0009,
        V_NBT = 0x0A00,
        V_11 = 0x000B,
        V_12 = 0x000C,
        V_13 = 0x000D,
    };


    ChunkManager::ChunkManager() {
        chunkData = new chunk::ChunkData();
    }


    ChunkManager::~ChunkManager() {
        delete chunkData;
    }


    MU void ChunkManager::readChunk(MU const lce::CONSOLE console) const {
        auto managerIn = DataManager(data, size);
        managerIn.seekStart();

        chunkData->lastVersion = managerIn.readInt16();

        switch(chunkData->lastVersion) {
            case V_NBT:
                chunkData->lastVersion = V_11;
                chunk::ChunkV10(chunkData, &managerIn).readChunk();
                break;
            case V_8: case V_9: case V_11:
                chunk::ChunkV11(chunkData, &managerIn).readChunk();
                break;
            case V_12:
                chunk::ChunkV12(chunkData, &managerIn).readChunk();
                break;
            case V_13:
                chunk::ChunkV13(chunkData, &managerIn).readChunk();
                break;
            default:;
        }
    }


    MU void ChunkManager::writeChunk(MU lce::CONSOLE console) {
        Data outBuffer(CHUNK_BUFFER_SIZE);
        memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
        auto managerOut = DataManager(outBuffer);

        managerOut.seekStart();
        switch (chunkData->lastVersion) {
            case V_NBT:
                chunk::ChunkV10(chunkData, &managerOut).writeChunk();
            break; case V_8: case V_9: case V_11:
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV11(chunkData, &managerOut).writeChunk();
            break; case V_12:
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV12(chunkData, &managerOut).writeChunk();
            break; case V_13:
                printf("ChunkManager::writeChunk v13 forbidden\n");
                exit(-1);
            default:;
        }

        const Data outData(managerOut.getPosition());
        memcpy(outData.data, outBuffer.data, outData.size);
        outBuffer.deallocate();
        deallocate();

        data = outData.data;
        size = outData.size;
        fileData.setDecSize(size);
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureDecompress(const lce::CONSOLE console) {
        if (fileData.getCompressed() == 0U
            || console == lce::CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return SUCCESS;
        }
        fileData.setCompressed(0U);

        u32 dec_size_copy = fileData.getDecSize();
        Data decompData(fileData.getDecSize());

        // TODO: XBOX1 case is not handled
        int result = SUCCESS;
        switch (console) {
            case lce::CONSOLE::XBOX360:
                dec_size_copy = XDecompress(decompData.start(), &decompData.size, data, size);
                break;
            case lce::CONSOLE::RPCS3:
            case lce::CONSOLE::PS3:
                result = tinf_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA:
            case lce::CONSOLE::PS4:
                result = tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            default:
                break;
        }

        deallocate();

        if (fileData.getRLE() != 0U) {
            allocate(fileData.getDecSize());
            RLE_decompress(decompData.start(),
                decompData.size, start(), dec_size_copy);
            decompData.deallocate();
        } else {
            data = decompData.data;
            size = dec_size_copy;
            decompData.reset();
        }
        return result;
    }


    // TODO: rewrite to return status
    void ChunkManager::ensureCompressed(const lce::CONSOLE console) {
        if (fileData.getCompressed() != 0U
            || console == lce::CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return;
        }
        fileData.setCompressed(1U);
        fileData.setDecSize(size);

        if (fileData.getRLE() != 0U) {
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
        int status;
        switch (console) {
            case lce::CONSOLE::XBOX360:
                // TODO: leaks memory
                // XCompress(comp_ptr, comp_size, data_ptr, data_size);
                break;
            case lce::CONSOLE::PS3:
                // TODO: leaks memory
                // tinf_compress(comp_ptr, comp_size, data_ptr, data_size);
                break;

            case lce::CONSOLE::RPCS3: {
                if (status = compress(comp_ptr, &comp_size, data, size); status != 0) {
                    printf("error has occurred compressing chunk\n");
                }
                deallocate();
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            }

            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::PS4:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA:
                if (status = compress(comp_ptr, &comp_size, data, size); status != 0) {
                    printf("error has occurred compressing chunk\n");
                }
                deallocate();
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            default:
                break;
        }

    }


    void ChunkManager::setSizeFromReading(c_u32 sizeIn) {
        fileData.setRLE(sizeIn >> 31);
        fileData.setUnknown(sizeIn >> 30 & 1);
        size = sizeIn & 0x00FFFFFF;
    }


    u32 ChunkManager::getSizeForWriting() const {
        u32 sizeOut = size;
        if (fileData.getRLE() != 0U) { sizeOut |= 0x80000000; }
        if (fileData.getUnknown() != 0U) { sizeOut |= 0x40000000; }
        return sizeOut;
    }


}