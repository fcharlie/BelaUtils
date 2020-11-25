/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "deflate.h"
#include "hamlet.h"
#include "zlib_utils.h"

void *bench_deflate_hamlet_setup(void)
{
        return malloc(sizeof(hamlet));
}

void bench_deflate_hamlet(void *aux)
{
        uint8_t *dst = aux;
        size_t dst_used;

        hwdeflate(hamlet, sizeof(hamlet),
                  dst, sizeof(hamlet), &dst_used);
}

void bench_deflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_deflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
