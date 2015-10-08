#include <string.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "frandom.h"

/*
 * Fast random string generator
 */

/* We use 64 bits random number to set 8 bytes string at a time */
#define SLICE_LENGTH 8

/**
 * This two bitwise value is used to avoid four characters which can not be
 * accepted by memcached as a key: '\r', '\n', '\0', and space.
 * Usage: value & BIT_MASK | BIT_SET
 */
//#define BIT_MASK_NON_ASCII (0xd7d7d7d7d7d7d7d7)
//#define BIT_MASK_ASCII (0x5757575757575757)
//#define BIT_SET (0x101010101010101)
//#define BIT_MASK BIT_MASK_NON_ASCII

const uint64_t bit_sets[4] = { 0x3030303030303030,
                               0x4040404040404040,
                               0x5050505050505050,
                               0x6060606060606060
};
const uint64_t bit_masks[4] = { 0x3f3f3f3f3f3f3f3f,
                                0x4f4f4f4f4f4f4f4f,
                                0x5f5f5f5f5f5f5f5f,
                                0x6f6f6f6f6f6f6f6f,
};

/**
 * random string generator for less than 8 bytes
 */
#define string_gen_last(buf, seed, level, len) {                         \
    uint64_t ret = (frandom(seed, (*level)++) & BIT_MASK) | BIT_SET;     \
    memcpy((char *)buf, (char *)&ret, len);                                      \
}

/**
 * 8 byte random string generator
 */
#define string_gen_1(buf, seed, level) {                                  \
    *(uint64_t *)buf = (frandom(seed, (*level)++) & BIT_MASK) | BIT_SET;  \
}

/**
 * 24 byte random string generator
 */
#define string_gen_2(buf, seed, level) {                                \
    register uint64_t rd1 = frandom(seed, (*level)++);                  \
    register uint64_t rd2 = frandom(seed, (*level)++);                  \
    register uint64_t *string = (uint64_t *)buf;                        \
    string[0] = (rd1 & BIT_MASK) | BIT_SET;                             \
    string[1] = (rd2 & BIT_MASK) | BIT_SET;                             \
    string[2] = ((rd1 ^ rd2) & BIT_MASK) | BIT_SET;                     \
}

/**
 * 56 byte random string generator
 */
#define string_gen_3(buf, seed, level) {                            \
    register uint64_t rd1 = frandom(seed, (*level)++);              \
    register uint64_t rd2 = frandom(seed, (*level)++);              \
    register uint64_t rd3 = frandom(seed, (*level)++);              \
                                                                    \
    register uint64_t rd_12 = rd1 ^ rd2;                            \
                                                                    \
    uint64_t *string = (uint64_t *)buf;                             \
    string[0] = (rd1 & BIT_MASK) | BIT_SET;                         \
    string[1] = (rd2 & BIT_MASK) | BIT_SET;                         \
    string[2] = (rd3 & BIT_MASK) | BIT_SET;                         \
    string[3] = (rd_12 & BIT_MASK) | BIT_SET;                       \
    string[4] = ((rd1 ^ rd3) & BIT_MASK) | BIT_SET;                 \
    string[5] = ((rd2 ^ rd3) & BIT_MASK) | BIT_SET;                 \
    string[6] = ((rd_12 ^ rd3) & BIT_MASK) | BIT_SET;               \
}

/**
 * 120 byte random string generator
 */
