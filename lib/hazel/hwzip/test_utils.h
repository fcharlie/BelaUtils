/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>

#define CHECK(cond) \
        do { \
                if (!(cond)) { check_failed(__FILE__, __LINE__, #cond); } \
        } while (0)

void check_failed(const char *file, unsigned line, const char *cond);

uint32_t next_test_rand(uint32_t x);

#endif
