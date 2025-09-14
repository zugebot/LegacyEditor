#ifndef XCOMPRESS_H
#define XCOMPRESS_H
#include "lzx_conf.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif
    LZX_API int xcompress(uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t *dst_len);
#ifdef __cplusplus
}
#endif

#endif // XCOMPRESS_H