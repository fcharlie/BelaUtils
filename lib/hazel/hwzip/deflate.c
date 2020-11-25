/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "deflate.h"

#include <assert.h>
#include <string.h>
#include "bitstream.h"
#include "huffman.h"
#include "lz77.h"
#include "tables.h"

#define LITLEN_EOB 256
#define LITLEN_MAX 285
#define LITLEN_TBL_OFFSET 257
#define MIN_LEN 3
#define MAX_LEN 258

#define DISTSYM_MAX 29
#define MIN_DISTANCE 1
#define MAX_DISTANCE 32768

#define MIN_CODELEN_LENS 4
#define MAX_CODELEN_LENS 19

#define MIN_LITLEN_LENS 257
#define MAX_LITLEN_LENS 288

#define MIN_DIST_LENS 1
#define MAX_DIST_LENS 32

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

/* Output the (dist,len) backref at dst_pos in dst using 64-bit wide writes.
   There must be enough room for len bytes rounded to the next multiple of 8. */
static void output_backref64(uint8_t *dst, size_t dst_pos, size_t dist,
                             size_t len)
{
        size_t i;
        uint64_t tmp;

        assert(len > 0);
        assert(dist <= dst_pos && "cannot reference before beginning of dst");

        if (len > dist) {
                /* Self-overlapping backref; fall back to byte-by-byte copy. */
                lz77_output_backref(dst, dst_pos, dist, len);
                return;
        }

        i = 0;
        do {
                memcpy(&tmp, &dst[dst_pos - dist + i], 8);
                memcpy(&dst[dst_pos + i], &tmp, 8);
                i += 8;
        } while (i < len);
}

static inf_stat_t inf_block(istream_t *is, uint8_t *dst, size_t dst_cap,
                            size_t *dst_pos,
                            const huffman_decoder_t *litlen_dec,
                            const huffman_decoder_t *dist_dec)
{
        uint64_t bits;
        size_t used, used_tot, dist, len;
        int litlen, distsym;
        uint16_t ebits;

        while (true) {
                /* Read a litlen symbol. */
                bits = istream_bits(is);
                litlen = huffman_decode(litlen_dec, (uint16_t)bits, &used);
                bits >>= used;
                used_tot = used;

                if (litlen < 0 || litlen > LITLEN_MAX) {
                        /* Failed to decode, or invalid symbol. */
                        return HWINF_ERR;
                } else if (litlen <= UINT8_MAX) {
                        /* Literal. */
                        if (!istream_advance(is, used_tot)) {
                                return HWINF_ERR;
                        }
                        if (*dst_pos == dst_cap) {
                                return HWINF_FULL;
                        }
                        lz77_output_lit(dst, (*dst_pos)++, (uint8_t)litlen);
                        continue;
                } else if (litlen == LITLEN_EOB) {
                        /* End of block. */
                        if (!istream_advance(is, used_tot)) {
                                return HWINF_ERR;
                        }
                        return HWINF_OK;
                }

                /* It is a back reference. Figure out the length. */
                assert(litlen >= LITLEN_TBL_OFFSET && litlen <= LITLEN_MAX);
                len   = litlen_tbl[litlen - LITLEN_TBL_OFFSET].base_len;
                ebits = litlen_tbl[litlen - LITLEN_TBL_OFFSET].ebits;
                if (ebits != 0) {
                        len += lsb(bits, ebits);
                        bits >>= ebits;
                        used_tot += ebits;
                }
                assert(len >= MIN_LEN && len <= MAX_LEN);

                /* Get the distance. */
                distsym = huffman_decode(dist_dec, (uint16_t)bits, &used);
                bits >>= used;
                used_tot += used;

                if (distsym < 0 || distsym > DISTSYM_MAX) {
                        /* Failed to decode, or invalid symbol. */
                        return HWINF_ERR;
                }
                dist  = dist_tbl[distsym].base_dist;
                ebits = dist_tbl[distsym].ebits;
                if (ebits != 0) {
                        dist += lsb(bits, ebits);
                        bits >>= ebits;
                        used_tot += ebits;
                }
                assert(dist >= MIN_DISTANCE && dist <= MAX_DISTANCE);

                assert(used_tot <= ISTREAM_MIN_BITS);
                if (!istream_advance(is, used_tot)) {
                        return HWINF_ERR;
                }

                /* Bounds check and output the backref. */
                if (dist > *dst_pos) {
                        return HWINF_ERR;
                }
                if (round_up(len, 8) <= dst_cap - *dst_pos) {
                        output_backref64(dst, *dst_pos, dist, len);
                } else if (len <= dst_cap - *dst_pos) {
                        lz77_output_backref(dst, *dst_pos, dist, len);
                } else {
                        return HWINF_FULL;
                }
                (*dst_pos) += len;
        }
}

