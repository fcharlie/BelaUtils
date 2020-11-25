/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t reverse8(uint8_t x)
{
        int i;
        uint8_t res;

        res = 0;
        for (i = 0; i < 8; i++) {
                /* Check whether the ith least significant bit is set. */
                if (x & (1U << i)) {
                        /* Set the ith most significant bit. */
                        res |= 1U << (8 - 1 - i);
                }
        }

        return res;
}

static void print_reverse8(void)
{
        int i;

        printf("const uint8_t reverse8_tbl[UINT8_MAX + 1] = {\n");

        for (i = 0; i <= UINT8_MAX; i++) {
                printf("/* 0x%02x */ 0x%02x,\n", i, reverse8((uint8_t)i));
        }

        printf("};\n\n");
}

static void print_fixed_litlen_lengths(void)
{
        int i;

        printf("const uint8_t fixed_litlen_lengths[288] = {\n");

        /* RFC 1951, 3.2.6 */
        for (i = 0; i <= 143; i++) { printf("/* %3d */ %d,\n", i, 8); }
        for (     ; i <= 255; i++) { printf("/* %3d */ %d,\n", i, 9); }
        for (     ; i <= 279; i++) { printf("/* %3d */ %d,\n", i, 7); }
        for (     ; i <= 287; i++) { printf("/* %3d */ %d,\n", i, 8); }

        printf("};\n\n");
}

static void print_fixed_dist_lengths(void)
{
        int i;

        printf ("const uint8_t fixed_dist_lengths[32] = {\n");

        /* RFC 1951, 3.2.6 */
        for (i = 0; i < 32; i++) { printf("/* %2d */ %d,\n", i, 5); }

        printf("};\n\n");
}

/* RFC 1951, 3.2.5 */
static struct {
        uint16_t litlen;
        uint16_t base_len;
        uint16_t ebits;
} litlen_tbl[29] = {
        { 257, 3,   0 },
        { 258, 4,   0 },
        { 259, 5,   0 },
        { 260, 6,   0 },
        { 261, 7,   0 },
        { 262, 8,   0 },
        { 263, 9,   0 },
        { 264, 10,  0 },
        { 265, 11,  1 },
        { 266, 13,  1 },
        { 267, 15,  1 },
        { 268, 17,  1 },
        { 269, 19,  2 },
        { 270, 23,  2 },
        { 271, 27,  2 },
        { 272, 31,  2 },
        { 273, 35,  3 },
        { 274, 43,  3 },
        { 275, 51,  3 },
        { 276, 59,  3 },
        { 277, 67,  4 },
        { 278, 83,  4 },
        { 279, 99,  4 },
        { 280, 115, 4 },
        { 281, 131, 5 },
        { 282, 163, 5 },
        { 283, 195, 5 },
        { 284, 227, 5 },
        { 285, 258, 0 }
};

static void print_litlen_tbl(void)
{
        size_t i;

        printf("const struct litlen_tbl_t litlen_tbl[29] = {\n");

        for (i = 0; i < sizeof(litlen_tbl) / sizeof(litlen_tbl[0]); i++) {
                printf("/* %3u */ { %u, %u },\n", litlen_tbl[i].litlen,
                       litlen_tbl[i].base_len, litlen_tbl[i].ebits);
        }
        printf("};\n\n");
}

static void print_len2litlen(void)
{
        size_t i, len;

        printf("const uint16_t len2litlen[259] = {\n");

        /* Lengths 0, 1, 2 are not valid. */
        printf("/*   0 */ 0xffff,\n");
        printf("/*   1 */ 0xffff,\n");
        printf("/*   2 */ 0xffff,\n");

        i = 0;
        for (len = 3; len <= 258; len++) {
                if (len == litlen_tbl[i + 1].base_len) {
                        i++;
                }
                printf("/* %3u */ %u,\n", (unsigned)len, litlen_tbl[i].litlen);
        }

        printf("};\n\n");
}

