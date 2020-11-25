/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "zip.h"

#include <string.h>
#include "bits.h"
#include "crc32.h"
#include "deflate.h"


/* Read 16/32 bits little-endian and bump p forward afterwards. */
#define READ16(p) ((p) += 2, read16le((p) - 2))
#define READ32(p) ((p) += 4, read32le((p) - 4))

/* Write 16/32 bits little-endian and bump p forward afterwards. */
#define WRITE16(p, x) (write16le((p), (x)), (p) += 2)
#define WRITE32(p, x) (write32le((p), (x)), (p) += 4)


/* End of Central Directory Record. */
struct eocdr {
        uint16_t disk_nbr;        /* Number of this disk. */
        uint16_t cd_start_disk;   /* Nbr. of disk with start of the CD. */
        uint16_t disk_cd_entries; /* Nbr. of CD entries on this disk. */
        uint16_t cd_entries;      /* Nbr. of Central Directory entries. */
        uint32_t cd_size;         /* Central Directory size in bytes. */
        uint32_t cd_offset;       /* Central Directory file offset. */
        uint16_t comment_len;     /* Archive comment length. */
        const uint8_t *comment;   /* Archive comment. */
};

/* Size of the End of Central Directory Record, not including comment. */
#define EOCDR_BASE_SZ 22
#define EOCDR_SIGNATURE 0x06054b50  /* "PK\5\6" little-endian. */

static bool find_eocdr(struct eocdr *r, const uint8_t *src, size_t src_len)
{
        size_t comment_len;
        const uint8_t *p;
        uint32_t signature;

        for (comment_len = 0; comment_len <= UINT16_MAX; comment_len++) {
                if (src_len < EOCDR_BASE_SZ + comment_len) {
                        break;
                }

                p = &src[src_len - EOCDR_BASE_SZ - comment_len];
                signature = READ32(p);

                if (signature == EOCDR_SIGNATURE) {
                        r->disk_nbr = READ16(p);
                        r->cd_start_disk = READ16(p);
                        r->disk_cd_entries = READ16(p);
                        r->cd_entries = READ16(p);
                        r->cd_size = READ32(p);
                        r->cd_offset = READ32(p);
                        r->comment_len = READ16(p);
                        r->comment = p;
                        assert(p == &src[src_len - comment_len] &&
                               "All fields read.");

                        if (r->comment_len == comment_len) {
                                return true;
                        }
                }
        }

        return false;
}

static size_t write_eocdr(uint8_t *dst, const struct eocdr *r)
{
        uint8_t *p = dst;

        WRITE32(p, EOCDR_SIGNATURE);
        WRITE16(p, r->disk_nbr);
        WRITE16(p, r->cd_start_disk);
        WRITE16(p, r->disk_cd_entries);
        WRITE16(p, r->cd_entries);
        WRITE32(p, r->cd_size);
        WRITE32(p, r->cd_offset);
        WRITE16(p, r->comment_len);
        assert(p - dst == EOCDR_BASE_SZ);

        if (r->comment_len != 0) {
                memcpy(p, r->comment, r->comment_len);
                p += r->comment_len;
        }

        return (size_t)(p - dst);
}

#define EXT_ATTR_DIR (1U << 4)
#define EXT_ATTR_ARC (1U << 5)

/* Central File Header (Central Directory Entry) */
struct cfh {
        uint16_t made_by_ver;    /* Version made by. */
        uint16_t extract_ver;    /* Version needed to extract. */
        uint16_t gp_flag;        /* General purpose bit flag. */
        uint16_t method;         /* Compression method. */
        uint16_t mod_time;       /* Modification time. */
        uint16_t mod_date;       /* Modification date. */
        uint32_t crc32;          /* CRC-32 checksum. */
        uint32_t comp_size;      /* Compressed size. */
        uint32_t uncomp_size;    /* Uncompressed size. */
        uint16_t name_len;       /* Filename length. */
        uint16_t extra_len;      /* Extra data length. */
        uint16_t comment_len;    /* Comment length. */
        uint16_t disk_nbr_start; /* Disk nbr. where file begins. */
        uint16_t int_attrs;      /* Internal file attributes. */
        uint32_t ext_attrs;      /* External file attributes. */
        uint32_t lfh_offset;     /* Local File Header offset. */
        const uint8_t *name;     /* Filename. */
        const uint8_t *extra;    /* Extra data. */
        const uint8_t *comment;  /* File comment. */
};

