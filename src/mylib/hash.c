#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "return.h"
#include "mylib.h"
#include "log.h"
#include "my_mutex.h"

#include "hash.h"

#define HASH_INIT_SIZE 10
#define HASH_GROW_FACTOR 2
#define REHASH_RATE 2

typedef enum {
    HASH_EMPTY = 0,
    HASH_OCCUPIED,
    HASH_REMOVED,
} hash_entry_status_t;

typedef struct {
    void                     *entry;
    void                     *key;
    size_t                    key_size;
    hash_entry_status_t       status;
} hash_element_t;

struct hash_t {
    hash_element_t           *elements;
    hash_entry_key_func_t     hash_entry_key_func;
    hash_key_size_func_t      hash_key_size_func;
    hash_entry_free_func_t    hash_entry_free_func;
    size_t                    hash_size;
    size_t                    hash_entry_count;
    size_t                    hash_entry_removed_count;
    my_rwlock_t               hash_rwlock;
};

static int
find_first_prime(int n) {
    while (n ++) {
        int m = (int)sqrt(n);
        int find_flag = 1;
        for (int i = 2; i <= m; i ++) {
            if (n % i == 0) {
                find_flag = 0;
                break;
            }
        }
        if (find_flag == 1) {
            return n;
        }
    }
    return -1;
}

#define mix64(a,b,c) \
{                                \
  a -= b; a -= c; a ^= (c>>43);  \
  b -= c; b -= a; b ^= (a<<9);   \
  c -= a; c -= b; c ^= (b>>8);   \
  a -= b; a -= c; a ^= (c>>38);  \
  b -= c; b -= a; b ^= (a<<23);  \
  c -= a; c -= b; c ^= (b>>5);   \
  a -= b; a -= c; a ^= (c>>35);  \
  b -= c; b -= a; b ^= (a<<49);  \
  c -= a; c -= b; c ^= (b>>11);  \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<18);  \
  c -= a; c -= b; c ^= (b>>22);  \
}

static uint64_t
hash_func(void *key, size_t size, uint64_t level) {

    uint64_t length = (uint64_t)size;
    uint64_t a, b, c, len;
    unsigned char *k = key;

    /* Set up the internal state */
    len = length;
    a = b = level; /* the previous hash value */
    c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

    /*---------------------------------------- handle most of the key */
    if (((size_t) k) & 7) {
        while (len >= 24) {
            a += (k[0]
                    + ((uint64_t)k[1]<<8)
                    + ((uint64_t)k[2]<<16)
                    + ((uint64_t)k[3] << 24)
                    + ((uint64_t) k[4] << 32)
                    + ((uint64_t) k[5] << 40)
                    + ((uint64_t) k[6] << 48)
                    + ((uint64_t) k[7] << 56));
            b += (k[8]
                    + ((uint64_t) k[9] << 8)
                    + ((uint64_t) k[10] << 16)
                    + ((uint64_t) k[11] << 24)
                    + ((uint64_t) k[12] << 32)
                    + ((uint64_t) k[13] << 40)
                    + ((uint64_t) k[14] << 48)
                    + ((uint64_t) k[15] << 56));
            c += (k[16] +
                    ((uint64_t) k[17] << 8)
                    + ((uint64_t) k[18] << 16)
                    + ((uint64_t) k[19] << 24)
                    + ((uint64_t) k[20] << 32)
                    + ((uint64_t) k[21] << 40)
                    + ((uint64_t) k[22] << 48)
                    + ((uint64_t) k[23] << 56));
            mix64(a, b, c);
            k += 24;
            len -= 24;
        }
    } else {
        while (len >= 24) /* aligned */
        {
            a += *(uint64_t *) (k + 0);
            b += *(uint64_t *) (k + 8);
            c += *(uint64_t *) (k + 16);
            mix64(a, b, c);
            k += 24;
            len -= 24;
        }
    }

    /*------------------------------------- handle the last 23 bytes */
    c += length;
    switch (len) /* all the case statements fall through */
    {
    case 23: c += ((uint64_t) k[22] << 56);
    case 22: c += ((uint64_t) k[21] << 48);
    case 21: c += ((uint64_t) k[20] << 40);
    case 20: c += ((uint64_t) k[19] << 32);
    case 19: c += ((uint64_t) k[18] << 24);
    case 18: c += ((uint64_t) k[17] << 16);
    case 17: c += ((uint64_t) k[16] << 8);
        /* the first byte of c is reserved for the length */
    case 16: b += ((uint64_t) k[15] << 56);
    case 15: b += ((uint64_t) k[14] << 48);
    case 14: b += ((uint64_t) k[13] << 40);
    case 13: b += ((uint64_t) k[12] << 32);
    case 12: b += ((uint64_t) k[11] << 24);
    case 11: b += ((uint64_t) k[10] << 16);
    case 10: b += ((uint64_t) k[9] << 8);
    case 9: b += ((uint64_t) k[8]);
    case 8: a += ((uint64_t) k[7] << 56);
    case 7: a += ((uint64_t) k[6] << 48);
    case 6: a += ((uint64_t) k[5] << 40);
    case 5: a += ((uint64_t) k[4] << 32);
    case 4: a += ((uint64_t) k[3] << 24);
    case 3: a += ((uint64_t) k[2] << 16);
    case 2: a += ((uint64_t) k[1] << 8);
    case 1: a += ((uint64_t) k[0]);
        /* case 0: nothing left to add */
    }
    mix64(a, b, c);
    /*-------------------------------------------- report the result */
    return c;
}