#define string_gen_4(buf, seed, level) {                            \
    register uint64_t rd1 = frandom(seed, (*level)++);              \
    register uint64_t rd2 = frandom(seed, (*level)++);              \
    register uint64_t rd3 = frandom(seed, (*level)++);              \
    register uint64_t rd4 = frandom(seed, (*level)++);              \
                                                                    \
    register uint64_t rd_12 = rd1 ^ rd2;                            \
    register uint64_t rd_23 = rd2 ^ rd3;                            \
    register uint64_t rd_34 = rd3 ^ rd4;                            \
    register uint64_t rd_123 = rd_12 ^ rd3;                         \
                                                                    \
    uint64_t *string = (uint64_t *)buf;                             \
    string[0] = (rd1 & BIT_MASK) | BIT_SET;                         \
    string[1] = (rd2 & BIT_MASK) | BIT_SET;                         \
    string[2] = (rd3 & BIT_MASK) | BIT_SET;                         \
    string[3] = (rd4 & BIT_MASK) | BIT_SET;                         \
    string[4] = (rd_12 & BIT_MASK) | BIT_SET;                       \
    string[5] = ((rd1 ^ rd3) & BIT_MASK) | BIT_SET;                 \
    string[6] = ((rd1 ^ rd4) & BIT_MASK) | BIT_SET;                 \
    string[7] = (rd_23 & BIT_MASK) | BIT_SET;                       \
    string[8] = ((rd2 ^ rd4) & BIT_MASK) | BIT_SET;                 \
    string[9] = (rd_34 & BIT_MASK) | BIT_SET;                       \
    string[10] = (rd_123 & BIT_MASK) | BIT_SET;                     \
    string[11] = ((rd_12 ^ rd4) & BIT_MASK) | BIT_SET;              \
    string[12] = ((rd1 ^ rd_34) & BIT_MASK) | BIT_SET;              \
    string[13] = ((rd_23 ^ rd4) & BIT_MASK) | BIT_SET;              \
    string[14] = ((rd_123 ^ rd4) & BIT_MASK) | BIT_SET;             \
}

/**
 * 248 byte random string generator
 */
#define string_gen_5(buf, seed, level) {                            \
    register uint64_t rd1 = frandom(seed, (*level)++);              \
    register uint64_t rd2 = frandom(seed, (*level)++);              \
    register uint64_t rd3 = frandom(seed, (*level)++);              \
    register uint64_t rd4 = frandom(seed, (*level)++);              \
    register uint64_t rd5 = frandom(seed, (*level)++);              \
                                                                    \
    uint64_t rd_12 = rd1 ^ rd2;                                     \
    uint64_t rd_13 = rd1 ^ rd3;                                     \
    uint64_t rd_23 = rd2 ^ rd3;                                     \
    uint64_t rd_34 = rd3 ^ rd4;                                     \
    uint64_t rd_45 = rd4 ^ rd5;                                     \
    uint64_t rd_123 = rd_12 ^ rd3;                                  \
    uint64_t rd_234 = rd_23 ^ rd4;                                  \
    uint64_t rd_345 = rd_34 ^ rd5;                                  \
    uint64_t rd_1234 = rd_123 ^ rd4;                                \
                                                                    \
    uint64_t *string = (uint64_t *)buf;                             \
    string[0] = (rd1 & BIT_MASK) | BIT_SET;                         \
    string[1] = (rd2 & BIT_MASK) | BIT_SET;                         \
    string[2] = (rd3 & BIT_MASK) | BIT_SET;                         \
    string[3] = (rd4 & BIT_MASK) | BIT_SET;                         \
    string[4] = (rd5 & BIT_MASK) | BIT_SET;                         \
    string[5] = (rd_12 & BIT_MASK) | BIT_SET;                       \
    string[6] = (rd_13 & BIT_MASK) | BIT_SET;                       \
    string[7] = ((rd1 ^ rd4) & BIT_MASK) | BIT_SET;                 \
    string[8] = ((rd1 ^ rd5) & BIT_MASK) | BIT_SET;                 \
    string[9] = (rd_23 & BIT_MASK) | BIT_SET;                       \
    string[10] = ((rd2 ^ rd4) & BIT_MASK) | BIT_SET;                \
    string[11] = ((rd2 ^ rd5) & BIT_MASK) | BIT_SET;                \
    string[12] = (rd_34 & BIT_MASK) | BIT_SET;                      \
    string[13] = ((rd3 ^ rd5) & BIT_MASK) | BIT_SET;                \
    string[14] = (rd_45 & BIT_MASK) | BIT_SET;                      \
    string[15] = (rd_123 & BIT_MASK) | BIT_SET;                     \
    string[16] = ((rd_12 ^ rd4) & BIT_MASK) | BIT_SET;              \
    string[17] = ((rd_12 ^ rd5) & BIT_MASK) | BIT_SET;              \
    string[18] = ((rd_13 ^ rd4) & BIT_MASK) | BIT_SET;              \
    string[19] = ((rd_13 ^ rd5) & BIT_MASK) | BIT_SET;              \
    string[20] = ((rd1 ^ rd_45) & BIT_MASK) | BIT_SET;              \
    string[21] = (rd_234 & BIT_MASK) | BIT_SET;                     \
    string[22] = ((rd_23 ^ rd5) & BIT_MASK) | BIT_SET;              \
    string[23] = ((rd2 ^ rd_45) & BIT_MASK) | BIT_SET;              \
    string[24] = (rd_345 & BIT_MASK) | BIT_SET;                     \
    string[25] = (rd_1234 & BIT_MASK) | BIT_SET;                    \
    string[26] = ((rd_123 ^ rd5) & BIT_MASK) | BIT_SET;             \
    string[27] = ((rd_12 ^ rd_45) & BIT_MASK) | BIT_SET;            \
    string[28] = ((rd1 ^ rd_345) & BIT_MASK) | BIT_SET;             \
    string[29] = ((rd2 ^ rd_345) & BIT_MASK) | BIT_SET;             \
    string[30] = ((rd_1234 ^ rd5) & BIT_MASK) | BIT_SET;            \
}

