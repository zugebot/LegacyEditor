#pragma once

#include <array>
#include <cstdint>

/**
 * Hades‑style Game Rule File header (GRF).
 * Mirrors the original C# structure exactly, including the four
 *   unknown bytes used by “world GRF” variants.
 */
class GameRuleFileHeader {
public:
    /// Compression level flags stored in the GRF header (byte 2).
    enum class CompressionLevel : uint8_t {
        None             = 0,
        Compressed       = 1,
        CompressedRle    = 2,
        CompressedRleCrc = 3,
    };

    /// Physical compression algorithm (platform‑specific).
    enum class CompressionType : int8_t {
        Unknown = -1,
        Zlib,    // Vita / WiiU / Switch
        Deflate, // PlayStation‑3
        XMem     // Xbox‑360
    };

    uint32_t         Crc            = 0xFFFF'FFFF;   ///< Stored CRC32 (only for level >= CompressedRleCrc)
    CompressionLevel Level          = CompressionLevel::None;
    CompressionType  Type           = CompressionType::Unknown;
    std::array<uint8_t,4> Unknown{};                 ///< Reserved; non‑zero in “world GRF” files

    // ---------------------------------------------------------------------
    GameRuleFileHeader() = default;
    GameRuleFileHeader(uint32_t crc, CompressionLevel lvl)                     : Crc(crc), Level(lvl) {}
    GameRuleFileHeader(uint32_t crc, CompressionLevel lvl, CompressionType t)  : Crc(crc), Level(lvl), Type(t) {}
    GameRuleFileHeader(uint32_t crc, CompressionLevel lvl, const std::array<uint8_t,4>& bytes)
        : Crc(crc), Level(lvl), Unknown(bytes) {}
    GameRuleFileHeader(uint32_t crc, CompressionLevel lvl, CompressionType t, const std::array<uint8_t,4>& bytes)
        : Crc(crc), Level(lvl), Type(t), Unknown(bytes) {}
};
