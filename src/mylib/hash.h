#ifndef HASH_H
#define HASH_H

#include <sys/types.h>
#include "return.h"

typedef struct hash_t hash_t;

typedef void *(*hash_entry_key_func_t)(void *entry);

typedef size_t (*hash_key_size_func_t)(void *key);

typedef void (*hash_entry_free_func_t)(void *entry);

hash_t *
hash_alloc(hash_entry_key_func_t   hash_entry_key_func,
           hash_key_size_func_t    hash_key_size_func,
           hash_entry_free_func_t  hash_entry_free_func);

ret_t
hash_add(hash_t *hash, void *entry);

void *
hash_get(hash_t *hash, void *key);

ret_t
hash_remove(hash_t *hash, void *key);

void **
hash_get_keys(hash_t *hash, size_t *number);

void **
hash_get_entries(hash_t *hash, size_t *number);

void
hash_destroy(hash_t *hash);

#endif // HASH_H
