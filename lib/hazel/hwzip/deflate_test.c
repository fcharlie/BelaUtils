/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "deflate.h"

#include <stdlib.h>
#include "bitstream.h"
#include "hamlet.h"
#include "huffman.h"
#include "test_utils.h"
#include "zlib_utils.h"

/* Compress src, then decompress it and check that it matches.
   Returns the size of the compressed data. */
static size_t deflate_roundtrip(const uint8_t *src, size_t len)
{
        uint8_t *compressed, *decompressed;
        size_t compressed_sz, decompressed_sz, compressed_used;
        size_t i, tmp;

        compressed = malloc(len * 2 + 100);
        CHECK(hwdeflate(src, len, compressed, 2 * len + 100, &compressed_sz));

        decompressed = malloc(len);
        CHECK(hwinflate(compressed, compressed_sz, &compressed_used,
                        decompressed, len, &decompressed_sz) == HWINF_OK);
        CHECK(compressed_used == compressed_sz);
        CHECK(decompressed_sz == len);
        CHECK(src == NULL || memcmp(src, decompressed, len) == 0);

        if (len < 1000) {
                /* For small inputs, check that any too small buffer fails. */
                for (i = 0; i < compressed_used; i++) {
                        CHECK(!hwdeflate(src, len, compressed, i, &tmp));
                }
        } else if (compressed_sz > 500) {
                /* For larger inputs, try cutting off the first block. */
                CHECK(!hwdeflate(src, len, compressed, 500, &tmp));
        }

        free(compressed);
        free(decompressed);

        return compressed_sz;
}

typedef enum {
        UNCOMP = 0x0,
        STATIC = 0x1,
        DYNAMIC = 0x2
} block_t;

static void check_deflate_string(const char *str, block_t expected_type)
{
        uint8_t comp[1000];
        size_t comp_sz;

        CHECK(hwdeflate((const uint8_t*)str, strlen(str), comp,
                        sizeof(comp), &comp_sz));
        CHECK(((comp[0] & 7) >> 1) == expected_type);

        deflate_roundtrip((const uint8_t*)str, strlen(str));
}

void test_deflate_basic(void)
{
        char buf[256];
        size_t i;

        /* Empty input; a static block is shortest. */
        deflate_roundtrip((const uint8_t*)"", 0);
        check_deflate_string("", STATIC);

        /* One byte; a static block is shortest. */
        check_deflate_string("a", STATIC);

        /* Repeated substring. */
        check_deflate_string("hellohello", STATIC);

        /* Non-repeated long string with small alphabet. Dynamic. */
        check_deflate_string("abcdefghijklmnopqrstuvwxyz"
                             "zyxwvutsrqponmlkjihgfedcba", DYNAMIC);

        /* No repetition, uniform distribution. Uncompressed. */
        for (i = 0; i < 255; i++) {
                buf[i] = (char)(i + 1);
        }
        buf[255] = 0;
        check_deflate_string(buf, UNCOMP);
}

void test_deflate_hamlet(void)
{
        size_t len;

        len = deflate_roundtrip(hamlet, sizeof(hamlet));

        /* Update if we make compression better. */
        CHECK(len == 79708);
}

void test_deflate_mixed_blocks(void)
{
        uint8_t *src, *p;
        uint32_t r;
        size_t i, j;
        const size_t src_size = 2 * 1024 * 1024;

        src = malloc(src_size);

        memset(src, 0, src_size);

        p = src;
        r = 0;
        for (i = 0; i < 5; i++) {
                /* Data suitable for compressed blocks. */
                memcpy(src, hamlet, sizeof(hamlet));
                p += sizeof(hamlet);

                /* Random data, likely to go in an uncompressed block. */
                for (j = 0; j < 128000; j++) {
                        r = next_test_rand(r);
                        *p++ = (uint8_t)(r >> 24);
                }
        }

        deflate_roundtrip(src, src_size);

        free(src);
}

void test_deflate_random(void)
{
        uint8_t *src;
        const size_t src_size = 3 * 1024 * 1024;
        uint32_t r;
        size_t i;

        src = malloc(src_size);

        r = 0;
        for (i = 0; i < src_size; i++) {
                r = next_test_rand(r);
                src[i] = (uint8_t)(r >> 24);
        }

        deflate_roundtrip(src, src_size);

        free(src);
}


#define MIN_LITLEN_LENS 257
#define MAX_LITLEN_LENS 288
#define MIN_DIST_LENS 1
#define MAX_DIST_LENS 32
#define MIN_CODELEN_LENS 4
#define MAX_CODELEN_LENS 19
#define CODELEN_MAX_LIT 15
#define CODELEN_COPY 16
#define CODELEN_COPY_MIN 3
#define CODELEN_COPY_MAX 6
#define CODELEN_ZEROS 17
#define CODELEN_ZEROS_MIN 3
#define CODELEN_ZEROS_MAX 10
#define CODELEN_ZEROS2 18
#define CODELEN_ZEROS2_MIN 11
#define CODELEN_ZEROS2_MAX 138

