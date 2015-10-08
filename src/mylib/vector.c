/*
 * File:     vector.c
 * Date:     Dec 3, 2010
 * Author:   Haowei
 */
#include <string.h>

#include "log.h"
#include "mylib.h"
#include "my_mutex.h"
#include "return.h"

#include "vector.h"

#define VECTOR_INIT_SIZE 10
#define VECTOR_GROW_FACTOR 2

typedef struct {
    void *entry;
} vector_element_t;

struct vector_t {
    vector_element_t         *elements;
    size_t                    start;
    size_t                    end;
    size_t                    capacity;
    vector_entry_free_func    entry_free_func;
    my_rwlock_t               rwlock;
    bool                      isempty;
};

static inline size_t
get_next_index(vector_t *vector, size_t index) {
    return index < vector->capacity - 1 ? index + 1 : 0;
}

static inline size_t
get_prev_index(vector_t *vector, size_t index) {
    return index == 0 ? vector->capacity - 1 : index - 1;
}

static inline size_t
get_real_index(vector_t *vector, size_t index) {
    size_t real_index = vector->start + index;
    return real_index < vector->capacity ?
           real_index :
           real_index - vector->capacity;
}

static inline bool
vector_isfull(vector_t *vector) {
    if (!vector->isempty && vector->end == vector->start) {
        return true;
    } else {
        return false;
    }
}

static ret_t
vector_grow(vector_t *vector) {
    my_assert(vector_isfull(vector));

    size_t new_capacity = vector->capacity * VECTOR_GROW_FACTOR;

    vector_element_t *new_elements =
        my_malloc(new_capacity * sizeof(new_capacity));

    size_t first_size = vector->capacity - vector->start;
    memcpy(new_elements,
           vector->elements + vector->start,
           sizeof(vector_element_t) * first_size);

    memcpy(new_elements + first_size,
           vector->elements,
           sizeof(vector_element_t) * vector->start);

    free(vector->elements);

    vector->elements = new_elements;
    vector->start    = 0;
    vector->end      = vector->capacity;
    vector->capacity = new_capacity;

    return RET_SUCCEED;
}

vector_t *
vector_alloc(vector_entry_free_func entry_free_func) {

    vector_t *vector = my_malloc(sizeof(vector_t));

    vector->capacity         = VECTOR_INIT_SIZE;
    vector->elements         =
        my_malloc(vector->capacity * sizeof(vector_element_t));
    vector->start            = 0;
    vector->end              = 0;
    vector->entry_free_func  = entry_free_func;
    vector->rwlock           = (my_rwlock_t)MY_RWLOCK_INITER;
    vector->isempty          = true;

    return vector;
}

ret_t
vector_push_back(vector_t *vector, void *entry) {
    ret_t ret = RET_SUCCEED;

    my_wrlock(&vector->rwlock);

    if (vector_isfull(vector) && vector_grow(vector) != RET_SUCCEED) {
        log_error("Failed to grow vector");
        ret = RET_FAILED;
    } else {

        vector->elements[vector->end].entry = entry;
        vector->end = get_next_index(vector, vector->end);
        vector->isempty = false;
    }

    my_rwunlock(&vector->rwlock);
    return ret;
}

void *
vector_pop(vector_t *vector) {
    void *ret;
    my_wrlock(&vector->rwlock);

    if (vector->isempty) {
        ret = NULL;
    } else {
        size_t index = get_prev_index(vector, vector->end);
        ret = vector->elements[index].entry;
        vector->end = index;
        if (vector->end == vector->start)
            vector->isempty = true;
    }
    my_rwunlock(&vector->rwlock);

    return ret;
}

// Return succeed or failed
ret_t
vector_unshift(vector_t *vector, void *entry) {
    ret_t ret = RET_SUCCEED;

    my_wrlock(&vector->rwlock);

    if (vector_isfull(vector) && vector_grow(vector) != RET_SUCCEED) {
        log_error("Failed to grow vector");
        ret = RET_FAILED;
    } else {
        size_t index = get_prev_index(vector, vector->start);

        vector->elements[index].entry = entry;
        vector->start = index;
        vector->isempty = false;
    }

    my_rwunlock(&vector->rwlock);
    return ret;
}

// Return the entry pointer
void *
vector_shift(vector_t *vector) {
    void *ret;
    my_wrlock(&vector->rwlock);

    if (vector->isempty) {
        ret = NULL;
    } else {
        ret = vector->elements[vector->start].entry;
        vector->start = get_next_index(vector, vector->start);
        if (vector->end == vector->start)
            vector->isempty = true;
    }
    my_rwunlock(&vector->rwlock);

    return ret;
}

ret_t
vector_insert(vector_t *vector, void *entry, size_t index) {
    ret_t ret = RET_FAILED;

    my_wrlock(&vector->rwlock);

    // TODO implement
    log_error("vector_insert not implemented yet!");

    my_rwunlock(&vector->rwlock);
    return ret;
}

void *
vector_remove(vector_t *vector, size_t index) {
    my_wrlock(&vector->rwlock);

    // TODO implement
    log_error("vector_remove not implemented yet!");

    my_rwunlock(&vector->rwlock);

    return NULL;
}

// Return the entry pointer
void *
vector_get(vector_t *vector, size_t index) {
    void *ret;
    my_rdlock(&vector->rwlock);

    if (vector->isempty || index >= vector->capacity) {
        log_error("Invalid vector index: %zd", index);
        ret = NULL;
    } else {
        size_t real_index = get_real_index(vector, index);
        ret = vector->elements[real_index].entry;
    }
    my_rwunlock(&vector->rwlock);

    return ret;
}

// Return the current size of vector
size_t
vector_size(vector_t *vector) {
    size_t ret;
    my_rdlock(&vector->rwlock);
    if (vector->isempty) {
        ret = 0;
    } else {
        ret = vector->start < vector->end ?
              vector->end - vector->start :
              vector->capacity - vector->start + vector->end;
    }
    my_rwunlock(&vector->rwlock);
    return ret;
}

bool
vector_isempty(vector_t *vector) {
    // Do not add lock here
    return vector->isempty;
}

ret_t
vector_empty(vector_t *vector) {
    my_wrlock(&vector->rwlock);

    if (!vector->isempty) {
        for (size_t i = vector->start; i != vector->end;
             i = get_next_index(vector, i)) {
            (*vector->entry_free_func)(vector->elements[i].entry);
        }

    }

    vector->elements       =
        my_realloc(vector->elements, sizeof(vector_element_t) * VECTOR_INIT_SIZE);
    vector->start          = 0;
    vector->end            = 0;
    vector->isempty        = true;
    vector->capacity       = VECTOR_INIT_SIZE;

    my_rwunlock(&vector->rwlock);

    return RET_SUCCEED;
}

void
vector_destroy(vector_t *vector) {
    my_wrlock(&vector->rwlock);

    for (size_t i = vector->start; i != vector->end;
         i = get_next_index(vector, i)) {
        (*vector->entry_free_func)(vector->elements[i].entry);
    }

    free(vector->elements);
    free(vector);
}