/* Size of a Central File Header, not including name, extra, and comment. */
#define CFH_BASE_SZ 46
#define CFH_SIGNATURE 0x02014b50 /* "PK\1\2" little-endian. */

static bool read_cfh(struct cfh *cfh, const uint8_t *src, size_t src_len,
                     size_t offset)
{
        const uint8_t *p;
        uint32_t signature;

        if (offset > src_len || src_len - offset < CFH_BASE_SZ) {
                return false;
        }

        p = &src[offset];
        signature = READ32(p);
        if (signature != CFH_SIGNATURE) {
                return false;
        }

        cfh->made_by_ver = READ16(p);
        cfh->extract_ver = READ16(p);
        cfh->gp_flag = READ16(p);
        cfh->method = READ16(p);
        cfh->mod_time = READ16(p);
        cfh->mod_date = READ16(p);
        cfh->crc32 = READ32(p);
        cfh->comp_size = READ32(p);
        cfh->uncomp_size = READ32(p);
        cfh->name_len = READ16(p);
        cfh->extra_len = READ16(p);
        cfh->comment_len = READ16(p);
        cfh->disk_nbr_start = READ16(p);
        cfh->int_attrs = READ16(p);
        cfh->ext_attrs = READ32(p);
        cfh->lfh_offset = READ32(p);
        cfh->name = p;
        cfh->extra = cfh->name + cfh->name_len;
        cfh->comment = cfh->extra + cfh->extra_len;
        assert(p == &src[offset + CFH_BASE_SZ] && "All fields read.");

        if (src_len - offset - CFH_BASE_SZ <
            cfh->name_len + cfh->extra_len + cfh->comment_len) {
                return false;
        }

        return true;
}

static size_t write_cfh(uint8_t *dst, const struct cfh *cfh)
{
        uint8_t *p = dst;

        WRITE32(p, CFH_SIGNATURE);
        WRITE16(p, cfh->made_by_ver);
        WRITE16(p, cfh->extract_ver);
        WRITE16(p, cfh->gp_flag);
        WRITE16(p, cfh->method);
        WRITE16(p, cfh->mod_time);
        WRITE16(p, cfh->mod_date);
        WRITE32(p, cfh->crc32);
        WRITE32(p, cfh->comp_size);
        WRITE32(p, cfh->uncomp_size);
        WRITE16(p, cfh->name_len);
        WRITE16(p, cfh->extra_len);
        WRITE16(p, cfh->comment_len);
        WRITE16(p, cfh->disk_nbr_start);
        WRITE16(p, cfh->int_attrs);
        WRITE32(p, cfh->ext_attrs);
        WRITE32(p, cfh->lfh_offset);
        assert(p - dst == CFH_BASE_SZ);

        if (cfh->name_len != 0) {
                memcpy(p, cfh->name, cfh->name_len);
                p += cfh->name_len;
        }

        if (cfh->extra_len != 0) {
                memcpy(p, cfh->extra, cfh->extra_len);
                p += cfh->extra_len;
        }

        if (cfh->comment_len != 0) {
                memcpy(p, cfh->comment, cfh->comment_len);
                p += cfh->comment_len;
        }

        return (size_t)(p - dst);
}


/* Local File Header. */
struct lfh {
        uint16_t extract_ver;
        uint16_t gp_flag;
        uint16_t method;
        uint16_t mod_time;
        uint16_t mod_date;
        uint32_t crc32;
        uint32_t comp_size;
        uint32_t uncomp_size;
        uint16_t name_len;
        uint16_t extra_len;
        const uint8_t *name;
        const uint8_t *extra;
};

/* Size of a Local File Header, not including name and extra. */
#define LFH_BASE_SZ 30
#define LFH_SIGNATURE 0x04034b50 /* "PK\3\4" little-endian. */

