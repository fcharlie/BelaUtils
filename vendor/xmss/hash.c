#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>

#include "hash_address.h"
#include "utils.h"
#include "params.h"
#include "hash.h"
#include "fips202.h"

#define XMSS_HASH_PADDING_F 0
#define XMSS_HASH_PADDING_H 1
#define XMSS_HASH_PADDING_HASH 2
#define XMSS_HASH_PADDING_PRF 3

void addr_to_bytes(unsigned char *bytes, const uint32_t addr[8])
{
    int i;
    for (i = 0; i < 8; i++) {
        ull_to_bytes(bytes + i*4, 4, addr[i]);
    }
}

static int core_hash(const xmss_params *params,
                     unsigned char *out,
                     const unsigned char *in, unsigned long long inlen)
{
    if (params->n == 32 && params->func == XMSS_SHA2) {
        SHA256(in, inlen, out);
    }
    else if (params->n == 32 && params->func == XMSS_SHAKE) {
        shake128(out, 32, in, inlen);
    }
    else if (params->n == 64 && params->func == XMSS_SHA2) {
        SHA512(in, inlen, out);
    }
    else if (params->n == 64 && params->func == XMSS_SHAKE) {
        shake256(out, 64, in, inlen);
    }
    else {
        return -1;
    }
    return 0;
}

/*
 * Computes PRF(key, in), for a key of params->n bytes, and a 32-byte input.
 */
int prf(const xmss_params *params,
        unsigned char *out, const unsigned char in[32],
        const unsigned char *key)
{
    unsigned char buf[2*params->n + 32];

    ull_to_bytes(buf, params->n, XMSS_HASH_PADDING_PRF);
    memcpy(buf + params->n, key, params->n);
    memcpy(buf + 2*params->n, in, 32);

    return core_hash(params, out, buf, 2*params->n + 32);
}

/*
 * Computes the message hash using R, the public root, the index of the leaf
 * node, and the message. Notably, it requires m_with_prefix to have 4*n bytes
 * of space before the message, to use for the prefix. This is necessary to
 * prevent having to move the message around (and thus allocate memory for it).
 */
int hash_message(const xmss_params *params, unsigned char *out,
                 const unsigned char *R, const unsigned char *root,
                 unsigned long long idx,
                 unsigned char *m_with_prefix, unsigned long long mlen)
{
    /* We're creating a hash using input of the form:
       toByte(X, 32) || R || root || index || M */
    ull_to_bytes(m_with_prefix, params->n, XMSS_HASH_PADDING_HASH);
    memcpy(m_with_prefix + params->n, R, params->n);
    memcpy(m_with_prefix + 2*params->n, root, params->n);
    ull_to_bytes(m_with_prefix + 3*params->n, params->n, idx);

    return core_hash(params, out, m_with_prefix, mlen + 4*params->n);
}

/**
 * We assume the left half is in in[0]...in[n-1]
 */
int thash_h(const xmss_params *params,
            unsigned char *out, const unsigned char *in,
            const unsigned char *pub_seed, uint32_t addr[8])
{
    unsigned char buf[4 * params->n];
    unsigned char bitmask[2 * params->n];
    unsigned char addr_as_bytes[32];
    unsigned int i;

    /* Set the function padding. */
    ull_to_bytes(buf, params->n, XMSS_HASH_PADDING_H);

    /* Generate the n-byte key. */
    set_key_and_mask(addr, 0);
    addr_to_bytes(addr_as_bytes, addr);
    prf(params, buf + params->n, addr_as_bytes, pub_seed);

    /* Generate the 2n-byte mask. */
    set_key_and_mask(addr, 1);
    addr_to_bytes(addr_as_bytes, addr);
    prf(params, bitmask, addr_as_bytes, pub_seed);

    set_key_and_mask(addr, 2);
    addr_to_bytes(addr_as_bytes, addr);
    prf(params, bitmask + params->n, addr_as_bytes, pub_seed);

    for (i = 0; i < 2 * params->n; i++) {
        buf[2*params->n + i] = in[i] ^ bitmask[i];
    }
    return core_hash(params, out, buf, 4 * params->n);
}

int thash_f(const xmss_params *params,
            unsigned char *out, const unsigned char *in,
            const unsigned char *pub_seed, uint32_t addr[8])
{
    unsigned char buf[3 * params->n];
    unsigned char bitmask[params->n];
    unsigned char addr_as_bytes[32];
    unsigned int i;

    /* Set the function padding. */
    ull_to_bytes(buf, params->n, XMSS_HASH_PADDING_F);

    /* Generate the n-byte key. */
    set_key_and_mask(addr, 0);
    addr_to_bytes(addr_as_bytes, addr);
    prf(params, buf + params->n, addr_as_bytes, pub_seed);

    /* Generate the n-byte mask. */
    set_key_and_mask(addr, 1);
    addr_to_bytes(addr_as_bytes, addr);
    prf(params, bitmask, addr_as_bytes, pub_seed);

    for (i = 0; i < params->n; i++) {
        buf[2*params->n + i] = in[i] ^ bitmask[i];
    }
    return core_hash(params, out, buf, 3 * params->n);
}
