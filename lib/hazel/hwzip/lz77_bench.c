/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <stdlib.h>
#include "lz77.h"
#include "hamlet.h"

void *bench_lz77_compress_setup(void)
{
        return NULL;
}

static bool output_backref_nop(size_t dist, size_t len, void *aux)
{
        (void)dist;
        (void)len;
        (void)aux;
        return true;
}

static bool output_lit_nop(uint8_t lit, void *aux)
{
        (void)lit;
        (void)aux;
        return true;
}

void bench_lz77_compress(void *aux)
{
        (void)aux;
        lz77_compress(hamlet, sizeof(hamlet), &output_lit_nop,
                      &output_backref_nop, NULL);
}

void bench_lz77_compress_teardown(void *aux)
{
        (void)aux;
}

size_t bench_lz77_compress_bytes(void)
{
        return sizeof(hamlet);
}