static bool read_lfh(struct lfh *lfh, const uint8_t *src, size_t src_len,
                     size_t offset)
{
        const uint8_t *p;
        uint32_t signature;

        if (offset > src_len || src_len - offset < LFH_BASE_SZ) {
                return false;
        }

        p = &src[offset];
        signature = READ32(p);
        if (signature != LFH_SIGNATURE) {
                return false;
        }

        lfh->extract_ver = READ16(p);
        lfh->gp_flag = READ16(p);
        lfh->method = READ16(p);
        lfh->mod_time = READ16(p);
        lfh->mod_date = READ16(p);
        lfh->crc32 = READ32(p);
        lfh->comp_size = READ32(p);
        lfh->uncomp_size = READ32(p);
        lfh->name_len = READ16(p);
        lfh->extra_len = READ16(p);
        lfh->name = p;
        lfh->extra = lfh->name + lfh->name_len;
        assert(p == &src[offset + LFH_BASE_SZ] && "All fields read.");

        if (src_len - offset - LFH_BASE_SZ < lfh->name_len + lfh->extra_len) {
                return false;
        }

        return true;
}

static size_t write_lfh(uint8_t *dst, const struct lfh *lfh)
{
        uint8_t *p = dst;

        WRITE32(p, LFH_SIGNATURE);
        WRITE16(p, lfh->extract_ver);
        WRITE16(p, lfh->gp_flag);
        WRITE16(p, lfh->method);
        WRITE16(p, lfh->mod_time);
        WRITE16(p, lfh->mod_date);
        WRITE32(p, lfh->crc32);
        WRITE32(p, lfh->comp_size);
        WRITE32(p, lfh->uncomp_size);
        WRITE16(p, lfh->name_len);
        WRITE16(p, lfh->extra_len);
        assert(p - dst == LFH_BASE_SZ);

        if (lfh->name_len != 0) {
                memcpy(p, lfh->name, lfh->name_len);
                p += lfh->name_len;
        }

        if (lfh->extra_len != 0) {
                memcpy(p, lfh->extra, lfh->extra_len);
                p += lfh->extra_len;
        }

        return (size_t)(p - dst);
}


/* Convert DOS date and time to time_t. */
static time_t dos2ctime(uint16_t dos_date, uint16_t dos_time)
{
        struct tm tm = {0};

        tm.tm_sec = (dos_time & 0x1f) * 2;  /* Bits 0--4:  Secs divided by 2. */
        tm.tm_min = (dos_time >> 5) & 0x3f; /* Bits 5--10: Minute. */
        tm.tm_hour = (dos_time >> 11);      /* Bits 11-15: Hour (0--23). */

        tm.tm_mday = (dos_date & 0x1f);          /* Bits 0--4: Day (1--31). */
        tm.tm_mon = ((dos_date >> 5) & 0xf) - 1; /* Bits 5--8: Month (1--12). */
        tm.tm_year = (dos_date >> 9) + 80;       /* Bits 9--15: Year-1980. */

        tm.tm_isdst = -1;

        return mktime(&tm);
}

/* Convert time_t to DOS date and time. */
static void ctime2dos(time_t t, uint16_t *dos_date, uint16_t *dos_time)
{
        struct tm *tm = localtime(&t);

        *dos_time = 0;
        *dos_time |= tm->tm_sec / 2;    /* Bits 0--4:  Second divided by two. */
        *dos_time |= tm->tm_min << 5;   /* Bits 5--10: Minute. */
        *dos_time |= tm->tm_hour << 11; /* Bits 11-15: Hour. */

        *dos_date = 0;
        *dos_date |= tm->tm_mday;             /* Bits 0--4:  Day (1--31). */
        *dos_date |= (tm->tm_mon + 1) << 5;   /* Bits 5--8:  Month (1--12). */
        *dos_date |= (tm->tm_year - 80) << 9; /* Bits 9--15: Year from 1980. */
}


