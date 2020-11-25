/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "deflate.h"
#include "hamlet.h"
#include "zlib_utils.h"

static size_t hamlet_deflated_size;

void *bench_inflate_hamlet_setup(void)
{
        uint8_t *compressed = malloc(sizeof(hamlet));

        hamlet_deflated_size = zlib_deflate(hamlet, sizeof(hamlet), 9,
                                            compressed, sizeof(hamlet));

        return compressed;
}

void bench_inflate_hamlet(void *aux)
{
        static uint8_t out[sizeof(hamlet)];
        uint8_t *compressed = aux;
        inf_stat_t s;
        size_t src_used, dst_len;

        s = hwinflate(compressed, hamlet_deflated_size, &src_used,
                      out, sizeof(out), &dst_len);

        assert(s == HWINF_OK);
        assert(dst_len == sizeof(hamlet));
        assert(memcmp(out, hamlet, sizeof(hamlet)) == 0);
}

void bench_inflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_inflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
