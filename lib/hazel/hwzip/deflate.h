/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef DEFLATE_H
#define DEFLATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
        HWINF_OK,   /* Inflation was successful. */
        HWINF_FULL, /* Not enough room in the output buffer. */
        HWINF_ERR   /* Error in the input data. */
} inf_stat_t;

/* Decompress (inflate) the Deflate stream in src. The number of input bytes
   used, at most src_len, is stored in *src_used on success. Output is written
   to dst. The number of bytes written, at most dst_cap, is stored in *dst_used
   on success. src[0..src_len-1] and dst[0..dst_cap-1] must not overlap.
   Returns a status value as defined above. */
inf_stat_t hwinflate(const uint8_t *src, size_t src_len, size_t *src_used,
                     uint8_t *dst, size_t dst_cap, size_t *dst_used);


/* Compress (deflate) the data in src into dst. The number of bytes output, at
   most dst_cap, is stored in *dst_used. Returns false if there is not enough
   room in dst. src and dst must not overlap. */
bool hwdeflate(const uint8_t *src, size_t src_len, uint8_t *dst,
               size_t dst_cap, size_t *dst_used);

#endif