/* RFC 1951, 3.2.5 */
static struct {
        uint16_t dist;
        uint16_t base_dist;
        uint16_t ebits;
} dist_tbl[30] = {
        { 0,  1,      0 },
        { 1,  2,      0 },
        { 2,  3,      0 },
        { 3,  4,      0 },
        { 4,  5,      1 },
        { 5,  7,      1 },
        { 6,  9,      2 },
        { 7,  13,     2 },
        { 8,  17,     3 },
        { 9,  25,     3 },
        { 10, 33,     4 },
        { 11, 49,     4 },
        { 12, 65,     5 },
        { 13, 97,     5 },
        { 14, 129,    6 },
        { 15, 193,    6 },
        { 16, 257,    7 },
        { 17, 385,    7 },
        { 18, 513,    8 },
        { 19, 769,    8 },
        { 20, 1025,   9 },
        { 21, 1537,   9 },
        { 22, 2049,  10 },
        { 23, 3073,  10 },
        { 24, 4097,  11 },
        { 25, 6145,  11 },
        { 26, 8193,  12 },
        { 27, 12289, 12 },
        { 28, 16385, 13 },
        { 29, 24577, 13 }
};

static void print_dist_tbl(void)
{
        size_t i;

        printf("const struct dist_tbl_t dist_tbl[30] = {\n");

        for (i = 0; i < sizeof(dist_tbl) / sizeof(dist_tbl[0]); i++) {
                printf("/* %2u */ { %u, %u },\n", dist_tbl[i].dist,
                       dist_tbl[i].base_dist, dist_tbl[i].ebits);
        }
        printf("};\n\n");
}

static void print_distance2dist(void)
{
        uint8_t distance2dist_lo[256];
        uint8_t distance2dist_hi[256];
        uint8_t dist;
        size_t distance;
        unsigned i;

        /* For each distance. */
        for (distance = 1; distance <= 32768; distance++) {
                /* Find the corresponding dist code. */
                dist = 29;
                while (dist_tbl[dist].base_dist > distance) {
                        dist--;
                }

                if (distance <= 256) {
                        distance2dist_lo[(distance - 1)] = dist;
                } else {
                        distance2dist_hi[(distance - 1) >> 7] = dist;
                }
        }

        printf("const uint8_t distance2dist_lo[256] = {\n");
        for (i = 0; i < 256; i++) {
                printf("%4u, /* %3u */\n", distance2dist_lo[i], i + 1);
        }
        printf("};\n\n");

        printf("const uint8_t distance2dist_hi[256] = {\n");
        for (i = 0; i < 256; i++) {
                if (i < 2) {
                        printf("0xff, /* invalid */\n");
                        continue;
                }

                printf("%4u, /* %5u-- */\n", distance2dist_hi[i], (i << 7) + 1);
        }
        printf("};\n\n");
}

static uint32_t crc32_tab_entry(int idx)
{
        /* Based on Figure 14-7 in Hacker's Delight (2nd ed.) Equivalent to
           cm_tab() in https://www.zlib.net/crc_v3.txt with
           cm_width = 32, cm_refin = true, cm_poly = 0xcbf43926. */

        int i;
        uint32_t crc = (uint32_t)idx;
        uint32_t mask;

        assert(idx >= 0 && idx <= 255);

        for (i = 0; i < 8; i++) {
                mask = -(crc & 1);
                crc = (crc >> 1) ^ (0xedb88320 & mask);
        }

        return crc;
}

static void print_crc32_tbl(void)
{
        int i;

        printf("const uint32_t crc32_tbl[256] = {\n");

        for (i = 0; i < 256; i++) {
                printf("0x%08x,\n", crc32_tab_entry(i));
        }

        printf("};\n\n");
}

int main()
{
        printf("/* DO NOT EDIT! Generated by generate_tables.c. */\n\n");
        printf("#include \"tables.h\"\n\n");

        print_reverse8();
        print_fixed_litlen_lengths();
        print_fixed_dist_lengths();
        print_litlen_tbl();
        print_len2litlen();
        print_dist_tbl();
        print_distance2dist();
        print_crc32_tbl();

        return 0;
}
