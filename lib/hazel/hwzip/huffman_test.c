/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "huffman.h"
#include "test_utils.h"

void test_huffman_decode_basic(void)
{
        static const uint8_t lens[] = {
                3, /* sym 0:  000 */
                3, /* sym 1:  001 */
                3, /* sym 2:  010 */
                3, /* sym 3:  011 */
                3, /* sym 4:  100 */
                3, /* sym 5:  101 */
                4, /* sym 6:  1100 */
                4, /* sym 7:  1101 */
                0, /* sym 8:  */
                0, /* sym 9:  */
                0, /* sym 10: */
                0, /* sym 11: */
                0, /* sym 12: */
                0, /* sym 13: */
                0, /* sym 14: */
                0, /* sym 15: */
                6, /* sym 16: 111110 */
                5, /* sym 17: 11110 */
                4  /* sym 18: 1110 */
        };

        huffman_decoder_t d;
        size_t used;

        CHECK(huffman_decoder_init(&d, lens, sizeof(lens) / sizeof(lens[0])));

        /* 000 (msb-first) -> 000 (lsb-first)*/
        CHECK(huffman_decode(&d, 0x0, &used) == 0); CHECK(used == 3);

        /* 011 (msb-first) -> 110 (lsb-first)*/
        CHECK(huffman_decode(&d, 0x6, &used) == 3); CHECK(used == 3);

        /* 11110 (msb-first) -> 01111 (lsb-first)*/
        CHECK(huffman_decode(&d, 0x0f, &used) == 17); CHECK(used == 5);

        /* 111110 (msb-first) -> 011111 (lsb-first)*/
        CHECK(huffman_decode(&d, 0x1f, &used) == 16); CHECK(used == 6);

        /* 1111111 (msb-first) -> 1111111 (lsb-first)*/
        CHECK(huffman_decode(&d, 0x7f, &used) == -1);

        /* Make sure used is set even when decoding fails. */
        CHECK(used == 0);
}

void test_huffman_decode_canonical(void)
{
        /* Long enough codewords to not just hit the lookup table. */
        static const uint8_t lens[] = {
                3,  /* sym 0: 0000 (0x0) */
                3,  /* sym 1: 0001 (0x1) */
                3,  /* sym 2: 0010 (0x2) */
                15, /* sym 3: 0011 0000 0000 0000 (0x3000) */
                15, /* sym 4: 0011 0000 0000 0001 (0x3001) */
                15, /* sym 5: 0011 0000 0000 0010 (0x3002) */
        };

        huffman_decoder_t d;
        size_t used;

        CHECK(huffman_decoder_init(&d, lens, sizeof(lens) / sizeof(lens[0])));

        CHECK(huffman_decode(&d, reverse16(0x0, 3), &used) == 0);
        CHECK(used == 3);
        CHECK(huffman_decode(&d, reverse16(0x1, 3), &used) == 1);
        CHECK(used == 3);
        CHECK(huffman_decode(&d, reverse16(0x2, 3), &used) == 2);
        CHECK(used == 3);

        CHECK(huffman_decode(&d, reverse16(0x3000, 15), &used) == 3);
        CHECK(used == 15);
        CHECK(huffman_decode(&d, reverse16(0x3001, 15), &used) == 4);
        CHECK(used == 15);
        CHECK(huffman_decode(&d, reverse16(0x3002, 15), &used) == 5);
        CHECK(used == 15);
        CHECK(huffman_decode(&d, reverse16(0x3000, 15), &used) == 3);
        CHECK(used == 15);
}

void test_huffman_decode_evil(void)
{
        /* More length-4 symbols than are possible. This will overflow the
           sentinel bits. */
        static const uint8_t lens[] = {
                1,
                2,
                3,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4
        };

        huffman_decoder_t d;

        CHECK(!huffman_decoder_init(&d, lens, sizeof(lens) / sizeof(lens[0])));
}

void test_huffman_decode_empty(void)
{
        huffman_decoder_t d;
        size_t used;

        CHECK(huffman_decoder_init(&d, NULL, 0));

        /* Hit the lookup table. */
        CHECK(huffman_decode(&d, 0x0, &used) == -1);

        /* Hit the canonical decoder. */
        CHECK(huffman_decode(&d, 0xffff, &used) == -1);
}

