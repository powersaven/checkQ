/*
 * File:     vector.h
 * Date:     Dec 3, 2010
 * Author:   Haowei
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <sys/types.h>

#include "return.h"
#include "my_bool.h"

typedef struct vector_t vector_t;

typedef void (*vector_entry_free_func)(void *entry);

/**
 * Allocate and initialize a vector
 * @param entry_free_func    A function pointer. This function frees the vector element
 * @return vector_t *        The vector instance (pointer)
 */
vector_t *
vector_alloc(vector_entry_free_func entry_free_func);

/**
 * Push a vector entry to the end of vector. The vector size will grow by 1
 * @param vector     The vector instance
 * @param entry      The pointer of entry which the user wish to push
 * @return ret_t     RET_FAILED for failed, or RET_SUCCEED for succeed
 */
ret_t
vector_push_back(vector_t *vector, void *entry);

/**
 * Pop a vector from the end of vector. The vector size will be shrinked by 1
 * @param vector      The vector instance
 * @return void *     The pointer of poped entry
 */
void * 
vector_pop(vector_t *vector);

/**
 * Push a vector entry to the beginning of vector. The vector size will grow by 1
 * @param vector     The vector instance
 * @param entry      The pointer of entry which the user wish to push
 * @return ret_t     RET_FAILED for failed, or RET_SUCCEED for succeed
 */
ret_t
vector_unshift(vector_t *vector, void *entry);

/**
 * Pop a vector from the end of vector. The vector size will be shrinked by 1
 * @param vector      The vector instance
 * @return void *     The pointer of poped entry
 */
void *
vector_shift(vector_t *vector);

ret_t
vector_insert(vector_t *vector, void *entry, size_t index);

void *
vector_remove(vector_t *vector, size_t index);

// Return the entry pointer
void *
vector_get(vector_t *vector, size_t index);

// Return the current size of vector
size_t
vector_size(vector_t *vector);

bool
vector_isempty(vector_t *vector);

ret_t
vector_empty(vector_t *vector);

void
vector_destroy(vector_t *vector);

#endif // VECTOR_H
