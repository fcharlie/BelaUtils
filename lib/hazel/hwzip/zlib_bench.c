/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "hamlet.h"
#include "zlib_utils.h"

#define DST_SZ (1024 * 1024)

void *bench_zlib_deflate_hamlet_setup(void)
{
        return malloc(DST_SZ);
}

void bench_zlib_deflate_hamlet(void *aux)
{
        uint8_t *dst = aux;

        zlib_deflate(hamlet, sizeof(hamlet), 9, dst, DST_SZ);
}

void bench_zlib_deflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_deflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


void *bench_zlib_inflate_hamlet_setup(void)
{
        uint8_t *dst = malloc(DST_SZ * 2);

        zlib_deflate(hamlet, sizeof(hamlet), 9, dst, DST_SZ);

        return dst;
}

void bench_zlib_inflate_hamlet(void *aux)
{
        uint8_t *comp = aux;
        uint8_t *decomp = &comp[DST_SZ];
        size_t decomp_sz;

        decomp_sz = zlib_inflate(comp, DST_SZ, decomp, DST_SZ);

        assert(decomp_sz == sizeof(hamlet));
        assert(memcmp(decomp, hamlet, sizeof(hamlet)) == 0);
}

void bench_zlib_inflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_inflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
