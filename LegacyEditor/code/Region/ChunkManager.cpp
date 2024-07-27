#include "ChunkManager.hpp"

#include <cstring>

#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"

#include "lce/processor.hpp"

#include "LegacyEditor/utils/XBOX_LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"

#include "LegacyEditor/code/Chunk/v10.hpp"
#include "LegacyEditor/code/Chunk/v11.hpp"
#include "LegacyEditor/code/Chunk/v12.hpp"
#include "LegacyEditor/code/Chunk/v13.hpp"
#include "LegacyEditor/code/Chunk/chunkData.hpp"


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


    int ChunkManager::checkVersion() const {
        if (this->data == nullptr) {
            return -1;
        }
        DataManager checker(data, size);
        int version = checker.readInt16AtOffset(0);
        return version;

    }


    MU void ChunkManager::readChunk(MU const lce::CONSOLE inConsole) const {
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
        Data outBuffer;
        outBuffer.allocate(CHUNK_BUFFER_SIZE);
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

        Data outData;
        outData.allocate(managerOut.getPosition());
        std::memcpy(outData.data, outBuffer.data, outData.size);
        outBuffer.deallocate();
        deallocate();

        data = outData.data;
        size = outData.size;
        fileData.setDecSize(size);
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureDecompress(lce::CONSOLE consoleIn) {
        if (fileData.getCompressedFlag() == 0U
            /*|| console == lce::CONSOLE::NONE*/
            || data == nullptr
            || size == 0) {
            return SUCCESS;
        }

        fileData.setCompressedFlag(0U);

        u32 dec_size = fileData.getDecSize();
        Data decompData;
        decompData.setScopeDealloc(true);
        decompData.allocate(fileData.getDecSize());

        // TODO: XBOX1 case is not handled
        int result = SUCCESS;
        switch (consoleIn) {
            case lce::CONSOLE::XBOX360: {
                u8 *ptr = data;
                dec_size = XDecompress(
                        decompData.start(), &decompData.size, ptr, size);
                decompData.size = dec_size;
                break;
            }
            case lce::CONSOLE::RPCS3:
            case lce::CONSOLE::PS3: {
                result = tinf_uncompress(
                        decompData.start(), &decompData.size, data, size);
                break;
            }
            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA:
            case lce::CONSOLE::PS4:
                result = tinf_zlib_uncompress(
                        decompData.start(), &decompData.size, data, size);
                break;
            default:
                break;
        }

        deallocate();

        if (fileData.getRLEFlag() != 0U) {
            allocate(fileData.getRLESize());
            RLE_decompress(decompData.start(),
                decompData.size, start(), dec_size);

            decompData.deallocate();

        } else {
            data = decompData.data;
            size = dec_size;
            decompData.reset();
        }

        return result;
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureCompressed(const lce::CONSOLE console) {
        if (fileData.getCompressedFlag() != 0U
            || console == lce::CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return SUCCESS;
        }
        fileData.setCompressedFlag(1U);
        fileData.setDecSize(size);

        if (fileData.getRLEFlag() != 0U) {
            Data rleBuffer;
            rleBuffer.allocate(size);
            RLE_compress(data, size, rleBuffer.data, rleBuffer.size);
            deallocate();
            data = rleBuffer.data;
            size = rleBuffer.size;
            fileData.setRLESize(size);
            rleBuffer.reset();
        }

        // allocate memory and recompress


        // TODO: Does it work for vita?
        int status = 0;
        switch (console) {
            case lce::CONSOLE::XBOX360:
                printf("trying to write xbox360 chunk with ChunkManager::ensureCompressed, not supported yet\n");
                // TODO: leaks memory
                // XCompress(comp_ptr, comp_size, data_ptr, data_size);
                break;

            case lce::CONSOLE::PS3:
            case lce::CONSOLE::RPCS3: {
                auto *comp_ptr = new u8[size];
                uLongf comp_size = size;

                status = compress(comp_ptr, &comp_size, data, size);
                deallocate();
                if (status != 0) {
                    printf("error has occurred compressing chunk\n");
                    return MALLOC_FAILED;
                }

                // copy it over, and remove ZLIB header
                data = new u8[comp_size - 2];
                size = comp_size - 2;
                std::memcpy(data, comp_ptr + 2, size);
                // zero out ending integrity check, as the console does
                // std::memset(data + comp_size - 6, 0, 4);

                delete[] comp_ptr;
                comp_size = 0;
                break;
            }

            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::PS4:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA: {
                auto* comp_ptr = new u8[size];
                uLongf comp_size = size;

                status = compress(comp_ptr, &comp_size, data, size);
                deallocate();
                if (status != 0) {
                    printf("error has occurred compressing chunk\n");
                    return MALLOC_FAILED;
                }
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            }
            default:
                break;
        }

        return SUCCESS;
    }


    void ChunkManager::setSizeFromReading(c_u32 sizeIn) {
        fileData.setRLEFlag(sizeIn >> 31);
        fileData.setUnknownFlag(sizeIn >> 30 & 1);
        size = sizeIn & 0x00FFFFFF;
    }


    u32 ChunkManager::getSizeForWriting() const {
        u32 sizeOut = size;
        if (fileData.getRLEFlag() != 0U) { sizeOut |= 0x80000000; }
        if (fileData.getUnknownFlag() != 0U) { sizeOut |= 0x40000000; }
        return sizeOut;
    }


}