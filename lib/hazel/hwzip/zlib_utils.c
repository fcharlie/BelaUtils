/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "zlib_utils.h"

#include <assert.h>

size_t zlib_deflate(const uint8_t *src, size_t src_len, int level,
                    uint8_t *dst, size_t dst_cap)
{
        z_stream strm;
        int ret;
        size_t dst_size;

        assert(level >= 0 && level <= 9);

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        ret = deflateInit2(&strm, level, Z_DEFLATED, -15, 9,
                           level == 1 ? Z_FIXED : Z_DEFAULT_STRATEGY);
        assert(ret == Z_OK);

        strm.avail_in = (unsigned)src_len;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
        strm.next_in = (uint8_t*)src;
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        strm.avail_out = (unsigned)dst_cap;
        strm.next_out = dst;

        ret = deflate(&strm, Z_FINISH);
        assert(ret != Z_STREAM_ERROR);
        assert(strm.avail_in == 0);

        dst_size = dst_cap - strm.avail_out;

        deflateEnd(&strm);

        return dst_size;
}

size_t zlib_inflate(const uint8_t *src, size_t src_len,
                    uint8_t *dst, size_t dst_cap)
{
        z_stream strm;
        int ret;
        size_t dst_size;

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        ret = inflateInit2(&strm, -15);
        assert(ret == Z_OK);

        strm.avail_in = (unsigned)src_len;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
        strm.next_in = (uint8_t*)src;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        strm.avail_out = (unsigned)dst_cap;
        strm.next_out = dst;

        ret = inflate(&strm, Z_FINISH);

        assert(ret == Z_STREAM_END);

        dst_size = dst_cap - strm.avail_out;

        inflateEnd(&strm);

        return dst_size;
}
