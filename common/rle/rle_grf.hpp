#pragma once

#include "lce/processor.hpp"


namespace codec {

    inline u32 RLE_GRF_Decompress(
            const u8* dataIn, u32 sizeIn,
            u8*       dataOut, u32 sizeOut) {
        u32 indexIn = 0;
        u32 indexOut = 0;

        while (indexIn < sizeIn) {
            c_u8 first = dataIn[indexIn++];
            if (first != 255) {
                if (indexOut >= sizeOut) { goto EARLY_EXIT; }
                dataOut[indexOut++] = first;

            } else {
                if (indexIn >= sizeIn) { goto EARLY_EXIT; }
                c_u32 length = dataIn[indexIn++];

                if (length >= 3 && length <= 254) {

                    if (indexIn >= sizeIn) { goto EARLY_EXIT; }
                    u8 value = dataIn[indexIn++];

                    u32 runCount = static_cast<u32>(length) + 1;
                    if (indexOut + runCount > sizeOut) { goto EARLY_EXIT; }
                    for (u32 k = 0; k < length + 1; ++k) {
                        dataOut[indexOut++] = value;
                    }
                } else {
                    if (indexOut >= sizeOut) { goto EARLY_EXIT; }
                    dataOut[indexOut++] = length;
                }
            }
        }

        return indexOut;
    EARLY_EXIT:
        return 0;
    }













    inline bool shouldCompressRun(u8 value, u32 runLen) {
        if (value == 255) {
            // for 0xFF, compress whenever runLen ≥ 2
            return (runLen >= 2);
        }
        // for any other byte, compress only if runLen ≥ 4
        return (runLen > 3);
    }


    /**
     * Compresses “dataIn[0..sizeIn)” into “dataOut[0..]” under the RLE scheme above.
     *
     * @param dataIn   pointer to input bytes
     * @param sizeIn   number of input bytes
     * @param dataOut  pointer to output buffer (must be large enough)
     * @param outCap   capacity of dataOut in bytes
     * @return         number of bytes written into dataOut
     * @throws         std::runtime_error if outCap is insufficient or input is invalid
     */
    inline u32 RLE_GRF_Compress(
            const u8* dataIn, u32 sizeIn,
            u8*       dataOut, u32 outCap) {
        u32 inPos  = 0;
        u32 outPos = 0;

        while (inPos < sizeIn) {
            // 1) Determine the run value and run length
            u8  value  = dataIn[inPos];
            u32   runLen = 1;
            // Count how many times `value` repeats, up to potentially huge runs
            while ((inPos + runLen) < sizeIn && dataIn[inPos + runLen] == value) {
                ++runLen;
                // If runLen exceeds 255, we’ll break it into sub‐runs of at most 255 each
                if (runLen >= (u32)255 + 1) {
                    break;
                }
            }

            // 2) Possibly break the run into sub‐runs of at most (maxLength + 1) = 255
            u32 remaining = runLen;
            while (remaining > 0) {
                // Sub‐run length cannot exceed 255
                u32 chunkLen = (remaining > (u32)255 ? (u32)255 : remaining);

                // Decide whether THIS sub‐run should be compressed or emitted literally:
                if (shouldCompressRun(value, chunkLen)) {
                    // we need 3 bytes: [marker, (length−1), value]
                    if (outPos + 3 > outCap) {
                        return 0;
                    }
                    dataOut[outPos++] = 255;
                    dataOut[outPos++] = static_cast<u8>(chunkLen - 1);
                    dataOut[outPos++] = value;
                }
                else {
                    // emit chunkLen copies of 'value' literally
                    if (outPos + chunkLen > outCap) {
                        return 0;
                    }
                    for (u32 k = 0; k < chunkLen; ++k) {
                        dataOut[outPos++] = value;
                    }
                }

                remaining -= chunkLen;
            }

            inPos += runLen;
        }

        return outPos;
    }







}