bool zip_read(zip_t *zip, const uint8_t *src, size_t src_len)
{
        struct eocdr eocdr;
        struct cfh cfh;
        struct lfh lfh;
        size_t i, offset;
        const uint8_t *comp_data;

        zip->src = src;
        zip->src_len = src_len;

        if (!find_eocdr(&eocdr, src, src_len)) {
                return false;
        }

        if (eocdr.disk_nbr != 0 || eocdr.cd_start_disk != 0 ||
            eocdr.disk_cd_entries != eocdr.cd_entries) {
                return false; /* Cannot handle multi-volume archives. */
        }

        zip->num_members = eocdr.cd_entries;
        zip->comment = eocdr.comment;
        zip->comment_len = eocdr.comment_len;

        offset = eocdr.cd_offset;
        zip->members_begin = offset;

        /* Read the member info and do a few checks. */
        for (i = 0; i < eocdr.cd_entries; i++) {
                if (!read_cfh(&cfh, src, src_len, offset)) {
                        return false;
                }

                if (cfh.gp_flag & 1) {
                        return false; /* The member is encrypted. */
                }
                if (cfh.method != ZIP_STORED && cfh.method != ZIP_DEFLATED) {
                        return false; /* Unsupported compression method. */
                }
                if (cfh.method == ZIP_STORED &&
                    cfh.uncomp_size != cfh.comp_size) {
                  return false;
                }
                if (cfh.disk_nbr_start != 0) {
                        return false; /* Cannot handle multi-volume archives. */
                }
                if (memchr(cfh.name, '\0', cfh.name_len) != NULL) {
                        return false; /* Bad filename. */
                }

                if (!read_lfh(&lfh, src, src_len, cfh.lfh_offset)) {
                        return false;
                }

                comp_data = lfh.extra + lfh.extra_len;
                if (cfh.comp_size > src_len - (size_t)(comp_data - src)) {
                        return false; /* Member data does not fit in src. */
                }

                offset += CFH_BASE_SZ + cfh.name_len + cfh.extra_len +
                          cfh.comment_len;
        }

        zip->members_end = offset;

        return true;
}

zipmemb_t zip_member(const zip_t *zip, zipiter_t it)
{
        struct cfh cfh;
        struct lfh lfh;
        bool ok;
        zipmemb_t m;

        assert(it >= zip->members_begin && it < zip->members_end);

        ok = read_cfh(&cfh, zip->src, zip->src_len, it);
        assert(ok);

        ok = read_lfh(&lfh, zip->src, zip->src_len, cfh.lfh_offset);
        assert(ok);

        m.name = cfh.name;
        m.name_len = cfh.name_len;
        m.mtime = dos2ctime(cfh.mod_date, cfh.mod_time);
        m.comp_size = cfh.comp_size;
        m.comp_data = lfh.extra + lfh.extra_len;
        m.method = cfh.method;
        m.uncomp_size = cfh.uncomp_size;
        m.crc32 = cfh.crc32;
        m.comment = cfh.comment;
        m.comment_len = cfh.comment_len;
        m.is_dir = (cfh.ext_attrs & EXT_ATTR_DIR) != 0;

        m.next = it + CFH_BASE_SZ +
                 cfh.name_len + cfh.extra_len + cfh.comment_len;

        assert(m.next <= zip->members_end);

        return m;
}