/**
 * Fast random string generator
 * @brief Generates a string of characters which can be accepted by memmcached as a key.
 *        Use a reduced ASCII set.
 * @param buf buffer to write characters in
 * @param length buffer length
 * @seed random seed
 */
void
string_frandom(const char *buf, size_t length, uint64_t seed) {

    size_t len = length;
    uint64_t level = 0;
    int index = (seed & 0x30) >> 4;
    register uint64_t BIT_SET = bit_sets[index];
    register uint64_t BIT_MASK = bit_masks[index];

    /*
     * string generator selector
     * Select a generator by the length of the buf
     */
#define selector \
    entry(248, 5)       \
    entry(120, 4)       \
    entry(56, 3)        \
    entry(24, 2)        \
    entry(8, 1)
    /* end */

    while (len > 0) {
    /*
     * Selector entry
     * If the size of the buf fits, them jump to corresponding string generator
     */
#define entry(size, jump) \
    if (len >= size) {                                                         \
        string_gen_ ## jump(buf, seed, &level);                                \
        len -= size;                                                           \
        buf += size;                                                           \
        continue;                                                              \
    } else
    /* end */
        selector // start the selector
        {
            /* handle the last few bytes */
            string_gen_last(buf, seed, &level, len);
            break;
        }
#undef entry
    };
}


/**
 * mix -- mix 3 64-bit values reversibly.
 * mix() takes 48 machine instructions, but only 24 cycles on a superscalar
 * machine (like Intel's new MMX architecture).  It requires 4 64-bit
 * registers for 4::2 parallelism.
 * All 1-bit deltas, all 2-bit deltas, all deltas composed of top bits of
 * (a,b,c), and all deltas of bottom bits were tested.  All deltas were
 * tested both on random keys and on keys that were nearly all zero.
 * These deltas all cause every bit of c to change between 1/3 and 2/3
 * of the time (well, only 113/400 to 287/400 of the time for some
 * 2-bit delta).  These deltas all cause at least 80 bits to change
 * among (a,b,c) when the mix is run either forward or backward (yes it
 * is reversible).
 * This implies that a hash using mix64 has no funnels.  There may be
 * characteristics with 3-bit deltas or bigger, I didn't test for
 * those.
 */
#define mix64(a,b,c) \
{                                        \
    a -= b; a -= c; a ^= (c >> 43);      \
    b -= c; b -= a; b ^= (a<<9);         \
    c -= a; c -= b; c ^= (b>>8);         \
    a -= b; a -= c; a ^= (c>>38);        \
    b -= c; b -= a; b ^= (a<<23);        \
    c -= a; c -= b; c ^= (b>>5);         \
    a -= b; a -= c; a ^= (c>>35);        \
    b -= c; b -= a; b ^= (a<<49);        \
    c -= a; c -= b; c ^= (b>>11);        \
    a -= b; a -= c; a ^= (c>>12);        \
    b -= c; b -= a; b ^= (a<<18);        \
    c -= a; c -= b; c ^= (b>>22);        \
}

uint64_t
frandom(k, level)
register uint64_t k;        /* the key */
register uint64_t level;    /* the previous hash, or an arbitrary value */
{
    register uint64_t a,b;
    register uint64_t c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

    /* Set up the internal state */
    b = level;
    a = k + level;
    mix64(a,b,c);
    return c;
}