void test_huffman_encoder_init(void)
{
        static const uint16_t freqs[] = {
                /* 0  */ 8,
                /* 1  */ 1,
                /* 2  */ 1,
                /* 3  */ 2,
                /* 4  */ 5,
                /* 5  */ 10,
                /* 6  */ 9,
                /* 7  */ 1,
                /* 8  */ 0,
                /* 9  */ 0,
                /* 10 */ 0,
                /* 11 */ 0,
                /* 12 */ 0,
                /* 13 */ 0,
                /* 14 */ 0,
                /* 15 */ 0,
                /* 16 */ 1,
                /* 17 */ 3,
                /* 18 */ 5,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 6);

        CHECK(e.lengths[0 ] == 3);
        CHECK(e.lengths[1 ] == 6);
        CHECK(e.lengths[2 ] == 6);
        CHECK(e.lengths[3 ] == 5);
        CHECK(e.lengths[4 ] == 3);
        CHECK(e.lengths[5 ] == 2);
        CHECK(e.lengths[6 ] == 2);
        CHECK(e.lengths[7 ] == 6);
        CHECK(e.lengths[8 ] == 0);
        CHECK(e.lengths[9 ] == 0);
        CHECK(e.lengths[10] == 0);
        CHECK(e.lengths[11] == 0);
        CHECK(e.lengths[12] == 0);
        CHECK(e.lengths[13] == 0);
        CHECK(e.lengths[14] == 0);
        CHECK(e.lengths[15] == 0);
        CHECK(e.lengths[16] == 6);
        CHECK(e.lengths[17] == 5);
        CHECK(e.lengths[18] == 3);

        CHECK(e.codewords[5 ] == 0x0);
        CHECK(e.codewords[6 ] == 0x2);
        CHECK(e.codewords[0 ] == 0x1);
        CHECK(e.codewords[4 ] == 0x5);
        CHECK(e.codewords[18] == 0x3);
        CHECK(e.codewords[3 ] == 0x7);
        CHECK(e.codewords[17] == 0x17);
        CHECK(e.codewords[1 ] == 0x0f);
        CHECK(e.codewords[2 ] == 0x2f);
        CHECK(e.codewords[7 ] == 0x1f);
        CHECK(e.codewords[16] == 0x3f);
}

void test_huffman_lengths_one(void)
{
        static const uint16_t freqs[] = {
                /* 0 */ 0,
                /* 1 */ 0,
                /* 2 */ 0,
                /* 3 */ 4,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 6);

        CHECK(e.lengths[0] == 0);
        CHECK(e.lengths[1] == 0);
        CHECK(e.lengths[2] == 0);
        CHECK(e.lengths[3] == 1);
}

void test_huffman_lengths_two(void)
{
        static const uint16_t freqs[] = {
                /* 0 */ 1,
                /* 1 */ 0,
                /* 2 */ 0,
                /* 3 */ 4,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 6);

        CHECK(e.lengths[0] == 1);
        CHECK(e.lengths[1] == 0);
        CHECK(e.lengths[2] == 0);
        CHECK(e.lengths[3] == 1);
}


void test_huffman_lengths_none(void)
{
        static const uint16_t freqs[] = {
                /* 0 */ 0,
                /* 1 */ 0,
                /* 2 */ 0,
                /* 3 */ 0,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 6);

        CHECK(e.lengths[0] == 0);
        CHECK(e.lengths[1] == 0);
        CHECK(e.lengths[2] == 0);
        CHECK(e.lengths[3] == 0);
}

void test_huffman_lengths_limited(void)
{
        static const uint16_t freqs[] = {
                /* 0 */ 1,
                /* 1 */ 2,
                /* 2 */ 4,
                /* 3 */ 8,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 2);

        CHECK(e.lengths[0] == 2);
        CHECK(e.lengths[1] == 2);
        CHECK(e.lengths[2] == 2);
        CHECK(e.lengths[3] == 2);
}

void test_huffman_lengths_max_freq(void)
{
        static const uint16_t freqs[] = {
                /* 0 */ 16383,
                /* 1 */ 16384,
                /* 2 */ 16384,
                /* 3 */ 16384
        };

        huffman_encoder_t e;

        assert(freqs[0] + freqs[1] + freqs[2] + freqs[3] == UINT16_MAX);
        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 10);

        CHECK(e.lengths[0] == 2);
        CHECK(e.lengths[1] == 2);
        CHECK(e.lengths[2] == 2);
        CHECK(e.lengths[3] == 2);
}

void test_huffman_lengths_max_syms(void)
{
        uint16_t freqs[MAX_HUFFMAN_SYMBOLS];
        huffman_encoder_t e;
        size_t i;

        for (i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
                freqs[i] = 1;
        }

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 15);

        for (i = 0; i < MAX_HUFFMAN_SYMBOLS; i++) {
                CHECK(e.lengths[i] == 8 || e.lengths[i] == 9);
        }
}

void test_huffman_encoder_init2(void)
{
        uint8_t lens[288];
        size_t i;
        huffman_encoder_t e;

        /* Code lengths used for fixed Huffman code deflate blocks. */
        for (i = 0; i <= 143; i++) {
                lens[i] = 8;
        }
        for (; i <= 255; i++) {
                lens[i] = 9;
        }
        for (; i <= 279; i++) {
                lens[i] = 7;
        }
        for (; i <= 287; i++) {
                lens[i] = 8;
        }

        huffman_encoder_init2(&e, lens, sizeof(lens) / sizeof(lens[0]));

        CHECK(e.codewords[255] == 0x1ff);
}
