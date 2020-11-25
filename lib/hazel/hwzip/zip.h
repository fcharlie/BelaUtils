/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef ZIP_H
#define ZIP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef size_t zipiter_t; /* Zip archive member iterator. */

typedef struct zip_t zip_t;
struct zip_t {
        uint16_t num_members;    /* Number of members. */
        const uint8_t *comment;  /* Zip file comment (not terminated). */
        uint16_t comment_len;    /* Zip file comment length. */
        zipiter_t members_begin; /* Iterator to the first member. */
        zipiter_t members_end;   /* Iterator to the end of members. */

        const uint8_t *src;
        size_t src_len;
};

typedef enum { ZIP_STORED = 0, ZIP_DEFLATED = 8 } method_t;

typedef struct zipmemb_t zipmemb_t;
struct zipmemb_t {
        const uint8_t *name;      /* Member name (not null terminated). */
        uint16_t name_len;        /* Member name length. */
        time_t mtime;             /* Modification time. */
        uint32_t comp_size;       /* Compressed size. */
        const uint8_t *comp_data; /* Compressed data. */
        method_t method;          /* Compression method. */
        uint32_t uncomp_size;     /* Uncompressed size. */
        uint32_t crc32;           /* CRC-32 checksum. */
        const uint8_t *comment;   /* Comment (not null terminated). */
        uint16_t comment_len;     /* Comment length. */
        bool is_dir;              /* Whether this is a directory. */
        zipiter_t next;           /* Iterator to the next member. */
};

/* Initialize zip based on the source data. Returns true on success, or false
   if the data could not be parsed as a valid Zip file. */
bool zip_read(zip_t *zip, const uint8_t *src, size_t src_len);

/* Get the Zip archive member through iterator it. */
zipmemb_t zip_member(const zip_t *zip, zipiter_t it);


/* Write a Zip file containing num_memb members into dst, which must be large
   enough to hold the resulting data. Returns the number of bytes written, which
   is guaranteed to be less than or equal to the result of zip_max_size() when
   called with the corresponding arguments. comment shall be a null-terminated
   string or null. callback shall be null or point to a function which will
   get called after the compression of each member. */
uint32_t zip_write(uint8_t *dst, uint16_t num_memb,
                   const char *const *filenames,
                   const uint8_t *const *file_data,
                   const uint32_t *file_sizes,
                   const time_t *mtimes,
                   const char *comment,
                   void (*callback)(const char *filename, uint32_t size,
                                    uint32_t comp_size));

/* Compute an upper bound on the dst size required by zip_write() for an
 * archive with num_memb members with certain filenames, sizes, and archive
 * comment. Returns zero on error, e.g. if a filename is longer than 2^16-1, or
 * if the total file size is larger than 2^32-1. */
uint32_t zip_max_size(uint16_t num_memb, const char *const *filenames,
                      const uint32_t *file_sizes, const char *comment);

#endif
