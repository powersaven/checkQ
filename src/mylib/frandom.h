#ifndef FRANDOM_H
#define FRANDOM_H
#include <string.h>
#include <stdint.h>

/*
 * Fast random string generator
 */

/**
 * Fast random string generator
 * @brief Generates a string of characters which can be accepted by memmcached as a key.
 *        Use a reduced ASCII set.
 * @param buf buffer to write characters in
 * @param length buffer length
 * @seed random seed
 */
void string_frandom(const char *buf, size_t length, uint64_t seed);

/**
 * Fast random number generator
 */
uint64_t frandom(uint64_t seed, uint64_t level);

#endif /* FRANDOM_H */
