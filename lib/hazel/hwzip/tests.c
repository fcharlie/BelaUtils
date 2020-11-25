/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <stddef.h>
#include <stdio.h>

#define TESTS \
TEST(bits_lsb) \
TEST(bits_read16le) \
TEST(bits_read32le) \
TEST(bits_read64le) \
TEST(bits_reverse16) \
TEST(bits_round_up) \
TEST(bits_write16le) \
TEST(bits_write32le) \
TEST(bits_write64le) \
TEST(crc32_basic) \
TEST(deflate_basic) \
TEST(deflate_hamlet) \
TEST(deflate_mixed_blocks) \
TEST(deflate_random) \
TEST(deflate_no_dist_codes) \
TEST(huffman_decode_basic) \
TEST(huffman_decode_canonical) \
TEST(huffman_decode_empty) \
TEST(huffman_decode_evil) \
TEST(huffman_encoder_init) \
TEST(huffman_encoder_init2) \
TEST(huffman_lengths_limited) \
TEST(huffman_lengths_max_freq) \
TEST(huffman_lengths_max_syms) \
TEST(huffman_lengths_none) \
TEST(huffman_lengths_one) \
TEST(huffman_lengths_two) \
TEST(inflate_hamlet) \
TEST(inflate_invalid_block_header) \
TEST(inflate_no_dist_codes) \
TEST(inflate_twocities_intro) \
TEST(inflate_uncompressed) \
TEST(istream_basic) \
TEST(lz77_aaa) \
TEST(lz77_backref) \
TEST(lz77_chain) \
TEST(lz77_deferred) \
TEST(lz77_empty) \
TEST(lz77_hamlet) \
TEST(lz77_literals) \
TEST(lz77_output_fail) \
TEST(lz77_outside_window) \
TEST(lz77_remaining_backref) \
TEST(ostream_basic) \
TEST(ostream_write_bytes_aligned) \
TEST(zip_bad_stored_uncomp_size) \
TEST(zip_basic) \
TEST(zip_empty) \
TEST(zip_in_zip) \
TEST(zip_max_comment) \
TEST(zip_out_of_bounds_member) \
TEST(zip_pk) \
TEST(zip_write_basic) \
TEST(zip_write_empty)

/* Declarations for the test functions. */
#define TEST(name) extern void test_ ## name(void);
TESTS
#undef TEST

/* Array of test names and functions. */
static struct {
        const char *name;
        void (*func)(void);
} const tests[] = {
#define TEST(name) { #name, test_ ## name },
TESTS
#undef TEST
};

int main() {
        size_t i;

        for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
                printf("%-32s", tests[i].name);
                fflush(stdout);
                (tests[i].func)();
                printf(" pass\n");
        }

        return 0;
}
