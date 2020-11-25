/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

/* Build and run like this:

   $ clang -g -fsanitize=address,fuzzer zip_fuzzer.c zip.c deflate.c \
         lz77.c huffman.c tables.c crc32.c -o zip_fuzzer &&
         ./zip_fuzzer -max_len=100000 -rss_limit_mb=4096 zip_corpus/
 */

#include "zip.h"

#include "crc32.h"
#include "deflate.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

static uint8_t *inflate_member(const zipmemb_t *m)
{
        uint8_t *p;
        size_t src_used, dst_used;

        assert(m->method == ZIP_DEFLATED);

        p = malloc(m->uncomp_size);
        if (p == NULL) {
                return NULL;
        }

        if (hwinflate(m->comp_data, m->comp_size, &src_used, p, m->uncomp_size,
                      &dst_used) != HWINF_OK) {
                free(p);
                return NULL;
        }

        if (src_used != m->comp_size || dst_used != m->uncomp_size) {
                free(p);
                return NULL;
        }

        return p;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        zip_t z;
        size_t n;
        zipiter_t it;
        zipmemb_t m;
        uint8_t *inflated;
        const uint8_t *uncomp_data;

        if (!zip_read(&z, data, size)) {
                return 0;
        }

        n = 0;
        for (it = z.members_begin; it != z.members_end; it = m.next) {
                n++;
                m = zip_member(&z, it);

                assert(m.name + m.name_len <= data + size);
                assert(m.comment + m.comment_len <= data + size);
                assert(m.comp_data + m.comp_size <= data + size);

                if (m.method == ZIP_STORED) {
                        inflated = NULL;
                        uncomp_data = m.comp_data;
                } else {
                        assert(m.method == ZIP_DEFLATED);
                        inflated = inflate_member(&m);
                        if (inflated == NULL) {
                                continue;
                        }
                        uncomp_data = inflated;
                }

                crc32(uncomp_data, m.uncomp_size);
                free(inflated);
        }
        assert(n == z.num_members);

        return 0;
}
