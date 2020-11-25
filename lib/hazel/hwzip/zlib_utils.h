/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef ZLIB_UTILS_H
#define ZLIB_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <zlib.h>

/* Utility functions for using zlib. */


/* Deflate the data in src[0..src_len-1] into dst[0..dst_cap-1]. Level must be
   between 0 and 9. At level 1, fixed Huffman blocks are used. Assumes that
   there is enough room in the output buffer. Returns the number of output
   bytes. */
size_t zlib_deflate(const uint8_t *src, size_t src_len, int level,
                    uint8_t *dst, size_t dst_cap);

/* Inflate the data in src[0..src_len-1] into dst[0..dst_cap-1]. Assumes that
   there is enough room in dst, and that the input is valid. Returns the number
   of output bytes. */
size_t zlib_inflate(const uint8_t *src, size_t src_len,
                    uint8_t *dst, size_t dst_cap);

#endif
