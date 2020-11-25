/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

/* Build and run like this:

   $ clang -g -fsanitize=address,fuzzer inflate_fuzzer.c deflate.c lz77.c \
         huffman.c tables.c -o inflate_fuzzer &&
         ./inflate_fuzzer -max_len=1000000 -rss_limit_mb=4096 inflate_corpus/
 */

#include "deflate.h"

#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t uncomp[1024 * 1024];
        size_t src_used, uncomp_sz;

        /* Discard overly large inputs. */
        if (size > 100 * 1024) {
                return 0;
        }

        /* Check that we don't hit any asserts, ASan errors, etc. */
        hwinflate(data, size, &src_used, uncomp, sizeof(uncomp), &uncomp_sz);

        return 0;
}
