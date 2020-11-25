/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "crc32.h"

#include "tables.h"

/* See Figure 14-7 in Hacker's Delight (2nd ed.), or
   crc_reflected() in https://www.zlib.net/crc_v3.txt
   or Garry S. Brown's implementation e.g. in
   https://opensource.apple.com/source/xnu/xnu-792.13.8/bsd/libkern/crc32.c */

uint32_t crc32(const uint8_t *src, size_t n)
{
        size_t i;
        uint32_t crc = 0xffffffff;

        for (i = 0; i < n; i++) {
                crc = (crc >> 8) ^ crc32_tbl[(crc ^ src[i]) & 0xff];
        }

        return crc ^ 0xffffffff;
}
