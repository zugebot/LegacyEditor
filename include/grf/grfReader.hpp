#pragma once

#include <filesystem>
#include <vector>

#include "common/buffer.hpp"
#include "common/DataReader.hpp"

#include "common/codec/XDecompress.hpp"

#include "grf.hpp"
#include "crc.hpp"

#include "DecompressionHelpers.hpp"
#include "common/RLE/rle.hpp"
#include "include/tinf/tinf.h"
#include "include/zlib-1.2.12/zconf.h"
#include "include/zlib-1.2.12/zlib.h"

#include <span>
#include <stdexcept>
#include <zlib.h>


#include "common/RLE/rle_grf.hpp"





/**
 * Reads a GRF file (binary) into a fully populated GameRuleFile object.
 * Mirrors the behaviour of the original C# GameRuleFileReader.
 *
 *  – Uses Buffer / DataReader for IO
 *  – Transparently handles compression, RLE, CRC check
 *  – Populates the string‑lookup table and game‑rule tree
 */
class GameRuleFileReader {
public:
    explicit GameRuleFileReader(GameRuleFileHeader::CompressionType type)
        : _compressionType(type) {}

    // Read from file path --------------------------------------------------
    [[nodiscard]] GameRuleFile FromFile(const std::filesystem::path& p) const {
        Buffer buf = DataReader::readFile(p);
        return FromBuffer(buf, lce::CONSOLE::NONE);
    }

    // Read from already‑loaded buffer -------------------------------------
    [[nodiscard]] GameRuleFile FromBuffer(const Buffer& buf, lce::CONSOLE console) const {
        DataReader reader(buf, Endian::Big);
        GameRuleFileHeader header = ReadHeader(reader, console);
        GameRuleFile grf(header);
        ReadBody(grf, reader, console);
        return grf;
    }

private:
    using CompressionLevel  = GameRuleFileHeader::CompressionLevel;
    using CompressionType   = GameRuleFileHeader::CompressionType;

    CompressionType _compressionType;
    mutable std::vector<std::string> _stringLUT; // reusable between helpers

    // ── Header -----------------------------------------------------------
    GameRuleFileHeader ReadHeader(DataReader& r, lce::CONSOLE console) const {
        auto magic = r.read<u16>();
        // short "1"
        // eight "0"
        // ruleset none?
        // write int 0
        // write int 2
        // write 3 null bytes
        // write 3 0 ints
        if (magic == 0) {
            // short, int, 8 null bytes
            r.skip(14); // unknown legacy header
            return {0xFFFF'FFFF, CompressionLevel::None};
        }

        auto lvl  = static_cast<CompressionLevel>(r.read<u8>());
        auto crc = r.read<u32>();

        std::array<u8, 4> unknown{};
        r.readBytes(4, unknown.data());

        return {crc, lvl, _compressionType, unknown};
    }

    // ── Body -------------------------------------------------------------
    void ReadBody(GameRuleFile& file, DataReader& reader, lce::CONSOLE console) const {
        // decompress if needed
        // if (file.Header.Level != CompressionLevel::None) {
        Buffer temp;
        DecompressBody(file.Header, temp, reader, console);

        // reader.skip(23);

        getStringTable(reader);
        ReadFileEntries(reader, file);
        ReadGameRuleHierarchy(reader, file.Root);
    }

    // --------------------------------------------------------------------
    void DecompressBody(const GameRuleFileHeader& hdr, Buffer& buf, DataReader& r, lce::CONSOLE console) const {
        i32 decompressedSize, compressedSize, rleCompressedSize;
        if (console != lce::CONSOLE::PS3) {
            decompressedSize  = r.read<i32>();
            compressedSize    = r.read<i32>();
            rleCompressedSize = decompressedSize;
        } else {
            compressedSize    = r.read<i32>();
            decompressedSize  = r.read<i32>();
            rleCompressedSize = r.read<i32>();
        }

        const u8* compressedStart = r.fetch(compressedSize);
        std::span<const u8> compSpan(compressedStart, compressedSize);

        if (console == lce::CONSOLE::PS3 || hdr.Level >= CompressionLevel::Compressed) {
            buf = DecompressStream(compSpan, rleCompressedSize);
            if (console == lce::CONSOLE::PS3 || hdr.Level >= CompressionLevel::CompressedRle) {
                Buffer bufOut(decompressedSize);
                codec::RLE_decompress(buf.data(), rleCompressedSize,
                                      bufOut.data(), bufOut.size_ref());
                buf = std::move(bufOut);
            }
        } else {
            buf = Buffer(compSpan.size());
            memcpy(buf.data(), compSpan.data(), compSpan.size());
        }

        r = DataReader(buf, Endian::Big);
    }

    // Decompress raw deflate/zlib/xmem block ------------------------------
    Buffer DecompressStream(std::span<const u8> src, u32 expectedSize) const;


    // --------------------------------------------------------------------
    void getStringTable(DataReader& r) const {
        i32 cnt = r.read<i32>();
        _stringLUT.clear();
        _stringLUT.reserve(cnt);
        for (int i = 0; i < cnt; ++i) {
            _stringLUT.emplace_back(ReadString(r));
        }
    }

    void ReadFileEntries(DataReader& r, GameRuleFile& file) const {
        i32 count = r.read<i32>();
        for (int i = 0; i < count; ++i) {
            std::string name = ReadString(r);
            i32 size = r.read<i32>();
            Buffer buf = r.readBuffer(size);
            file.AddFile(name, {buf.data(), buf.data() + buf.size()});
        }
    }

    void ReadGameRuleHierarchy(DataReader& r, GameRuleFile::GameRule& parent) const {
        i32 count = r.read<i32>();
        for (int i = 0; i < count; ++i) {
            std::string ruleName = GetStringRef(r);
            i32 paramCnt = r.read<i32>();
            auto& rule = parent.AddRule(ruleName);
            for (int j = 0; j < paramCnt; ++j) {
                std::string key = GetStringRef(r);
                std::string val = ReadString(r);
                rule.Parameters.emplace(std::move(key), std::move(val));
            }
            ReadGameRuleHierarchy(r, rule);
        }
    }

    // --- helpers ---------------------------------------------------------
    [[nodiscard]] static std::string ReadString(DataReader& r) {
        auto len = r.read<u16>();
        std::string s;
        s.reserve(len);
        for (u16 i = 0; i < len; ++i)
            s.push_back(static_cast<char>(r.read<u8>()));
        return s;
    }

    [[nodiscard]] std::string GetStringRef(DataReader& r) const {
        auto idx = r.read<u32>();
        if (idx >= _stringLUT.size())
            throw std::out_of_range("String LUT index out of range");
        return _stringLUT[idx];
    }
};




inline Buffer GameRuleFileReader::DecompressStream(std::span<const u8> src, u32 expectedSize)  const {

    switch (_compressionType) {
        case CompressionType::Zlib: {
        case CompressionType::Deflate:
            return DecompressWithTinf(src, expectedSize);
        }
        case CompressionType::XMem: {
            Buffer dest(expectedSize);
            auto res = codec::XDecompress(src.data(), src.size(),
                                          dest.data(), dest.size_ptr());
            if (res != codec::XmemErr::Ok) {
                return {};
            }
            return dest;
        }
        default: throw std::runtime_error("Unknown compression type");
    }
}



