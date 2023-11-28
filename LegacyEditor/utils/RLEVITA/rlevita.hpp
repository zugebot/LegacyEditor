#pragma once

#include "LegacyEditor/utils/processor.hpp"


static void RLEVITA_DECOMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut);
static u32 RLEVITA_COMPRESS(u8* dataIn, u32 sizeIn, u8* dataOut, u32 sizeOut);