hash_t *
hash_alloc(hash_entry_key_func_t   hash_entry_key_func,
           hash_key_size_func_t    hash_key_size_func,
           hash_entry_free_func_t  hash_entry_free_func) {
    hash_t *hash = my_malloc(sizeof(hash_t));

    size_t init_size = find_first_prime(HASH_INIT_SIZE);

    hash->elements = my_malloc(init_size * sizeof(hash_element_t));
    memset(hash->elements, 0, init_size * sizeof(hash_element_t));

    hash->hash_entry_key_func = hash_entry_key_func;
    hash->hash_entry_free_func = hash_entry_free_func;
    hash->hash_key_size_func = hash_key_size_func;

    hash->hash_size = init_size;
    hash->hash_entry_removed_count = 0;
    hash->hash_entry_count = 0;

    hash->hash_rwlock = (my_rwlock_t)MY_RWLOCK_INITER;

    return hash;
}

static size_t
hash_find(hash_t *hash, void *key, size_t key_size, hash_entry_status_t *status) {

    for (uint64_t level = 0; ; level ++) {
        uint64_t hash_key = hash_func(key, key_size, level);
        size_t index = hash_key % hash->hash_size;
        hash_element_t *element = &(hash->elements[index]);

        switch (element->status) {
        case HASH_EMPTY:
            *status = HASH_EMPTY;
            return index;
        case HASH_OCCUPIED:
            {
                void *get_key = element->key;
                size_t get_key_size = element->key_size;
                if (get_key_size == key_size
                        && memcmp(key, get_key, key_size) == 0) {
                    *status = HASH_OCCUPIED;
                    return index;
                }
            }
        case HASH_REMOVED: break;
        default:
            log_error("Invalid hash entry status: %d", element->status);
            exit(RET_FAILED);
        }
    }
}

static ret_t
rehash(hash_t *hash, hash_element_t *src_elements, size_t src_size) {
    memset(hash->elements, 0, sizeof(hash_element_t) * hash->hash_size);
    for (size_t i = 0; i < src_size; i ++) {
        hash_element_t *element = &(src_elements[i]);
        if (element->status == HASH_OCCUPIED) {
            hash_entry_status_t status;
            size_t index = hash_find(hash, element->key, element->key_size, &status);
            if (status != HASH_EMPTY) {
                log_error("Rehash found a duplicated key. "
                          "Please make sure each key is an individual copy.");
                return RET_FAILED;
            }
            hash->elements[index] = *element;
        }
    }
    return RET_SUCCEED;
}

