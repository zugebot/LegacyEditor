#include "ChunkManager.hpp"

#include "include/lce/processor.hpp"

#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"


#include "common/RLE/rle.hpp"
#include "common/codec/XDecompress.hpp"

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/v10.hpp"
#include "code/Chunk/v11.hpp"
#include "code/Chunk/v12.hpp"


namespace editor {





    ChunkManager::ChunkManager() {
        chunkData = new chunk::ChunkData();
        this->buffer.clear();
    }


    ChunkManager::~ChunkManager() {
        delete chunkData;
        chunkData = nullptr;
    }


    MU int ChunkManager::checkVersion() const {
        if (this->buffer.empty()) {
            return -1;
        }
        DataReader checker(buffer.data(), buffer.size());
        int version = checker.peek_at<u16>(0);
        return version;

    }


    MU void ChunkManager::readChunk(MU const lce::CONSOLE inConsole) {
        // if the file is compressed, decompress it first
        if (chunkHeader.isZipCompressed()) {
            int status = ensureDecompress(inConsole);
            if (status != SUCCESS) {
                return;
            }
        }
        // cannot read chunk if there is no data
        if (buffer.empty()) {
            return;
        }
        // read the chunk
        DataReader reader(buffer.span());

        chunkData->lastVersion = reader.read<u16>();
        if (chunkData->lastVersion == 0x0A00) { // start of NBT
            chunkData->lastVersion = chunk::eChunkVersion::V_NBT;
            reader.rewind();
        }

        switch(chunkData->lastVersion) {
            case chunk::eChunkVersion::V_NBT:
                chunk::ChunkVNBT(chunkData).readChunk(reader);
                break;
            case chunk::eChunkVersion::V_8: 
            case chunk::eChunkVersion::V_9: 
            case chunk::eChunkVersion::V_11:
                chunk::ChunkV11(chunkData).readChunk(reader);
                break;
            case chunk::eChunkVersion::V_12:
            case chunk::eChunkVersion::V_13:
                chunk::ChunkV12(chunkData).readChunk(reader);
                break;
            default:;
        }



        /*
        if (chunkData->chunkX == 0 && chunkData->chunkZ == -10) {
            try {
                DataWriter::writeFile(R"(C:\Users\jerrin\CLionProjects\LegacyEditor\chunks\0_-10.read)",
                                      reader.span());
            } catch (const std::exception& e) {

            }
        }
         */
    }