uint32_t zip_write(uint8_t *dst, uint16_t num_memb,
                   const char *const *filenames,
                   const uint8_t *const *file_data,
                   const uint32_t *file_sizes,
                   const time_t *mtimes,
                   const char *comment,
                   void (*callback)(const char *filename, uint32_t size,
                                    uint32_t comp_size))
{
        uint16_t i;
        uint8_t *p;
        struct eocdr eocdr;
        struct cfh cfh;
        struct lfh lfh;
        bool ok;
        uint16_t name_len;
        uint8_t *data_dst;
        size_t comp_sz;
        uint32_t lfh_offset, cd_offset, eocdr_offset;

        p = dst;

        /* Write Local File Headers and deflated or stored data. */
        for (i = 0; i < num_memb; i++) {
                assert(filenames[i] != NULL);
                assert(strlen(filenames[i]) <= UINT16_MAX);
                name_len = (uint16_t)strlen(filenames[i]);

                data_dst = p + LFH_BASE_SZ + name_len;

                if (hwdeflate(file_data[i], file_sizes[i], data_dst,
                              file_sizes[i], &comp_sz) &&
                                comp_sz < file_sizes[i]) {
                        lfh.method = ZIP_DEFLATED;
                        assert(comp_sz <= UINT32_MAX);
                        lfh.comp_size = (uint32_t)comp_sz;
                } else {
                        memcpy(data_dst, file_data[i], file_sizes[i]);
                        lfh.method = ZIP_STORED;
                        lfh.comp_size = file_sizes[i];
                }

                if (callback != NULL) {
                        callback(filenames[i], file_sizes[i], lfh.comp_size);
                }

                lfh.extract_ver = (0 << 8) | 20; /* DOS | PKZIP 2.0 */
                lfh.gp_flag = (lfh.method == ZIP_DEFLATED ? (0x1 << 1) : 0x0);
                ctime2dos(mtimes[i], &lfh.mod_date, &lfh.mod_time);
                lfh.crc32 = crc32(file_data[i], file_sizes[i]);
                lfh.uncomp_size = file_sizes[i];
                lfh.name_len = name_len;
                lfh.extra_len = 0;
                lfh.name = (const uint8_t*)filenames[i];
                p += write_lfh(p, &lfh);
                p += lfh.comp_size;
        }

        assert((size_t)(p - dst) <= UINT32_MAX);
        cd_offset = (uint32_t)(p - dst);

        /* Write the Central Directory based on the Local File Headers. */
        lfh_offset = 0;
        for (i = 0; i < num_memb; i++) {
                ok = read_lfh(&lfh, dst, SIZE_MAX, lfh_offset);
                assert(ok);

                cfh.made_by_ver = lfh.extract_ver;
                cfh.extract_ver = lfh.extract_ver;
                cfh.gp_flag = lfh.gp_flag;
                cfh.method = lfh.method;
                cfh.mod_time = lfh.mod_time;
                cfh.mod_date = lfh.mod_date;
                cfh.crc32 = lfh.crc32;
                cfh.comp_size = lfh.comp_size;
                cfh.uncomp_size = lfh.uncomp_size;
                cfh.name_len = lfh.name_len;
                cfh.extra_len = 0;
                cfh.comment_len = 0;
                cfh.disk_nbr_start = 0;
                cfh.int_attrs = 0;
                cfh.ext_attrs = EXT_ATTR_ARC;
                cfh.lfh_offset = lfh_offset;
                cfh.name = lfh.name;
                p += write_cfh(p, &cfh);

                lfh_offset += LFH_BASE_SZ + lfh.name_len + lfh.comp_size;
        }

        assert((size_t)(p - dst) <= UINT32_MAX);
        eocdr_offset = (uint32_t)(p - dst);

        /* Write the End of Central Directory Record. */
        eocdr.disk_nbr = 0;
        eocdr.cd_start_disk = 0;
        eocdr.disk_cd_entries = num_memb;
        eocdr.cd_entries = num_memb;
        eocdr.cd_size = eocdr_offset - cd_offset;
        eocdr.cd_offset = cd_offset;
        eocdr.comment_len = (uint16_t)(comment == NULL ? 0 : strlen(comment));
        eocdr.comment = (const uint8_t*)comment;
        p += write_eocdr(p, &eocdr);

        assert((size_t)(p - dst) <= zip_max_size(num_memb, filenames,
                                                 file_sizes, comment));

        return (uint32_t)(p - dst);
}

uint32_t zip_max_size(uint16_t num_memb, const char *const *filenames,
                      const uint32_t *file_sizes, const char *comment)
{
        size_t comment_len, name_len;
        uint64_t total;
        uint16_t i;

        comment_len = (comment == NULL ? 0 : strlen(comment));
        if (comment_len > UINT16_MAX) {
                return 0;
        }

        total = EOCDR_BASE_SZ + comment_len; /* EOCDR */

        for (i = 0; i < num_memb; i++) {
                assert(filenames[i] != NULL);
                name_len = strlen(filenames[i]);
                if (name_len > UINT16_MAX) {
                        return 0;
                }

                total += CFH_BASE_SZ + name_len; /* Central File Header */
                total += LFH_BASE_SZ + name_len; /* Local File Header */
                total += file_sizes[i];          /* Uncompressed data size. */
        }

        if (total > UINT32_MAX) {
                return 0;
        }

        return (uint32_t)total;
}
