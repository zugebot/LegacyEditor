#include "chunkHandle.hpp"

#ifdef SUPPORT_XBOX360
#include <xdecompress.h>
#endif

#include <cstring>
#include <stdexcept>

#include "include/lce/processor.hpp"

#include "common/rle/rle.hpp"
#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zlib.h"

#include "common/data/DataReader.hpp"
#include "common/data/DataWriter.hpp"

#include "code/SaveFile/writeSettings.hpp"
#include "code/chunk/formats/chunkFormatNBT.hpp"
#include "code/chunk/formats/chunkFormatGridPaletted.hpp"
#include "code/chunk/formats/chunkFormatGridPalettedSubmerged.hpp"

namespace editor {

    ChunkHandle::ChunkHandle(ChunkHandle&& other) noexcept
        : header(other.header),
          buffer(std::move(other.buffer)),
          data{this},
          m_state(other.m_state),
          m_srcConsole(other.m_srcConsole),
          m_decoded(std::move(other.m_decoded)) {
        other.m_state = ChunkState::EMPTY;
        other.m_srcConsole = lce::CONSOLE::NONE;
        other.header.markDirty(false);
        other.buffer.clear();
        other.m_decoded.reset();
        other.resetProxyOwner();
        resetProxyOwner();
    }

    ChunkHandle& ChunkHandle::operator=(ChunkHandle&& other) noexcept {
        if (this == &other) return *this;

        header = other.header;
        buffer = std::move(other.buffer);
        m_state = other.m_state;
        m_srcConsole = other.m_srcConsole;
        m_decoded = std::move(other.m_decoded);

        other.m_state = ChunkState::EMPTY;
        other.m_srcConsole = lce::CONSOLE::NONE;
        other.header.markDirty(false);
        other.buffer.clear();
        other.m_decoded.reset();

        other.resetProxyOwner();
        resetProxyOwner();
        return *this;
    }

    void ChunkHandle::setEmpty() {
        header.setDecSize(0);
        header.setRLESize(0);
        header.markDirty(false);
        header.setRle(true);

        buffer.clear();
        m_decoded.reset();
        m_state = ChunkState::EMPTY;
    }

    void ChunkHandle::unpackSizeFlags(c_u32 word) {
        header.setRle((word >> 31) != 0);
        header.setNewSave(((word >> 30) & 1) != 0);
        *buffer.size_ptr() = word & 0x0FFFFFFF;
    }

    u32 ChunkHandle::packSizeFlags() const {
        u32 word = buffer.size();
        if (header.rle())     word |= 0x8000'0000u;
        if (header.newSave()) word |= 0x4000'0000u;
        return word;
    }

    int ChunkHandle::read(DataReader& rdr, lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::read refuses to take lce::CONSOLE::NONE");
        }

        m_srcConsole = c;

        unpackSizeFlags(rdr.read<u32>());

        const u32 compSize = buffer.size();

        if (lce::is_ps3_family(c)) {
            header.setDecSize(rdr.read<u32>());
            header.setRLESize(rdr.read<u32>());
        } else {
            const u32 sz = rdr.read<u32>();
            header.setDecSize(sz);
            header.setRLESize(sz); // non-PS3 files don't store rleSize separately in your format
        }

        if (compSize == 0 || header.getDecSize() == 0) {
            setEmpty();
            return SUCCESS;
        }

        if (!buffer.allocate(compSize, true)) return STATUS::MALLOC_FAILED;
        std::memcpy(buffer.data(), rdr.ptr(), compSize);

