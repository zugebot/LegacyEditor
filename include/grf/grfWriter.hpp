#pragma once

#include "buffer.hpp"
#include "DataWriter.hpp"
#include "GameRuleFile.hpp"  // your own ported GameRuleFile structures
#include "CRC32.hpp"         // your CRC implementation
#include <filesystem>

class GameRuleFileWriter {
public:
    static void WriteToFile(const GameRuleFile& ruleFile, const std::filesystem::path& outPath) {
        DataWriter writer(1024, Endian::Little);

        // Write the file header
        writer.write<u32>(ruleFile.Header.magic);
        writer.write<u32>(ruleFile.Files.size());
        writer.write<u8>(static_cast<u8>(ruleFile.Header.compression));

        // Write file entries
        std::vector<Buffer> compressedFiles;
        for (const auto& file : ruleFile.Files) {
            writer.write<u16>(file.Name.size());
            for (char c : file.Name) writer.write<u8>(c);

            Buffer compressedData = compressFile(file.Data, ruleFile.Header.compression);
            writer.write<u32>(compressedData.size());

            if (ruleFile.Header.compression == GameRuleFile::CompressionLevel::CompressedRleCrc) {
                u32 crc = CRC32::CRC(std::span<const u8>(compressedData.data(), compressedData.size()));
                writer.write<u32>(crc);
            }

            compressedFiles.push_back(std::move(compressedData));
        }

        // Write the compressed payloads
        for (const auto& comp : compressedFiles)
            writer.writeBytes(comp.data(), comp.size());

        writer.save(outPath);
    }

private:
    static Buffer compressFile(const std::vector<u8>& input, GameRuleFile::CompressionLevel level) {
        switch (level) {
            case GameRuleFile::CompressionLevel::None:
                return Buffer(input.data(), input.size());
            case GameRuleFile::CompressionLevel::Compressed:
                return Compress_Zlib(input);
            case GameRuleFile::CompressionLevel::CompressedRle:
                return Compress_RLE(input);
            case GameRuleFile::CompressionLevel::CompressedRleCrc:
                return Compress_RLE(input);  // CRC handled externally
            default:
                throw std::invalid_argument("Unsupported compression level");
        }
    }
};