/* RFC 1951, 3.2.7 */
static const int codelen_lengths_order[MAX_CODELEN_LENS] =
{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static inf_stat_t init_dyn_decoders(istream_t *is,
                                    huffman_decoder_t *litlen_dec,
                                    huffman_decoder_t *dist_dec)
{
        uint64_t bits;
        size_t num_litlen_lens, num_dist_lens, num_codelen_lens;
        uint8_t codelen_lengths[MAX_CODELEN_LENS];
        uint8_t code_lengths[MAX_LITLEN_LENS + MAX_DIST_LENS];
        size_t i, n, used;
        int sym;
        huffman_decoder_t codelen_dec;

        bits = istream_bits(is);

        /* Number of litlen codeword lengths (5 bits + 257). */
        num_litlen_lens = (size_t)(lsb(bits, 5) + MIN_LITLEN_LENS);
        bits >>= 5;
        assert(num_litlen_lens <= MAX_LITLEN_LENS);

        /* Number of dist codeword lengths (5 bits + 1). */
        num_dist_lens = (size_t)(lsb(bits, 5) + MIN_DIST_LENS);
        bits >>= 5;
        assert(num_dist_lens <= MAX_DIST_LENS);

        /* Number of code length lengths (4 bits + 4). */
        num_codelen_lens = (size_t)(lsb(bits, 4) + MIN_CODELEN_LENS);
        bits >>= 4;
        assert(num_codelen_lens <= MAX_CODELEN_LENS);

        if (!istream_advance(is, 5 + 5 + 4)) {
                return HWINF_ERR;
        }


        /* Read the codelen codeword lengths (3 bits each)
           and initialize the codelen decoder. */
        for (i = 0; i < num_codelen_lens; i++) {
                bits = istream_bits(is);
                codelen_lengths[codelen_lengths_order[i]] =
                        (uint8_t)lsb(bits, 3);
                if (!istream_advance(is, 3)) {
                        return HWINF_ERR;
                }
        }
        for (; i < MAX_CODELEN_LENS; i++) {
                codelen_lengths[codelen_lengths_order[i]] = 0;
        }
        if (!huffman_decoder_init(&codelen_dec, codelen_lengths,
                                  MAX_CODELEN_LENS)) {
                return HWINF_ERR;
        }


        /* Read the litlen and dist codeword lengths. */
        i = 0;
        while (i < num_litlen_lens + num_dist_lens) {
                bits = istream_bits(is);
                sym = huffman_decode(&codelen_dec, (uint16_t)bits, &used);
                bits >>= used;
                if (!istream_advance(is, used)) {
                        return HWINF_ERR;
                }

                if (sym >= 0 && sym <= CODELEN_MAX_LIT) {
                        /* A literal codeword length. */
                        code_lengths[i++] = (uint8_t)sym;
                } else if (sym == CODELEN_COPY) {
                        /* Copy the previous codeword length 3--6 times. */
                        if (i < 1) {
                                return HWINF_ERR; /* No previous length. */
                        }
                        /* 2 bits + 3 */
                        n = (size_t)lsb(bits, 2) + CODELEN_COPY_MIN;
                        if (!istream_advance(is, 2)) {
                                return HWINF_ERR;
                        }
                        assert(n >= CODELEN_COPY_MIN && n <= CODELEN_COPY_MAX);
                        if (i + n > num_litlen_lens + num_dist_lens) {
                                return HWINF_ERR;
                        }
                        while (n--) {
                                code_lengths[i] = code_lengths[i - 1];
                                i++;
                        }
                } else if (sym == CODELEN_ZEROS) {
                        /* 3--10 zeros; 3 bits + 3 */
                        n = (size_t)(lsb(bits, 3) + CODELEN_ZEROS_MIN);
                        if (!istream_advance(is, 3)) {
                                return HWINF_ERR;
                        }
                        assert(n >= CODELEN_ZEROS_MIN &&
                               n <= CODELEN_ZEROS_MAX);
                        if (i + n > num_litlen_lens + num_dist_lens) {
                                return HWINF_ERR;
                        }
                        while (n--) {
                                code_lengths[i++] = 0;
                        }
                } else if (sym == CODELEN_ZEROS2) {
                        /* 11--138 zeros; 7 bits + 138. */
                        n = (size_t)(lsb(bits, 7) + CODELEN_ZEROS2_MIN);
                        if (!istream_advance(is, 7)) {
                                return HWINF_ERR;
                        }
                        assert(n >= CODELEN_ZEROS2_MIN &&
                               n <= CODELEN_ZEROS2_MAX);
                        if (i + n > num_litlen_lens + num_dist_lens) {
                                return HWINF_ERR;
                        }
                        while (n--) {
                                code_lengths[i++] = 0;
                        }
                } else {
                        /* Invalid symbol. */
                        return HWINF_ERR;
                }
        }

        if (!huffman_decoder_init(litlen_dec, &code_lengths[0],
                                  num_litlen_lens)) {
                return HWINF_ERR;
        }
        if (!huffman_decoder_init(dist_dec, &code_lengths[num_litlen_lens],
                                  num_dist_lens)) {
                return HWINF_ERR;
        }

        return HWINF_OK;
}

static inf_stat_t inf_dyn_block(istream_t *is, uint8_t *dst,
                                size_t dst_cap, size_t *dst_pos)
{
        inf_stat_t s;
        huffman_decoder_t litlen_dec, dist_dec;

        s = init_dyn_decoders(is, &litlen_dec, &dist_dec);
        if (s != HWINF_OK) {
                return s;
        }

        return inf_block(is, dst, dst_cap, dst_pos, &litlen_dec, &dist_dec);
}

static inf_stat_t inf_fixed_block(istream_t *is, uint8_t *dst,
                                  size_t dst_cap, size_t *dst_pos)
{
        huffman_decoder_t litlen_dec, dist_dec;

        huffman_decoder_init(&litlen_dec, fixed_litlen_lengths,
                             sizeof(fixed_litlen_lengths) /
                             sizeof(fixed_litlen_lengths[0]));
        huffman_decoder_init(&dist_dec, fixed_dist_lengths,
                             sizeof(fixed_dist_lengths) /
                             sizeof(fixed_dist_lengths[0]));

        return inf_block(is, dst, dst_cap, dst_pos, &litlen_dec, &dist_dec);
}

static inf_stat_t inf_noncomp_block(istream_t *is, uint8_t *dst,
                                    size_t dst_cap, size_t *dst_pos)
{
        const uint8_t *p;
        uint16_t len, nlen;

        p = istream_byte_align(is);

        /* Read len and nlen (2 x 16 bits). */
        if (!istream_advance(is, 32)) {
                return HWINF_ERR; /* Not enough input. */
        }
        len  = read16le(p);
        nlen = read16le(p + 2);
        p += 4;

        if (nlen != (uint16_t)~len) {
                return HWINF_ERR;
        }

        if (!istream_advance(is, len * 8)) {
                return HWINF_ERR; /* Not enough input. */
        }

        if (dst_cap - *dst_pos < len) {
                return HWINF_FULL; /* Not enough room to output. */
        }

        memcpy(&dst[*dst_pos], p, len);
        *dst_pos += len;

        return HWINF_OK;
}

inf_stat_t hwinflate(const uint8_t *src, size_t src_len, size_t *src_used,
                     uint8_t *dst, size_t dst_cap, size_t *dst_used)
{
        istream_t is;
        size_t dst_pos;
        uint64_t bits;
        bool bfinal;
        inf_stat_t s;

        istream_init(&is, src, src_len);
        dst_pos = 0;

        do {
                /* Read the 3-bit block header. */
                bits = istream_bits(&is);
                if (!istream_advance(&is, 3)) {
                        return HWINF_ERR;
                }
                bfinal = bits & 1;
                bits >>= 1;

                switch (lsb(bits, 2)) {
                case 0: /* 00: No compression. */
                        s = inf_noncomp_block(&is, dst, dst_cap, &dst_pos);
                        break;
                case 1: /* 01: Compressed with fixed Huffman codes. */
                        s = inf_fixed_block(&is, dst, dst_cap, &dst_pos);
                        break;
                case 2: /* 10: Compressed with "dynamic" Huffman codes. */
                        s = inf_dyn_block(&is, dst, dst_cap, &dst_pos);
                        break;
                default: /* Invalid block type. */
                        return HWINF_ERR;
                }

                if (s != HWINF_OK) {
                        return s;
                }
        } while (!bfinal);

        *src_used = (size_t)(istream_byte_align(&is) - src);

        assert(dst_pos <= dst_cap);
        *dst_used = dst_pos;

        return HWINF_OK;
}


/* The largest number of bytes that will fit in any kind of block is 65,534.
   It will fit in an uncompressed block (max 65,535 bytes) and a Huffman
   block with only literals (65,535 symbols including end-of-block marker). */
#define MAX_BLOCK_LEN_BYTES 65534

typedef struct deflate_state_t deflate_state_t;
struct deflate_state_t {
        ostream_t os;

        const uint8_t *block_src; /* First src byte in the block. */

        size_t block_len;       /* Number of symbols in the current block. */
        size_t block_len_bytes; /* Number of src bytes in the block. */

        /* Symbol frequencies for the current block. */
        uint16_t litlen_freqs[LITLEN_MAX + 1];
        uint16_t dist_freqs[DISTSYM_MAX + 1];

        struct {
                uint16_t distance;    /* Backref distance. */
                union {
                        uint16_t lit; /* Literal byte or end-of-block. */
                        uint16_t len; /* Backref length (distance != 0). */
                } u;
        } block[MAX_BLOCK_LEN_BYTES + 1];
};

static void reset_block(deflate_state_t *s)
{
        s->block_len = 0;
        s->block_len_bytes = 0;
        memset(s->litlen_freqs, 0, sizeof(s->litlen_freqs));
        memset(s->dist_freqs, 0, sizeof(s->dist_freqs));
}

typedef struct codelen_sym_t codelen_sym_t;
struct codelen_sym_t {
        uint8_t sym;
        uint8_t count; /* For symbols 16, 17, 18. */
};

static inline size_t min(size_t a, size_t b)
{
        return a < b ? a : b;
}

/* Encode the n code lengths in lens into encoded, returning the number of
   elements in encoded. */
static size_t encode_lens(const uint8_t *lens, size_t n, codelen_sym_t *encoded)
{
        size_t i, j, num_encoded;
        uint8_t count;

        i = 0;
        num_encoded = 0;
        while (i < n) {
                if (lens[i] == 0) {
                        /* Scan past the end of this zero run (max 138). */
                        for (j = i; j < min(n, i + CODELEN_ZEROS2_MAX) &&
                                    lens[j] == 0; j++);
                        count = (uint8_t)(j - i);

                        if (count < CODELEN_ZEROS_MIN) {
                                /* Output a single zero. */
                                encoded[num_encoded++].sym = 0;
                                i++;
                                continue;
                        }

                        /* Output a repeated zero. */
                        if (count <= CODELEN_ZEROS_MAX) {
                                /* Repeated zero 3--10 times. */
                                assert(count >= CODELEN_ZEROS_MIN &&
                                       count <= CODELEN_ZEROS_MAX);
                                encoded[num_encoded].sym = CODELEN_ZEROS;
                                encoded[num_encoded++].count = count;
                        } else {
                                /* Repeated zero 11--138 times. */
                                assert(count >= CODELEN_ZEROS2_MIN &&
                                       count <= CODELEN_ZEROS2_MAX);
                                encoded[num_encoded].sym = CODELEN_ZEROS2;
                                encoded[num_encoded++].count = count;
                        }
                        i = j;
                        continue;
                }

                /* Output len. */
                encoded[num_encoded++].sym = lens[i++];

                /* Scan past the end of the run of this len (max 6). */
                for (j = i; j < min(n, i + CODELEN_COPY_MAX) &&
                            lens[j] == lens[i - 1]; j++);
                count = (uint8_t)(j - i);

                if (count >= CODELEN_COPY_MIN) {
                        /* Repeat last len 3--6 times. */
                        assert(count >= CODELEN_COPY_MIN &&
                               count <= CODELEN_COPY_MAX);
                        encoded[num_encoded].sym = CODELEN_COPY;
                        encoded[num_encoded++].count = count;
                        i = j;
                        continue;
                }
        }

        return num_encoded;
}

/* Zlib always emits two non-zero distance codeword lengths (see build_trees),
   and versions before 1.2.1.1 (and possibly other implementations) would fail
   to inflate compressed data where that's not the case. See comment in
   https://github.com/google/zopfli/blob/zopfli-1.0.3/src/zopfli/deflate.c#L75
   Tweak the encoder to ensure compatibility. */
static void tweak_dist_encoder(huffman_encoder_t *e)
{
        size_t i, n;
        size_t nonzero_idx = SIZE_MAX;

        n = 0;
        for (i = 0; i <= DISTSYM_MAX; i++) {
                if (e->lengths[i] != 0) {
                        n++;
                        nonzero_idx = i;

                        if (n == 2) {
                                return;
                        }
                }
        }

        assert(n < 2);

        if (n == 0) {
                /* Give symbols 0 and 1 codewords of length 1. */
                e->lengths[0] = 1;
                e->lengths[1] = 1;
                e->codewords[0] = 0;
                e->codewords[1] = 1;
                return;
        }

        assert(n == 1);
        assert(nonzero_idx != SIZE_MAX);

        if (nonzero_idx == 0) {
                /* Symbol 0 already has a codeword of length 1.
                   Give symbol 1 a codeword of length 1. */
                assert(e->lengths[0] == 1);
                assert(e->codewords[0] == 0);
                e->lengths[1] = 1;
                e->codewords[1] = 1;
        } else {
                /* Give symbol 0 a codeword of length 1 ("0") and
                   update the other symbol's codeword to "1". */
                assert(e->lengths[0] == 0);
                assert(e->codewords[nonzero_idx] == 0);
                e->lengths[0] = 1;
                e->codewords[0] = 0;
                e->codewords[nonzero_idx] = 1;
        }
}

/* Encode litlen_lens and dist_lens into encoded. *num_litlen_lens and
   *num_dist_lens will be set to the number of encoded litlen and dist lens,
   respectively. Returns the number of elements in encoded. */
static size_t encode_dist_litlen_lens(const uint8_t *litlen_lens,
                                      const uint8_t *dist_lens,
                                      codelen_sym_t *encoded,
                                      size_t *num_litlen_lens,
                                      size_t *num_dist_lens)
{
        size_t i, n;
        uint8_t lens[LITLEN_MAX + 1 + DISTSYM_MAX + 1];

        *num_litlen_lens = LITLEN_MAX + 1;
        *num_dist_lens = DISTSYM_MAX + 1;

        /* Drop trailing zero litlen lengths. */
        assert(litlen_lens[LITLEN_EOB] != 0 && "EOB len should be non-zero.");
        while (litlen_lens[*num_litlen_lens - 1] == 0) {
                (*num_litlen_lens)--;
        }
        assert(*num_litlen_lens >= MIN_LITLEN_LENS);

        /* Drop trailing zero dist lengths, keeping at least one. */
        while (dist_lens[*num_dist_lens - 1] == 0 && *num_dist_lens > 1) {
                (*num_dist_lens)--;
        }
        assert(*num_dist_lens >= MIN_DIST_LENS);

        /* Copy the lengths into a unified array. */
        n = 0;
        for (i = 0; i < *num_litlen_lens; i++) {
                lens[n++] = litlen_lens[i];
        }
        for (i = 0; i < *num_dist_lens; i++) {
                lens[n++] = dist_lens[i];
        }

        return encode_lens(lens, n, encoded);
}

/* Count the number of significant (not trailing zeros) codelen lengths. */
size_t count_codelen_lens(const uint8_t *codelen_lens)
{
        size_t n = MAX_CODELEN_LENS;

        /* Drop trailing zero lengths. */
        while (codelen_lens[codelen_lengths_order[n - 1]] == 0) {
                n--;
        }

        /* The first 4 lengths in the order (16, 17, 18, 0) cannot be used to
           encode any non-zero lengths. Since there will always be at least
           one non-zero codeword length (for EOB), n will be >= 4. */
        assert(n >= MIN_CODELEN_LENS && n <= MAX_CODELEN_LENS);

        return n;
}

/* Calculate the number of bits for an uncompressed block, including header. */
static size_t uncomp_block_len(const deflate_state_t *s)
{
        size_t bit_pos, padding;

        /* Bit position after writing the block header. */
        bit_pos = ostream_bit_pos(&s->os) + 3;
        padding = round_up(bit_pos, 8) - bit_pos;

        /* Header + padding + len/nlen + block contents. */
        return 3 + padding + 2 * 16 + s->block_len_bytes * 8;
}

/* Calculate the number of bits for a Huffman encoded block body. */
static size_t huffman_block_body_len(const deflate_state_t *s,
                                     const uint8_t *litlen_lens,
                                     const uint8_t *dist_lens)
{
        size_t i, freq, len;

        len = 0;

        for (i = 0; i <= LITLEN_MAX; i++) {
                freq = s->litlen_freqs[i];
                len += litlen_lens[i] * freq;

                if (i >= LITLEN_TBL_OFFSET) {
                        len += litlen_tbl[i - LITLEN_TBL_OFFSET].ebits * freq;
                }
        }

        for (i = 0; i <= DISTSYM_MAX; i++) {
                freq = s->dist_freqs[i];
                len += dist_lens[i] * freq;
                len += dist_tbl[i].ebits * freq;
        }

        return len;
}

/* Calculate the number of bits for a dynamic Huffman block. */
static size_t dyn_block_len(const deflate_state_t *s, size_t num_codelen_lens,
                            const uint16_t *codelen_freqs,
                            const huffman_encoder_t *codelen_enc,
                            const huffman_encoder_t *litlen_enc,
                            const huffman_encoder_t *dist_enc)
{
        size_t len, i, freq;

        /* Block header. */
        len = 3;

        /* Nbr of litlen, dist, and codelen lengths. */
        len += 5 + 5 + 4;

        /* Codelen lengths. */
        len += 3 * num_codelen_lens;

        /* Codelen encoding. */
        for (i = 0; i < MAX_CODELEN_LENS; i++) {
                freq = codelen_freqs[i];
                len += codelen_enc->lengths[i] * freq;

                /* Extra bits. */
                if (i == CODELEN_COPY) {
                        len += 2 * freq;
                } else if (i == CODELEN_ZEROS) {
                        len += 3 * freq;
                } else if (i == CODELEN_ZEROS2) {
                        len += 7 * freq;
                }
        }

        return len + huffman_block_body_len(s, litlen_enc->lengths,
                                            dist_enc->lengths);
}

static uint8_t distance2dist(size_t distance)
{
        assert(distance >= 1 && distance <= 32768);

        return (distance <= 256) ? distance2dist_lo[(distance - 1)]
                                 : distance2dist_hi[(distance - 1) >> 7];
}

static bool write_huffman_block(deflate_state_t *s,
                                const huffman_encoder_t *litlen_enc,
                                const huffman_encoder_t *dist_enc)
{
        size_t i, nbits;
        uint16_t distance, dist, len, litlen;
        uint64_t bits, ebits;

        for (i = 0; i < s->block_len; i++) {
                if (s->block[i].distance == 0) {
                        /* Literal or EOB. */
                        litlen = s->block[i].u.lit;
                        assert(litlen <= LITLEN_EOB);
                        if (!ostream_write(&s->os,
                                           litlen_enc->codewords[litlen],
                                           litlen_enc->lengths[litlen])) {
                                return false;
                        }
                        continue;
                }

                /* Back reference length. */
                len = s->block[i].u.len;
                litlen = len2litlen[len];

                /* litlen bits */
                bits = litlen_enc->codewords[litlen];
                nbits = litlen_enc->lengths[litlen];

                /* ebits */
                ebits = len - litlen_tbl[litlen - LITLEN_TBL_OFFSET].base_len;
                bits |= ebits << nbits;
                nbits += litlen_tbl[litlen - LITLEN_TBL_OFFSET].ebits;

                /* Back reference distance. */
                distance = s->block[i].distance;
                dist = distance2dist(distance);

                /* dist bits */
                bits |= (uint64_t)dist_enc->codewords[dist] << nbits;
                nbits += dist_enc->lengths[dist];

                /* ebits */
                ebits = distance - dist_tbl[dist].base_dist;
                bits |= ebits << nbits;
                nbits += dist_tbl[dist].ebits;

                if (!ostream_write(&s->os, bits, nbits)) {
                        return false;
                }
        }

        return true;
}

static bool write_dynamic_block(deflate_state_t *s, bool final,
                                size_t num_litlen_lens, size_t num_dist_lens,
                                size_t num_codelen_lens,
                                const huffman_encoder_t *codelen_enc,
                                const codelen_sym_t *encoded_lens,
                                size_t num_encoded_lens,
                                const huffman_encoder_t *litlen_enc,
                                const huffman_encoder_t *dist_enc)
{
        size_t i;
        uint8_t codelen, sym;
        size_t nbits;
        uint64_t bits, hlit, hdist, hclen, count;

        /* Block header. */
        bits = (0x2 << 1) | final;
        nbits = 3;

        /* hlit (5 bits) */
        hlit = num_litlen_lens - MIN_LITLEN_LENS;
        bits |= hlit << nbits;
        nbits += 5;

        /* hdist (5 bits) */
        hdist = num_dist_lens - MIN_DIST_LENS;
        bits |= hdist << nbits;
        nbits += 5;

        /* hclen (4 bits) */
        hclen = num_codelen_lens - MIN_CODELEN_LENS;
        bits |= hclen << nbits;
        nbits += 4;

        if (!ostream_write(&s->os, bits, nbits)) {
                return false;
        }

        /* Codelen lengths. */
        for (i = 0; i < num_codelen_lens; i++) {
                codelen = codelen_enc->lengths[codelen_lengths_order[i]];
                if (!ostream_write(&s->os, codelen, 3)) {
                        return false;
                }
        }

        /* Litlen and dist code lengths. */
        for (i = 0; i < num_encoded_lens; i++) {
                sym = encoded_lens[i].sym;

                bits = codelen_enc->codewords[sym];
                nbits = codelen_enc->lengths[sym];

                count = encoded_lens[i].count;
                if (sym == CODELEN_COPY) { /* 2 ebits */
                        bits |= (count - CODELEN_COPY_MIN) << nbits;
                        nbits += 2;
                } else if (sym == CODELEN_ZEROS) { /* 3 ebits */
                        bits |= (count - CODELEN_ZEROS_MIN) << nbits;
                        nbits += 3;
                } else if (sym == CODELEN_ZEROS2) { /* 7 ebits */
                        bits |= (count - CODELEN_ZEROS2_MIN) << nbits;
                        nbits += 7;
                }

                if (!ostream_write(&s->os, bits, nbits)) {
                        return false;
                }
        }

        return write_huffman_block(s, litlen_enc, dist_enc);
}

static bool write_static_block(deflate_state_t *s, bool final)
{
        huffman_encoder_t litlen_enc, dist_enc;

        /* Write the block header. */
        if (!ostream_write(&s->os, (0x1 << 1) | final, 3)) {
                return false;
        }

        huffman_encoder_init2(&litlen_enc, fixed_litlen_lengths,
                              sizeof(fixed_litlen_lengths) /
                              sizeof(fixed_litlen_lengths[0]));
        huffman_encoder_init2(&dist_enc, fixed_dist_lengths,
                              sizeof(fixed_dist_lengths) /
                              sizeof(fixed_dist_lengths[0]));

        return write_huffman_block(s, &litlen_enc, &dist_enc);
}

static bool write_uncomp_block(deflate_state_t *s, bool final)
{
        uint8_t len_nlen[4];

        /* Write the block header. */
        if (!ostream_write(&s->os, (0x0 << 1) | final, 3)) {
                return false;
        }

        len_nlen[0] = (uint8_t)(s->block_len_bytes >> 0);
        len_nlen[1] = (uint8_t)(s->block_len_bytes >> 8);
        len_nlen[2] = ~len_nlen[0];
        len_nlen[3] = ~len_nlen[1];

        if (!ostream_write_bytes_aligned(&s->os, len_nlen, sizeof(len_nlen))) {
                return false;
        }

        if (!ostream_write_bytes_aligned(&s->os, s->block_src,
                                         s->block_len_bytes)) {
                return false;
        }

        return true;
}

/* Write the current deflate block, marking it final if that parameter is true,
   returning false if there is not enough room in the output stream. */
static bool write_block(deflate_state_t *s, bool final)
{
        size_t old_bit_pos, uncomp_len, static_len, dynamic_len;
        huffman_encoder_t dyn_litlen_enc, dyn_dist_enc, codelen_enc;
        size_t num_encoded_lens, num_litlen_lens, num_dist_lens;
        codelen_sym_t encoded_lens[LITLEN_MAX + 1 + DISTSYM_MAX + 1];
        uint16_t codelen_freqs[MAX_CODELEN_LENS] = {0};
        size_t num_codelen_lens;
        size_t i;

        old_bit_pos = ostream_bit_pos(&s->os);

        /* Add the end-of-block marker in case we write a Huffman block. */
        assert(s->block_len < sizeof(s->block) / sizeof(s->block[0]));
        assert(s->litlen_freqs[LITLEN_EOB] == 0);
        s->block[s->block_len  ].distance = 0;
        s->block[s->block_len++].u.lit = LITLEN_EOB;
        s->litlen_freqs[LITLEN_EOB] = 1;


        uncomp_len = uncomp_block_len(s);

        static_len = 3 + huffman_block_body_len(s, fixed_litlen_lengths,
                                                fixed_dist_lengths);


        /* Compute "dynamic" Huffman codes. */
        huffman_encoder_init(&dyn_litlen_enc, s->litlen_freqs,
                             LITLEN_MAX + 1, 15);
        huffman_encoder_init(&dyn_dist_enc, s->dist_freqs, DISTSYM_MAX + 1, 15);
        tweak_dist_encoder(&dyn_dist_enc);

        /* Encode the litlen and dist code lengths. */
        num_encoded_lens = encode_dist_litlen_lens(dyn_litlen_enc.lengths,
                                                   dyn_dist_enc.lengths,
                                                   encoded_lens,
                                                   &num_litlen_lens,
                                                   &num_dist_lens);

        /* Compute the codelen code. */
        for (i = 0; i < num_encoded_lens; i++) {
                codelen_freqs[encoded_lens[i].sym]++;
        }
        huffman_encoder_init(&codelen_enc, codelen_freqs, MAX_CODELEN_LENS, 7);
        num_codelen_lens = count_codelen_lens(codelen_enc.lengths);

        dynamic_len = dyn_block_len(s, num_codelen_lens, codelen_freqs,
                                    &codelen_enc, &dyn_litlen_enc,
                                    &dyn_dist_enc);


        if (uncomp_len <= dynamic_len && uncomp_len <= static_len) {
                if (!write_uncomp_block(s, final)) {
                        return false;
                }
                assert(ostream_bit_pos(&s->os) - old_bit_pos == uncomp_len);
        } else if (static_len <= dynamic_len) {
                if (!write_static_block(s, final)) {
                        return false;
                }
                assert(ostream_bit_pos(&s->os) - old_bit_pos == static_len);
        } else {
                if (!write_dynamic_block(s, final, num_litlen_lens,
                                         num_dist_lens, num_codelen_lens,
                                         &codelen_enc, encoded_lens,
                                         num_encoded_lens, &dyn_litlen_enc,
                                         &dyn_dist_enc)) {
                        return false;
                }
                assert(ostream_bit_pos(&s->os) - old_bit_pos == dynamic_len);
        }

        return true;
}

static bool lit_callback(uint8_t lit, void *aux)
{
        deflate_state_t *s = aux;

        if (s->block_len_bytes + 1 > MAX_BLOCK_LEN_BYTES) {
                if (!write_block(s, false)) {
                        return false;
                }
                s->block_src += s->block_len_bytes;
                reset_block(s);
        }

        assert(s->block_len < sizeof(s->block) / sizeof(s->block[0]));
        s->block[s->block_len  ].distance = 0;
        s->block[s->block_len++].u.lit = lit;
        s->block_len_bytes++;

        s->litlen_freqs[lit]++;

        return true;
}

static bool backref_callback(size_t dist, size_t len, void *aux)
{
        deflate_state_t *s = aux;

        if (s->block_len_bytes + len > MAX_BLOCK_LEN_BYTES) {
                if (!write_block(s, false)) {
                        return false;
                }
                s->block_src += s->block_len_bytes;
                reset_block(s);
        }

        assert(s->block_len < sizeof(s->block) / sizeof(s->block[0]));
        s->block[s->block_len  ].distance = (uint16_t)dist;
        s->block[s->block_len++].u.len = (uint16_t)len;
        s->block_len_bytes += len;

        assert(len >= MIN_LEN && len <= MAX_LEN);
        assert(dist >= MIN_DISTANCE && dist <= MAX_DISTANCE);
        s->litlen_freqs[len2litlen[len]]++;
        s->dist_freqs[distance2dist(dist)]++;

        return true;
}

bool hwdeflate(const uint8_t *src, size_t src_len, uint8_t *dst,
               size_t dst_cap, size_t *dst_used)
{
        deflate_state_t s;

        ostream_init(&s.os, dst, dst_cap);
        reset_block(&s);
        s.block_src = src;

        if (!lz77_compress(src, src_len, &lit_callback,
                           &backref_callback, &s)) {
                return false;
        }

        if (!write_block(&s, true)) {
                return false;
        }

        /* The end of the final block should match the end of src. */
        assert(s.block_src + s.block_len_bytes == src + src_len);

        *dst_used = ostream_bytes_written(&s.os);

        return true;
}
