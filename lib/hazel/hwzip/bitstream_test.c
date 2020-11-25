/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "bitstream.h"

#include "bits.h"
#include "test_utils.h"

void test_istream_basic(void)
{
        istream_t is;
        const uint8_t bits = 0x47; /* 0100 0111 */
        const uint8_t arr[9] = { 0x45, 0x48 }; /* 01000101 01001000 */

        istream_init(&is, &bits, 1);
        CHECK(lsb(istream_bits(&is), 1) == 1); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 1); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 1); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 0); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 0); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 0); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 1); CHECK(istream_advance(&is, 1));
        CHECK(lsb(istream_bits(&is), 1) == 0); CHECK(istream_advance(&is, 1));
        CHECK(!istream_advance(&is, 1));

        istream_init(&is, arr, 9);
        CHECK(lsb(istream_bits(&is), 3) == 0x5); CHECK(istream_advance(&is, 3));
        CHECK(istream_byte_align(&is) == &arr[1]);
        CHECK(lsb(istream_bits(&is), 4) == 0x8); CHECK(istream_advance(&is, 4));
        CHECK(istream_byte_align(&is) == &arr[2]);
}

void test_ostream_basic(void)
{
        ostream_t os;
        uint8_t byte;
        uint8_t arr[10];

        ostream_init(&os, &byte, 1);

        /* Write 1, 0, 1, 1011, 1 */
        CHECK(ostream_write(&os, 0x1, 1));
        CHECK(ostream_write(&os, 0x0, 1));
        CHECK(ostream_write(&os, 0x1, 1));
        CHECK(ostream_write(&os, 0xB, 4));
        CHECK(ostream_write(&os, 0x1, 1));

        CHECK(ostream_bytes_written(&os) == 1);
        CHECK(byte == 0xDD); /* 1101 1101 */

        /* Try to write some more. Not enough room. */
        CHECK(!ostream_write(&os, 0x7, 3));


        ostream_init(&os, arr, 10);

        /* Write 60 bits so the first word is almost full. */
        CHECK(ostream_write(&os, 0x3ff, 10));
        CHECK(ostream_write(&os, 0x3ff, 10));
        CHECK(ostream_write(&os, 0x3ff, 10));
        CHECK(ostream_write(&os, 0x3ff, 10));
        CHECK(ostream_write(&os, 0x3ff, 10));
        CHECK(ostream_write(&os, 0x3ff, 10));

        /* Write another 8 bits. */
        CHECK(ostream_write(&os, 0x12, 8));

        CHECK(arr[0] = 0xff);
        CHECK(arr[1] = 0xff);
        CHECK(arr[2] = 0xff);
        CHECK(arr[3] = 0xff);
        CHECK(arr[4] = 0xff);
        CHECK(arr[5] = 0xff);
        CHECK(arr[6] = 0xff);
        CHECK(arr[7] = 0x2f);
        CHECK(arr[8] = 0x01);


        /* Writing 0 bits works and doesn't do anything. */
        ostream_init(&os, &byte, 1);
        CHECK(ostream_write(&os, 0x1, 1));
        CHECK(ostream_write(&os, 0x0, 0));
        CHECK(ostream_write(&os, 0x0, 0));
        CHECK(ostream_write(&os, 0x0, 0));
        CHECK(ostream_write(&os, 0x1, 1));
        CHECK(byte == 0x3);

        /* Try writing too much. */
        ostream_init(&os, arr, 10);
        CHECK(ostream_write(&os, 0x1234, 16));
        CHECK(ostream_write(&os, 0x1234, 16));
        CHECK(ostream_write(&os, 0x1234, 16));
        CHECK(ostream_write(&os, 0x1234, 16));
        CHECK(ostream_write(&os, 0x1234, 16));
        CHECK(!ostream_write(&os, 0x1, 1));

        /* Try writing too much. */
        ostream_init(&os, &byte, 1);
        CHECK(!ostream_write(&os, 0x1234, 16));

}

void test_ostream_write_bytes_aligned(void)
{
        ostream_t os;
        uint8_t arr[10];

        ostream_init(&os, arr, 10);

        /* Write a few bits. */
        CHECK(ostream_write(&os, 0x7, 3));
        CHECK(ostream_bit_pos(&os) == 3);

        /* Write some bytes aligned. */
        CHECK(ostream_write_bytes_aligned(&os, (const uint8_t*)"foo", 3));
        CHECK(ostream_bit_pos(&os) == 32);
        CHECK(arr[0] == 0x7);
        CHECK(arr[1] == 'f');
        CHECK(arr[2] == 'o');
        CHECK(arr[3] == 'o');

        /* Not enough room. */
        ostream_init(&os, arr, 1);
        CHECK(ostream_write(&os, 0x1, 1));
        CHECK(!ostream_write_bytes_aligned(&os, (const uint8_t*)"foo", 1));
}
