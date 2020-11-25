/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

/* Build and run like this:

   $ clang -g -fsanitize=address,fuzzer deflate_fuzzer.c deflate.c lz77.c \
         huffman.c tables.c zlib_utils.c -lz -o deflate_fuzzer &&
         ./deflate_fuzzer -max_len=1000000 -rss_limit_mb=4096 deflate_corpus/

 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "deflate.h"
#include "zlib_utils.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t compressed[1024 * 1024 + 1000];
        uint8_t decompressed[1024 * 1024];
        size_t compressed_sz, compressed_used, decompressed_sz;

        /* Discard overly large inputs. */
        if (size > 1024 * 1024) {
                return 0;
        }

        /* Check that we can compress it. */
        assert(hwdeflate(data, size, compressed, sizeof(compressed),
                         &compressed_sz));

        /* Check that we can decompress it. */
        assert(hwinflate(compressed, compressed_sz, &compressed_used,
                         decompressed, sizeof(decompressed),
                         &decompressed_sz) == HWINF_OK);
        assert(decompressed_sz == size);
        assert(compressed_used == compressed_sz);
        assert(memcmp(decompressed, data, size) == 0);

        /* Check that zlib can also decompress it. */
        decompressed_sz = zlib_inflate(compressed, sizeof(compressed),
                                       decompressed, sizeof(decompressed));
        assert(decompressed_sz == size);
        assert(memcmp(decompressed, data, size) == 0);

        return 0;
}
