#include "chunkHandle.hpp"

#include <xdecompress.h>

#include "include/lce/processor.hpp"

#include "common/rle/rle.hpp"
#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"

#include "code/SaveFile/writeSettings.hpp"

#include "code/chunk/formats/chunkFormatNBT.hpp"
#include "code/chunk/formats/chunkFormatGridPaletted.hpp"
#include "code/chunk/formats/chunkFormatGridPalettedSubmerged.hpp"


// TODO: WHATEVER I DID, IT'S NO LONGER CONVERTING CHUNKS, FAILING SILENTLY


namespace editor {


    void ChunkHandle::unpackSizeFlags(c_u32 word) {
        header.setRle(word >> 31);
        header.setNewSave((word >> 30) & 1);
        *buffer.size_ptr() = word & 0x0FFFFFFF;
    }


    u32 ChunkHandle::packSizeFlags() const {
        u32 word = buffer.size();
        if (header.rle())     { word |= 0x8000'0000u; }
        if (header.newSave()) { word |= 0x4000'0000u; }
        return word;
    }


    int ChunkHandle::read(DataReader& rdr, lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::read refuses to take \"lce::CONSOLE::NONE\" as argument");
        }

        unpackSizeFlags(rdr.read<u32>());

        if (!buffer.allocate(buffer.size(), true))
            return STATUS::MALLOC_FAILED;

        if (lce::is_ps3_family(c)/*c == lce::CONSOLE::PS3 || c == lce::CONSOLE::RPCS3*/) {
            header.setDecSize(rdr.read<u32>());
            header.setRLESize(rdr.read<u32>());
        } else {
            u32 sz = rdr.read<u32>();
            header.setDecSize(sz);
            header.setRLESize(sz);
        }

        std::memcpy(buffer.data(), rdr.ptr(), buffer.size());

        header.markWritten();
        header.markDirty(false);
        return SUCCESS;
    }


