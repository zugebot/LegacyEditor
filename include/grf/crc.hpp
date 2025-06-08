#pragma once

#include <array>
#include <vector>
#include <cstdint>

class GRF_CRC32 {
    static constexpr std::array<uint8_t, 60> offsets{
            0x05, 0x06, 0x07, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x01, 0x02, 0x06, 0x01,
            0x01, 0x02, 0x03, 0x04,
            0x05, 0x05, 0x06, 0x07,
            0x03, 0x04, 0x05, 0x06,
            0x07, 0x01, 0x02, 0x06,
            0x01, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x05, 0x06,
            0x07, 0x03, 0x04, 0x05,
            0x06, 0x07, 0x07, 0x05,
            0x04, 0x07, 0x05, 0x04,
            0x07, 0x05, 0x04, 0x07,
            0x06, 0x05, 0x04, 0x03,
    };

    static inline std::array<uint32_t, 256> CRCTable{};
    static inline bool hasCRCTable = false;

    static void MakeCRCTable() {
        const uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t temp = i;
            for (int j = 0; j < 8; ++j) {
                if (temp & 1)
                    temp = (temp >> 1) ^ polynomial;
                else
                    temp >>= 1;
            }
            CRCTable[i] = temp;
        }
        hasCRCTable = true;
    }

public:
    static uint32_t UpdateCRC(uint32_t _crc, const std::vector<uint8_t>& data) {
        if (!hasCRCTable)
            MakeCRCTable();

        uint32_t crc = _crc;
        size_t pos = 0;
        size_t offset = 0;

        while (pos < data.size()) {
            uint8_t value = data[pos];
            pos += offsets[offset];

            crc = CRCTable[(crc ^ value) & 0xFF] ^ (crc >> 8);

            offset = offset + 1U + ((offset + 1U >> 3) / 7) * static_cast<uint32_t>(-0x38);
            offset %= offsets.size(); // wrap to avoid out-of-bounds
        }

        return crc;
    }

    static uint32_t CRC(const std::vector<uint8_t>& data) {
        return ~UpdateCRC(0xFFFFFFFF, data);
    }
};
