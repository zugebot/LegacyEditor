#include "ChunkManager.hpp"

#include <cstring>

#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"

#include "include/lce/processor.hpp"

#include "common/RLE/rle.hpp"
#include "common/XBOX_LZX/XDecompress.hpp"

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/v10.hpp"
#include "code/Chunk/v11.hpp"
#include "code/Chunk/v12.hpp"
#include "code/Chunk/v13.hpp"


namespace editor {


    enum CHUNK_HEADER : i16 {
        V_NBT = 0x0A00,
        V_8   = 0x0008,
        V_9   = 0x0009,
        V_10  = 0x000A,
        V_11  = 0x000B,
        V_12  = 0x000C,
        V_13  = 0x000D,
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
        int version = checker.readAtOffset<u16>(0);
        return version;

    }


    MU void ChunkManager::readChunk(MU const lce::CONSOLE inConsole) {
        // cannot read chunk if there is no data
        if (size == 0) {
            return;
        }
        // if the file is compressed, decompress it first
        if (fileData.getCompressedFlag()) {
            ensureDecompress(inConsole);
        }
        // read the chunk
        DataManager managerIn(data, size);

        chunkData->lastVersion = managerIn.read<u16>();

        switch(chunkData->lastVersion) {
            case V_NBT:
                chunkData->lastVersion = V_10;
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


        u32 length = managerIn.ptr() - data;
        managerIn.update(managerIn.start(), length);
        // managerIn.update();
        // managerIn.size
        // managerIn.m_ptr = data;
        if (chunkData->chunkX == 0 && chunkData->chunkZ == -10) {
            managerIn.writeToFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\chunks\0_-10.read)");
        }
    }


    MU void ChunkManager::writeChunk(MU lce::CONSOLE outConsole) {
        Data outBuffer;
        outBuffer.allocate(CHUNK_BUFFER_SIZE);
#ifndef DONT_MEMSET0
        memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
#endif
        DataManager managerOut(outBuffer);


        switch (chunkData->lastVersion) {
            case V_10:
                chunk::ChunkV10(chunkData, &managerOut).writeChunk();
                break;
            case V_8:
            case V_9:
            case V_11:
                managerOut.write<u16>(chunkData->lastVersion);
                chunk::ChunkV11(chunkData, &managerOut).writeChunk();
                break;
            case V_12:
                managerOut.write<u16>(chunkData->lastVersion);
                chunk::ChunkV12(chunkData, &managerOut).writeChunk();
                break;
            case V_13:
                printf("ChunkManager::writeChunk v13 forbidden\n");
                exit(-1);
            default:;
        }

        // if (chunkData->chunkX == 0 && chunkData->chunkZ == -10) {
        //     managerOut.writeToFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\chunks\0_-10.write)");
        // }

        Data outData;
        outData.allocate(managerOut.tell());
        std::memcpy(outData.data, outBuffer.data, outData.size);
        outBuffer.deallocate();

        deallocate();
        data = outData.data;
        size = outData.size;

        fileData.setDecSize(size);
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureDecompress(lce::CONSOLE consoleIn, bool skipRLE) {
        if (fileData.getCompressedFlag() == 0U
            || data == nullptr
            || size == 0) {
            return SUCCESS;
        }

        u32 dec_size = fileData.getDecSize();
        Data decompData;
        decompData.allocate(fileData.getDecSize());


        int result = SUCCESS;
        switch (consoleIn) {
            case lce::CONSOLE::XBOX360: {
                u8 *ptr = data;
                result = XDecompress(
                        decompData.start(), &decompData.size, ptr, size);
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

        fileData.setCompressedFlag(0U);


        if (fileData.getRLEFlag() == 1U && !skipRLE) {
            deallocate();
            allocate(fileData.getRLESize());
            RLE_decompress(decompData.start(),
                decompData.size, start(), dec_size);

            fileData.setRLEFlag(0U);
            decompData.deallocate();

        } else {
            steal(decompData);
            if (!skipRLE) { size = dec_size; }
        }

        return result;
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureCompressed(const lce::CONSOLE console, bool skipRLE) {
        if (fileData.getCompressedFlag() != 0U
            || console == lce::CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return SUCCESS;
        }
        fileData.setCompressedFlag(1U);
        fileData.setDecSize(size);

        if (fileData.getRLEFlag() == 0U && !skipRLE) {
            Data rleBuffer;
            rleBuffer.allocate(size);
            RLE_compress(data, size, rleBuffer.data, rleBuffer.size);
            steal(rleBuffer);

            fileData.setRLESize(size);
            fileData.setRLEFlag(1);
        }

        // allocate memory and recompress
        int status = INVALID_CONSOLE;
        switch (console) {
            case lce::CONSOLE::XBOX360:
                printf("trying to write xbox360 chunk with ChunkManager::ensureCompressed, "
                       "not supported yet\n");
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
                auto comp_size = (uLongf)(float(size) * 1.25F);

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

        return status;
    }


    void ChunkManager::setSizeFromReading(c_u32 sizeIn) {
        fileData.setRLEFlag(sizeIn >> 31);
        fileData.setNewSaveFlag(sizeIn >> 30 & 1);
        size = sizeIn & 0x00FFFFFF;
    }


    u32 ChunkManager::getSizeForWriting() const {
        u32 sizeOut = size;
        if (fileData.getRLEFlag() != 0U) { sizeOut |= 0x80000000; }
        if (fileData.getNewSaveFlag() != 0U) { sizeOut |= 0x40000000; }
        return sizeOut;
    }


}