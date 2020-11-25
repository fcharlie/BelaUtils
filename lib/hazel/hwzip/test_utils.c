/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "test_utils.h"

#include <stdio.h>
#include <stdlib.h>

void check_failed(const char *file, unsigned line, const char *cond)
{
        fprintf(stderr, "\n%s:%u: CHECK(%s) failed.\n", file, line, cond);
        exit(1);
}

uint32_t next_test_rand(uint32_t x)
{
	/* Pseudo-random number generator, using the linear congruential
	   method (see Knuth, TAOCP Vol. 2) with some random constants
	   from the interwebs. */

	static const uint32_t a = 196314165;
	static const uint32_t c = 907633515;

	return a * x + c;
}
