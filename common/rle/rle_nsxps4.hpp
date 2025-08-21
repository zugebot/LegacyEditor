#pragma once

#include "include/lce/processor.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace codec {

    /**
     * A form of RLE decompression.
     *
     * @param dataIn buffer_in to parseLayer from
     * @param sizeIn buffer_in size
     * @param dataOut a pointer to allocated buffer_out
     * @param sizeOut the size of the allocated buffer_out
     */
    static Buffer RLE_NSXPS4_DECOMPRESS(Buffer& buffer) {
        DataReader reader(buffer.span(), Endian::Little);

        c_u32 sizeOut = reader.read<u32>();

        reader.setEndian(Endian::Big);

        DataWriter writer(sizeOut, Endian::Big);

        while (reader.tell() < buffer.size()) {

            if (c_u8 value = reader.read<u8>(); value != 0x00) {
                writer.write<u8>(value);

            } else {
                u32 numZeros = reader.read<u8>();

                if (numZeros == 0) {
                    numZeros = reader.read<u16>();
                    numZeros += 256;
                }

                writer.writePad(numZeros, 0);
            }
        }

        return writer.take();
    }


    /**
     * A form of RLE compression.
     *
     * @param dataIn buffer_in to parseLayer from
     * @param sizeIn buffer_in size
     * @param dataOut a pointer to allocated buffer_out
     * @param sizeOut the size of the allocated buffer_out
     */
    MU static Buffer RLE_NSXPS4_COMPRESS(Buffer& buffer, u32 sizeOut = 0) {
        DataReader reader(buffer.span());
        DataWriter writer(sizeOut, Endian::Little);
        writer.write<u32>(buffer.size());
        writer.setEndian(Endian::Big);

        while (reader.tell() < buffer.size()) {
            u8 value = reader.read<u8>();

            if (value != 0) {
                writer.write<u8>(value);
                continue;
            }

            u32 runCount = 1;

            // count **and consume** additional zeros
            while (!reader.eof() && reader.peek() == 0) {
                reader.read<u8>();
                ++runCount;
            }

            if (runCount < 256) {
                writer.write<u8>(0);
                writer.write<u8>(static_cast<u8>(runCount));
            } else {
                writer.write<u8>(0);
                writer.write<u8>(0);
                writer.write<u16>(static_cast<u16>(runCount - 256));
            }
        }

        return writer.take();
    }
}