struct block_header {
        bool bfinal;
        int type;
        size_t num_litlen_lens;
        size_t num_dist_lens;
        int code_lengths[MIN_LITLEN_LENS + MAX_LITLEN_LENS];
};

static const int codelen_lengths_order[MAX_CODELEN_LENS] =
{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static struct block_header read_block_header(istream_t *is)
{
        struct block_header h;
        uint64_t bits;
        size_t num_codelen_lens, used, i, n;
        uint8_t codelen_lengths[MAX_CODELEN_LENS];
        huffman_decoder_t codelen_dec;
        int sym;

        bits = istream_bits(is);
        h.bfinal = bits & 1;
        bits >>= 1;

        h.type = (int)lsb(bits, 2);
        istream_advance(is, 3);

        if (h.type != 2) {
                return h;
        }

        bits = istream_bits(is);

        /* Number of litlen codeword lengths (5 bits + 257). */
        h.num_litlen_lens = (size_t)(lsb(bits, 5) + MIN_LITLEN_LENS);
        bits >>= 5;
        assert(h.num_litlen_lens <= MAX_LITLEN_LENS);

        /* Number of dist codeword lengths (5 bits + 1). */
        h.num_dist_lens = (size_t)(lsb(bits, 5) + MIN_DIST_LENS);
        bits >>= 5;
        assert(h.num_dist_lens <= MAX_DIST_LENS);

        /* Number of code length lengths (4 bits + 4). */
        num_codelen_lens = (size_t)(lsb(bits, 4) + MIN_CODELEN_LENS);
        bits >>= 4;
        assert(num_codelen_lens <= MAX_CODELEN_LENS);

        istream_advance(is, 5 + 5 + 4);


        /* Read the codelen codeword lengths (3 bits each)
           and initialize the codelen decoder. */
        for (i = 0; i < num_codelen_lens; i++) {
                bits = istream_bits(is);
                codelen_lengths[codelen_lengths_order[i]] =
                        (uint8_t)lsb(bits, 3);
                istream_advance(is, 3);
        }
        for (; i < MAX_CODELEN_LENS; i++) {
                codelen_lengths[codelen_lengths_order[i]] = 0;
        }
        huffman_decoder_init(&codelen_dec, codelen_lengths, MAX_CODELEN_LENS);


        /* Read the litlen and dist codeword lengths. */
        i = 0;
        while (i < h.num_litlen_lens + h.num_dist_lens) {
                bits = istream_bits(is);
                sym = huffman_decode(&codelen_dec, (uint16_t)bits, &used);
                bits >>= used;
                istream_advance(is, used);
                assert(sym != -1);

                if (sym >= 0 && sym <= CODELEN_MAX_LIT) {
                        /* A literal codeword length. */
                        h.code_lengths[i++] = (uint8_t)sym;
                } else if (sym == CODELEN_COPY) {
                        /* Copy the previous codeword length 3--6 times. */
                        /* 2 bits + 3 */
                        n = (size_t)lsb(bits, 2) + CODELEN_COPY_MIN;
                        istream_advance(is, 2);
                        assert(n >= CODELEN_COPY_MIN && n <= CODELEN_COPY_MAX);
                        while (n--) {
                                h.code_lengths[i] = h.code_lengths[i - 1];
                                i++;
                        }
                } else if (sym == CODELEN_ZEROS) {
                        /* 3--10 zeros; 3 bits + 3 */
                        n = (size_t)(lsb(bits, 3) + CODELEN_ZEROS_MIN);
                        istream_advance(is, 3);
                        assert(n >= CODELEN_ZEROS_MIN &&
                               n <= CODELEN_ZEROS_MAX);
                        while (n--) {
                                h.code_lengths[i++] = 0;
                        }
                } else if (sym == CODELEN_ZEROS2) {
                        /* 11--138 zeros; 7 bits + 138. */
                        n = (size_t)(lsb(bits, 7) + CODELEN_ZEROS2_MIN);
                        istream_advance(is, 7);
                        assert(n >= CODELEN_ZEROS2_MIN &&
                               n <= CODELEN_ZEROS2_MAX);
                        while (n--) {
                                h.code_lengths[i++] = 0;
                        }
                }
        }

        return h;
}

void test_deflate_no_dist_codes(void)
{
        /* Compressing this will not use any dist codes, but check that we
           still encode two non-zero dist codes to be compatible with old
           zlib versions. */
        static const uint8_t src[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };

        uint8_t compressed[1000];
        size_t compressed_sz;
        istream_t is;
        struct block_header h;

        CHECK(hwdeflate(src, sizeof(src),
                        compressed, sizeof(compressed), &compressed_sz));

        istream_init(&is, compressed, compressed_sz);
        h = read_block_header(&is);

        CHECK(h.num_dist_lens == 2);
        CHECK(h.code_lengths[h.num_litlen_lens + 0] == 1);
        CHECK(h.code_lengths[h.num_litlen_lens + 1] == 1);
}
