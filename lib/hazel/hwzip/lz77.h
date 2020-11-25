/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef LZ77_H
#define LZ77_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LZ_WND_SIZE 32768
#define LZ_MAX_LEN  258

/* Perform LZ77 compression on the len bytes in src. Returns false as soon as
   either of the callback functions returns false, otherwise returns true when
   all bytes have been processed. */
bool lz77_compress(const uint8_t *src, size_t len,
                   bool (*lit_callback)(uint8_t lit, void *aux),
                   bool (*backref_callback)(size_t dist, size_t len, void *aux),
                   void *aux);

/* Output lit at dst_pos in dst. */
static inline void lz77_output_lit(uint8_t *dst, size_t dst_pos, uint8_t lit)
{
        dst[dst_pos] = lit;
}

/* Output the (dist,len) backref at dst_pos in dst. */
static inline void lz77_output_backref(uint8_t *dst, size_t dst_pos,
                                       size_t dist, size_t len)
{
        size_t i;

        assert(dist <= dst_pos && "cannot reference before beginning of dst");

        for (i = 0; i < len; i++) {
                dst[dst_pos] = dst[dst_pos - dist];
                dst_pos++;
        }
}

#endif