        header.markDirty(false);
        m_decoded.reset();
        m_state = ChunkState::COMPRESSED;
        return SUCCESS;
    }

    int ChunkHandle::write(DataWriter& wtr, lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::write refuses to take lce::CONSOLE::NONE");
        }

        if (m_state != ChunkState::COMPRESSED) {
            throw std::runtime_error("ChunkHandle::write called while not in COMPRESSED state");
        }

        wtr.write<u32>(packSizeFlags());

        if (lce::is_ps3_family(c)) {
            wtr.write<u32>(header.getDecSize());
            wtr.write<u32>(header.getRLESize());
        } else {
            wtr.write<u32>(header.getDecSize());
        }

        if (!buffer.empty()) {
            wtr.writeBytes(buffer.data(), buffer.size());
        }

        // Writing to the region output means our modifications are now represented in the file bytes.
        header.markDirty(false);
        return SUCCESS;
    }

    int ChunkHandle::decodeChunk(lce::CONSOLE c) {
        if (lce::is_console_none(c)) {
            throw std::runtime_error("ChunkHandle::decodeChunk refuses to take lce::CONSOLE::NONE");
        }

        if (m_state == ChunkState::EMPTY) return SUCCESS;
        if (m_state == ChunkState::DECODED) return SUCCESS;

        if (m_state != ChunkState::COMPRESSED) {
            throw std::runtime_error("ChunkHandle::decodeChunk invalid state");
        }

        // Preserve current meaning: header.rle() describes whether we must RLE-decompress after zlib stage.
        const bool hadRleStage = header.rle();

        Buffer decZip;
        decZip.allocate(hadRleStage ? header.getRLESize() : header.getDecSize());
        if (decZip.empty()) return SUCCESS;

        if (lce::is_xbox360_family(c)) {
#ifdef SUPPORT_XBOX360
            int error = xdecompress(decZip.data(), decZip.size_ptr(), buffer.data(), buffer.size());
            if (error) return STATUS::DECOMPRESS;
#else
            throw std::runtime_error("Xbox360 support is off, but decode was attempted");
#endif
        } else if (lce::is_ps3_family(c)) {
            int r = tinf_uncompress(decZip.data(), decZip.size_ptr(), buffer.data(), buffer.size());
            if (r != SUCCESS) return STATUS::DECOMPRESS;
        } else if (lce::is_wiiu_family(c) ||
                   lce::is_psvita_family(c) ||
                   lce::is_ps4_family(c) ||
                   lce::is_switch_family(c) ||
                   lce::is_newgen_family(c) ||
                   lce::is_xbox1_family(c)) {
            int r = tinf_zlib_uncompress(decZip.data(), decZip.size_ptr(), buffer.data(), buffer.size());
            if (r != TINF_OK && r != TINF_ADLER_ERROR) return STATUS::DECOMPRESS;
        } else {
            throw std::runtime_error("Unhandled console case in decode");
        }

        Buffer plain;
        if (hadRleStage) {
            plain.allocate(header.getDecSize());
            if (plain.empty()) return SUCCESS;

            codec::RLE_decompress(plain.data(), plain.size_ptr(), decZip.data(), decZip.size());

            // Match your current behavior: once decoded, treat header.rle as "buffer is plain now"
            header.setRle(false);
        } else {
            plain = std::move(decZip);
        }

        auto decoded = std::make_unique<ChunkData>();

        DataReader reader(plain.span(), Endian::Big);
        if (reader.peek() == 0x0A) {
            decoded->lastVersion = eChunkVersion::V_NBT;
            ChunkFormatNBT::readChunk(decoded.get(), reader);
        } else {
            decoded->lastVersion = reader.read<u16>();
            switch (decoded->lastVersion) {
                case eChunkVersion::V_8:
                case eChunkVersion::V_9:
                case eChunkVersion::V_10:
                case eChunkVersion::V_11:
                    ChunkFormatGridPaletted::readChunk(decoded.get(), reader);
                    break;
                case eChunkVersion::V_12:
                case eChunkVersion::V_13:
                    ChunkFormatGridPalettedSubmerged::readChunk(decoded.get(), reader);
                    break;
                default:
                    break;
            }
        }

        decoded->validChunk = true;

        m_decoded = std::move(decoded);

        // Enforce single representation
        buffer.clear();
        m_state = ChunkState::DECODED;

        return SUCCESS;
    }

    int ChunkHandle::encodeChunk(WriteSettings& settings) {
        if (lce::is_console_none(settings.m_schematic.save_console)) {
            throw std::runtime_error("ChunkHandle::encodeChunk refuses to take lce::CONSOLE::NONE");
        }

        if (m_state == ChunkState::EMPTY) return SUCCESS;
        if (m_state == ChunkState::COMPRESSED) return SUCCESS;

        if (m_state != ChunkState::DECODED || !m_decoded) {
            throw std::runtime_error("ChunkHandle::encodeChunk invalid state");
        }

        DataWriter wtr(256, Endian::Big);

        switch (settings.m_schematic.chunk_lastVersion) {
            case eChunkVersion::V_NBT:
                ChunkFormatNBT::writeChunk(m_decoded.get(), settings, wtr);
                break;
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_10:
            case eChunkVersion::V_11:
                wtr.write<u16>(settings.m_schematic.chunk_lastVersion);
                ChunkFormatGridPaletted::writeChunk(m_decoded.get(), settings, wtr);
                break;
            case eChunkVersion::V_12:
            case eChunkVersion::V_13:
                m_decoded->lastVersion = 12;
                wtr.write<u16>(settings.m_schematic.chunk_lastVersion);
                ChunkFormatGridPalettedSubmerged::writeChunk(m_decoded.get(), settings, wtr);
                break;
            default:
                break;
        }

        Buffer plain = std::move(wtr.take());
        header.setDecSize(plain.size());

        // Optional RLE stage, same logic as your current code:
        // If header.rle() is false (plain), we RLE-compress and then set it true.
        Buffer stage = std::move(plain);

        if (!header.rle()) {
            Buffer rleBuffer;
            const u32 safe = codec::RLE_safe_compress_size(stage.size());
            rleBuffer.allocate(safe);
            codec::RLE_compress(rleBuffer.data(), rleBuffer.size_ptr(), stage.data(), stage.size());
            stage = std::move(rleBuffer);

            header.setRLESize(stage.size());
            header.setRle(true);
        } else {
            header.setRLESize(stage.size());
        }

        const auto c = settings.m_schematic.save_console;

        if (lce::is_xbox360_family(c)) {
            return STATUS::NOT_IMPLEMENTED;
        }

        Buffer compressed((u32)(float(stage.size()) * 1.25F));
        int status = compress(compressed.data(), (uLongf*)compressed.size_ptr(), stage.data(), stage.size());
        if (status) return STATUS::COMPRESS;

        if (lce::is_ps3_family(c)) {
            Buffer stripped(compressed.size() - 2);
            std::memcpy(stripped.data(), compressed.data() + 2, stripped.size());
            compressed = std::move(stripped);
        } else if (!(lce::is_wiiu_family(c) ||
                     lce::is_psvita_family(c) ||
                     lce::is_ps4_family(c) ||
                     lce::is_switch_family(c) ||
                     lce::is_newgen_family(c) ||
                     lce::is_xbox1_family(c))) {
            throw std::runtime_error("Unhandled console case in encode");
        }

        buffer = std::move(compressed);

        // Enforce single representation
        m_decoded.reset();
        m_state = ChunkState::COMPRESSED;

        // Do NOT clear dirty here. Dirty clears on write().
        return SUCCESS;
    }

    ChunkData* ChunkHandle::decodeAndTouch() {
        if (m_state == ChunkState::EMPTY) {
            throw std::runtime_error("Attempted to access ChunkData for an EMPTY chunk slot");
        }

        // Always decode if needed
        if (m_state == ChunkState::COMPRESSED) {
            if (lce::is_console_none(m_srcConsole)) {
                throw std::runtime_error("ChunkHandle has no source console set for decoding");
            }
            int st = decodeChunk(m_srcConsole);
            if (st != SUCCESS) {
                throw std::runtime_error("decodeChunk failed");
            }
        }

        if (m_state != ChunkState::DECODED || !m_decoded) {
            throw std::runtime_error("ChunkHandle::decodeAndTouch failed to produce decoded data");
        }

        // Per your requirement: any access marks dirty (even reads)
        header.markDirty(true);
        return m_decoded.get();
    }

} // namespace editor