    MU void ChunkManager::writeChunk(MU lce::CONSOLE outConsole) {
        if (chunkHeader.isZipCompressed()) {
            return;
        }


//         Buffer outBuffer;
//         outBuffer.allocate(CHUNK_BUFFER_SIZE);
// #ifndef DONT_MEMSET0
//         memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
// #endif
        DataWriter writer;


        switch (chunkData->lastVersion) {
            case chunk::eChunkVersion::V_NBT:
                chunk::ChunkVNBT(chunkData).writeChunk(writer);
                break;
            case chunk::eChunkVersion::V_8:
            case chunk::eChunkVersion::V_9:
            case chunk::eChunkVersion::V_11:
                writer.write<u16>(chunkData->lastVersion);
                chunk::ChunkV11(chunkData).writeChunk(writer);
                break;
            case chunk::eChunkVersion::V_12:
            case chunk::eChunkVersion::V_13:
                chunkData->lastVersion = 12;
                writer.write<u16>(chunkData->lastVersion);
                chunk::ChunkV12(chunkData).writeChunk(writer);
                break;
            default:
                break;
        }

        buffer = std::move(writer.take());

        chunkHeader.setDecSize(buffer.size());

        if (!chunkHeader.isZipCompressed()) {
            int status = ensureCompressed(outConsole);
            if (status != SUCCESS) {
                return;
            }
        }
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureDecompress(lce::CONSOLE console) {
        if (chunkHeader.isZipCompressed() == 0U
            || buffer.empty()) {
            return SUCCESS;
        }

        Buffer decompressedZip;
        if (chunkHeader.isRLECompressed()) {
            decompressedZip.allocate(chunkHeader.getRLESize());
        } else {
            decompressedZip.allocate(chunkHeader.getDecSize());
        }


        int result = SUCCESS;
        switch (console) {
            case lce::CONSOLE::XBOX360: {
                codec::XmemErr err = codec::XDecompress(buffer.data(), buffer.size(),
                                                        decompressedZip.data(), decompressedZip.size_ptr());
                if (err != codec::XmemErr::Ok) {
                    result = -1;
                }
                break;
            }
            case lce::CONSOLE::RPCS3:
            case lce::CONSOLE::PS3: {
                result = tinf_uncompress(
                        decompressedZip.data(), decompressedZip.size_ptr(),
                        buffer.data(), buffer.size());
                break;
            }
            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA:
            case lce::CONSOLE::PS4:
            case lce::CONSOLE::XBOX1:
            case lce::CONSOLE::WINDURANGO:
                result = tinf_zlib_uncompress(
                        decompressedZip.data(), decompressedZip.size_ptr(),
                        buffer.data(), buffer.size());
                break;
            default:
                break;
        }
        if (result != SUCCESS) {
            return STATUS::DECOMPRESS;
        }
        chunkHeader.setZipCompressed(0U);


        if (chunkHeader.isRLECompressed() == 1U) {
            buffer.clear();
            buffer.allocate(chunkHeader.getDecSize());
            // TODO: why is this crashing, what did I do
            codec::RLE_decompress(decompressedZip.data(), decompressedZip.size(),
                                  buffer.data(), buffer.size_ref());
            chunkHeader.setRLECompressed(0U);
            decompressedZip.clear();
        } else {
            buffer = std::move(decompressedZip);
        }

        return result;
    }


    // TODO: rewrite to return status
    int ChunkManager::ensureCompressed(const lce::CONSOLE console) {
        if (chunkHeader.isZipCompressed() != 0U
            || console == lce::CONSOLE::NONE
            || buffer.empty()) {
            return SUCCESS;
        }

        chunkHeader.setZipCompressed(1U);
        chunkHeader.setDecSize(buffer.size());


        if (chunkHeader.isRLECompressed() == 0U) {
            Buffer rleBuffer;
            rleBuffer.allocate(buffer.size());
            codec::RLE_compress(buffer.data(), buffer.size(), rleBuffer.data(), rleBuffer.size_ref());
            buffer = std::move(rleBuffer);

            chunkHeader.setRLESize(buffer.size());
            chunkHeader.setRLECompressed(1);
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
                Buffer compressed(buffer.size());
                status = compress(compressed.data(), (uLongf*) compressed.size_ptr(),
                                  buffer.data(), buffer.size());
                buffer.clear();
                if (status != 0) {
                    printf("error has occurred compressing chunk\n");
                    return MALLOC_FAILED;
                }
                // copy it over, and remove ZLIB header
                buffer = Buffer(compressed.size() - 2);
                std::memcpy(buffer.data(), compressed.data() + 2, buffer.size());
                // zero out ending integrity check, as the console does
                // std::memset(data + comp_size - 6, 0, 4);
                break;
            }

            case lce::CONSOLE::SWITCH:
            case lce::CONSOLE::PS4:
            case lce::CONSOLE::WIIU:
            case lce::CONSOLE::VITA:
            case lce::CONSOLE::XBOX1:
            case lce::CONSOLE::WINDURANGO: {
                Buffer compressed((u32)(float(buffer.size()) * 1.25F));
                status = compress(compressed.data(), (uLongf*) compressed.size_ptr(),
                                  buffer.data(), buffer.size());
                buffer.clear();
                if (status != 0) {
                    printf("error has occurred compressing chunk\n");
                    return MALLOC_FAILED;
                }
                buffer = std::move(compressed);
                break;
            }
            default:
                break;
        }

        return status;
    }


    void ChunkManager::setVariableFlags(c_u32 sizeIn) {
        chunkHeader.setRLECompressed(sizeIn >> 31);
        chunkHeader.setNewSaveFlag((sizeIn >> 30) & 1);
        *buffer.size_ptr() = sizeIn & 0x3FFFFFFF;
    }


    u32 ChunkManager::getSizeForWriting() const {
        u32 sizeOut = buffer.size();
        if (chunkHeader.isRLECompressed() != 0U) { sizeOut |= 0x80000000; }
        if (chunkHeader.getNewSaveFlag() != 0U) { sizeOut |= 0x40000000; }
        return sizeOut;
    }


    int ChunkManager::read(DataReader& reader, lce::CONSOLE console) {
        setVariableFlags(reader.read<u32>());
        bool status = buffer.allocate(buffer.size());
        if (!status) {
            printf("Failed to allocate %d bytes for chunk", buffer.size());
            return STATUS::MALLOC_FAILED;
        }
        // TODO: is this needed
        std::memset(buffer.data(), 0, buffer.size());

        switch (console) {
            case lce::CONSOLE::PS3:
            case lce::CONSOLE::RPCS3: {
                chunkHeader.setDecSize(reader.read<u32>());
                chunkHeader.setRLESize(reader.read<u32>());
                break;
            }
            default:
                c_u32 dec_and_rle_size = reader.read<u32>();
                chunkHeader.setDecSize(dec_and_rle_size);
                chunkHeader.setRLESize(dec_and_rle_size);
                break;
        }
        std::memcpy(buffer.data(), reader.ptr(), buffer.size());
        return SUCCESS;
    }


    int ChunkManager::write(DataWriter& writer, lce::CONSOLE console) {
        writer.write<u32>(getSizeForWriting());
        switch (console) {
            case lce::CONSOLE::PS3:
            case lce::CONSOLE::RPCS3:
                writer.write<u32>(chunkHeader.getDecSize());
                writer.write<u32>(chunkHeader.getRLESize());
                break;
            default:
                writer.write<u32>(chunkHeader.getDecSize());
                break;
        }
        writer.writeBytes(buffer.data(), buffer.size());
        return SUCCESS;
    }


}