#ifndef XDECOMPRESS_H
#define XDECOMPRESS_H
#include "lzx_conf.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif
    LZX_API int xdecompress(uint8_t *dst, uint32_t *dst_len, uint8_t *src, uint32_t src_len);
#ifdef __cplusplus
}
#endif

#endif // XDECOMPRESS_H