ret_t
hash_add(hash_t *hash, void *entry) {
    ret_t ret = RET_SUCCEED;

    my_wrlock(&hash->hash_rwlock);

    hash_entry_status_t status;

    void *key = (*hash->hash_entry_key_func)(entry);
    size_t key_size = (*hash->hash_key_size_func)(key);

    size_t index = hash_find(hash, key, key_size, &status);

    if (status != HASH_EMPTY) {
        ret = RET_FAILED;
    } else {
        hash->elements[index] = (hash_element_t){
            .entry      = entry, 
            .key        = key, 
            .key_size   = key_size,
            .status     = HASH_OCCUPIED,
        };
        hash->hash_entry_count ++;

        if ((hash->hash_entry_count + hash->hash_entry_removed_count)
                * REHASH_RATE > hash->hash_size) {
            // A rehash is required
            size_t rehash_size;
            if (hash->hash_entry_count < hash->hash_entry_removed_count) {
                rehash_size = hash->hash_size;
            } else {
                rehash_size = find_first_prime(hash->hash_size * REHASH_RATE);
            }
            hash_element_t *rehash_elements = my_malloc(rehash_size * sizeof(hash_element_t));
            hash_element_t *old_elements = hash->elements;
            size_t old_size = hash->hash_size;
            hash->elements = rehash_elements;
            hash->hash_size = rehash_size;
            hash->hash_entry_removed_count = 0;

            if (rehash(hash, old_elements, old_size) != RET_SUCCEED) {
                log_error("Failed to rehash: %zd->%zd", old_size, rehash_size);
                return RET_FAILED;
            }

            free(old_elements);
        }
    }

    my_rwunlock(&hash->hash_rwlock);

    return ret;
}

void *
hash_get(hash_t *hash, void *key) {
    void *ret = NULL;

    my_rdlock(&hash->hash_rwlock);

    hash_entry_status_t status;

    size_t key_size = (*hash->hash_key_size_func)(key);
    size_t index = hash_find(hash, key, key_size, &status);

    if (status == HASH_OCCUPIED)
        ret = hash->elements[index].entry;

    my_rwunlock(&hash->hash_rwlock);
    return ret;
}

ret_t
hash_remove(hash_t *hash, void *key) {

    my_wrlock(&hash->hash_rwlock);

    hash_entry_status_t status;

    size_t key_size = (*hash->hash_key_size_func)(key);
    size_t index = hash_find(hash, key, key_size, &status);

    if (status == HASH_OCCUPIED) {
        hash->elements[index].status = HASH_REMOVED;
        hash->hash_entry_count --;
        hash->hash_entry_removed_count ++;
        (*hash->hash_entry_free_func)(hash->elements[index].entry);
    }

    my_rwunlock(&hash->hash_rwlock);
    return RET_SUCCEED;
}

void **
hash_get_entries(hash_t *hash, size_t *number) {
    my_rdlock(&hash->hash_rwlock);
    void **ret = my_malloc(hash->hash_entry_count * sizeof(void *));
    size_t ret_ptr = 0;
    for (size_t i = 0; i < hash->hash_size; i ++) {
        hash_element_t *element = &hash->elements[i];
        if (element->status == HASH_OCCUPIED) {
            ret[ret_ptr ++] = element->entry;
        }
    }
    *number = hash->hash_entry_count;
    my_rwunlock(&hash->hash_rwlock);
    return ret;
}

void **
hash_get_keys(hash_t *hash, size_t *number) {
    my_rdlock(&hash->hash_rwlock);
    void **ret = my_malloc(hash->hash_entry_count * sizeof(void *));
    size_t ret_ptr = 0;
    for (size_t i = 0; i < hash->hash_size; i ++) {
        hash_element_t *element = &(hash->elements[i]);
        if (element->status == HASH_OCCUPIED) {
            ret[ret_ptr ++] = element->key;
        }
    }
    *number = hash->hash_entry_count;
    my_rwunlock(&hash->hash_rwlock);
    return ret;
}

void
hash_destroy(hash_t *hash) {
    my_wrlock(&hash->hash_rwlock);
    for (size_t i = 0; i < hash->hash_size; i ++) {
        hash_element_t *element = &(hash->elements[i]);
        if (element->status == HASH_OCCUPIED) {
            (*hash->hash_entry_free_func)(element->entry);
        }
    }
    free(hash->elements);
    free(hash);
}