    int ChunkHandle::write(DataWriter& wtr, lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::write refuses to take \"lce::CONSOLE::NONE\" as argument");
        }

        wtr.write<u32>(packSizeFlags());

        if (lce::is_ps3_family(c)/*c == lce::CONSOLE::PS3 || c == lce::CONSOLE::RPCS3*/) {
            wtr.write<u32>(header.getDecSize());
            wtr.write<u32>(header.getRLESize());
        } else {
            wtr.write<u32>(header.getDecSize());
        }
        wtr.writeBytes(buffer.data(), buffer.size());

        header.markWritten();
        header.markDirty(false);
        return SUCCESS;
    }


    MU int ChunkHandle::decodeChunk(MU lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::decodeChunk refuses to take \"lce::CONSOLE::NONE\" as argument");
        }

        if (!header.written())
            return STATUS::ALREADY_WRITTEN;

        Buffer decZip;
        decZip.allocate(
                header.rle() ? header.getRLESize() : header.getDecSize()
        );
        if (decZip.empty())
            return SUCCESS;

        if (lce::is_xbox360_family(c)) {
            int error = xdecompress(
                    decZip.data(), decZip.size_ptr(),
                    buffer.data(), buffer.size());
            if (error) {
                return DECOMPRESS;
            }

        } else if (lce::is_ps3_family(c)) {
            int result = tinf_uncompress(
                    decZip.data(), decZip.size_ptr(),
                    buffer.data(), buffer.size());
            if (result != SUCCESS)
                return STATUS::DECOMPRESS;

        } else if (lce::is_wiiu_family(c) ||
                   lce::is_psvita_family(c) ||
                   lce::is_ps4_family(c) ||
                   lce::is_switch_family(c) ||
                   lce::is_newgen_family(c) ||
                   lce::is_xbox1_family(c)) {
            int result = tinf_zlib_uncompress(
                    decZip.data(), decZip.size_ptr(),
                    buffer.data(), buffer.size());
            if (result != SUCCESS)
                return STATUS::DECOMPRESS;
        } else {
            throw std::runtime_error("Chunk uncompress pipeline received unhandled console case!");
        }

        if (header.rle() == true) {
            buffer.clear();
            buffer.allocate(header.getDecSize());
            codec::RLE_decompress(
                    buffer.data(), buffer.size_ptr(),
                    decZip.data(), decZip.size());
            header.setRle(false);
            decZip.clear();
        } else {
            buffer = std::move(decZip);
        }

        // read the chunk
        DataReader reader(buffer.span(), Endian::Big);

        if (reader.peek() == 0x0A) { // start of NBT
            data->lastVersion = eChunkVersion::V_NBT;
            ChunkFormatNBT::readChunk(data.get(), reader);

        } else {
            data->lastVersion = reader.read<u16>();
            switch(data->lastVersion) {
                case eChunkVersion::V_8:
                case eChunkVersion::V_9:
                case eChunkVersion::V_10:
                case eChunkVersion::V_11:
                    ChunkFormatGridPaletted::readChunk(data.get(), reader);
                    break;
                case eChunkVersion::V_12:
                case eChunkVersion::V_13:
                    ChunkFormatGridPalettedSubmerged::readChunk(data.get(), reader);
                    break;
                default:;
            }
        }

        header.markDirty();
        header.clearWritten();
        return SUCCESS;
    }


    MU int ChunkHandle::encodeChunk(WriteSettings& settings) {
        if (lce::is_console_none(settings.m_schematic.save_console))
            throw std::runtime_error("ChunkHandle::encodeChunk refuses to take \"lce::CONSOLE::NONE\" as argument");

        if (header.written())
            return STATUS::ALREADY_WRITTEN;

        if (buffer.empty())
            return STATUS::SUCCESS;

        DataWriter wtr(256, Endian::Big);

        switch (settings.m_schematic.chunk_lastVersion) {
            case eChunkVersion::V_NBT:
                ChunkFormatNBT::writeChunk(data.get(), settings, wtr);
                break;
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_10:
            case eChunkVersion::V_11:
                wtr.write<u16>(settings.m_schematic.chunk_lastVersion);
                ChunkFormatGridPaletted::writeChunk(data.get(), settings, wtr);
                break;
            case eChunkVersion::V_12:
            case eChunkVersion::V_13:
                data->lastVersion = 12;
                wtr.write<u16>(settings.m_schematic.chunk_lastVersion);
                ChunkFormatGridPalettedSubmerged::writeChunk(data.get(), settings, wtr);
                break;
            default:
                break;
        }

        buffer = std::move(wtr.take());
        header.setDecSize(buffer.size());

        if (!header.rle()) {
            Buffer rleBuffer;
            c_u32 safe_size = codec::RLE_safe_compress_size(buffer.size());
            rleBuffer.allocate(safe_size);
            codec::RLE_compress(rleBuffer.data(), rleBuffer.size_ptr(),
                                buffer.data(), buffer.size());
            buffer = std::move(rleBuffer);

            header.setRLESize(buffer.size());
            header.setRle(true);
        }


        const auto c = settings.m_schematic.save_console;
        if (lce::is_xbox360_family(c)) {
            return STATUS::NOT_IMPLEMENTED;

        } else if (lce::is_ps3_family(c)) {
            Buffer compressed((u32)(float(buffer.size()) * 1.25F));
            int status = compress(compressed.data(), (uLongf*) compressed.size_ptr(),
                                  buffer.data(), buffer.size());
            buffer.clear();
            if (status)
                return STATUS::COMPRESS;
            // copy it over, and remove ZLIB header
            buffer = Buffer(compressed.size() - 2);
            std::memcpy(buffer.data(), compressed.data() + 2, buffer.size());
            // zero out ending integrity check, as the console does
            // std::memset(data + comp_size - 6, 0, 4);

        } else if (lce::is_wiiu_family(c) ||
                   lce::is_psvita_family(c) ||
                   lce::is_ps4_family(c) ||
                   lce::is_switch_family(c) ||
                   lce::is_newgen_family(c) ||
                   lce::is_xbox1_family(c)) {
            Buffer compressed((u32)(float(buffer.size()) * 1.25F));
            int status = compress(compressed.data(), (uLongf*) compressed.size_ptr(),
                                  buffer.data(), buffer.size());
            buffer.clear();
            if (status)
                return STATUS::COMPRESS;
            buffer = std::move(compressed);
        } else {
            throw std::runtime_error("Chunk uncompress pipeline received unhandled console case!");
        }

        header.markDirty(false);
        header.markWritten();

        return SUCCESS;
    }

}