/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "bits.h"
#include "test_utils.h"
#include <string.h>

void test_bits_reverse16(void)
{
        CHECK(reverse16(0x0000, 1) == 0x0);
        CHECK(reverse16(0xffff, 1) == 0x1);

        CHECK(reverse16(0x0000, 16) == 0x0000);
        CHECK(reverse16(0xffff, 16) == 0xffff);

        /* 0001 0010 0011 0100 -> 0010 1100 0100 1000 */
        CHECK(reverse16(0x1234, 16) == 0x2c48);

        /* 111 1111 0100 0001 -> 100 0001 0111 1111 */
        CHECK(reverse16(0x7f41, 15) == 0x417f);
}

void test_bits_read64le(void)
{
        static const uint8_t data[8] = { 0x88, 0x99, 0xaa, 0xbb,
                                         0xcc, 0xdd, 0xee, 0xff };

        CHECK(read64le(data) == 0xffeeddccbbaa9988);
}

void test_bits_read32le(void)
{
        static const uint8_t data[4] = { 0xcc, 0xdd, 0xee, 0xff };

        CHECK(read32le(data) == 0xffeeddcc);
}

void test_bits_read16le(void)
{
        static const uint8_t data[2] = { 0xee, 0xff };

        CHECK(read16le(data) == 0xffee);
}

void test_bits_write64le(void)
{
        static const uint8_t expected[8] = { 0x11, 0x22, 0x33, 0x44,
                                             0x55, 0x66, 0x77, 0x88 };
        uint8_t buf[8];

        write64le(buf, 0x8877665544332211);
        CHECK(memcmp(buf, expected, sizeof(expected)) == 0);
}

void test_bits_write32le(void)
{
        static const uint8_t expected[4] = { 0x11, 0x22, 0x33, 0x44 };
        uint8_t buf[4];

        write32le(buf, 0x44332211);
        CHECK(memcmp(buf, expected, sizeof(expected)) == 0);
}

void test_bits_write16le(void)
{
        static const uint8_t expected[2] = { 0x11, 0x22 };
        uint8_t buf[2];

        write16le(buf, 0x2211);
        CHECK(memcmp(buf, expected, sizeof(expected)) == 0);
}

void test_bits_lsb(void)
{
        CHECK(lsb(0x1122334455667788, 0) == 0x0);
        CHECK(lsb(0x1122334455667788, 5) == 0x8);
        CHECK(lsb(0x7722334455667788, 63) == 0x7722334455667788);
}

void test_bits_round_up(void)
{
        CHECK(round_up(0, 4) == 0);
        CHECK(round_up(1, 4) == 4);
        CHECK(round_up(2, 4) == 4);
        CHECK(round_up(3, 4) == 4);
        CHECK(round_up(4, 4) == 4);
        CHECK(round_up(5, 4) == 8);
}
