//
// File:       hash_test.c
// Date:       Dec 3, 2010
// Author:     Haowei
//

#include "hash.h"
#include "log.h"
#include "mylib.h"

typedef struct {
    int key;
    int value;
} entry_t;

void *
entry_key_func(void *entry) {
    return &(((entry_t *)entry)->key);
}

size_t
entry_key_size_func(void *key) {
    return sizeof(int);
}

void
entry_free_func(void *entry) {
    free(entry);
}

int
main(int argc, char *argv[]) {

    log_info("Starting hash test...");

    hash_t *hash = hash_alloc(entry_key_func,
                              entry_key_size_func,
                              entry_free_func);

    if (hash == NULL) {
        log_error("Failed to alloc hash");
        return RET_FAILED;
    }

    for (int i = 0; i < 1000; i ++) {
        entry_t *entry = my_malloc(sizeof(entry_t));
        entry->key = i;
        entry->value = i;
        if (hash_add(hash, entry) != RET_SUCCEED) {
            log_error("Failed to add hash entry");
            return RET_FAILED;
        }
    }

    for (int i = 0; i < 1000; i ++) {
        entry_t *entry = hash_get(hash, &i);
        if (entry == NULL) {
            log_error("Get miss: %d", i);
            return RET_FAILED;
        }
        if (entry->key != i) {
            log_error("Key not OK: entry->key: %d, i: %d", entry->key, i);
            return RET_FAILED;
        }

        if (entry->value != i) {
            log_error("Key not OK: entry->key: %d, i: %d", entry->value, i);
            return RET_FAILED;
        }

    }

    hash_destroy(hash);
}

