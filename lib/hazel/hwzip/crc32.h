/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef CRC32_H
#define CRC32_H

#include <stddef.h>
#include <stdint.h>

/* Compute the CRC-32 of src[0..n-1]. */
uint32_t crc32(const uint8_t *src, size_t n);

#endif
