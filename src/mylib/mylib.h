#ifndef MYLIB_H
#define MYLIB_H

#include <sys/types.h>

#include "sig_hdl.h"
#include "my_bool.h"
#include "my_assert.h"

#define attr_unused __attribute__((unused))

#define my_malloc cq_malloc

void *
cq_malloc(const size_t size);

void
my_free(void* p);

void *
my_realloc(void *ptr, size_t size);

ssize_t
my_read(int fd, void *buf, size_t count);

ssize_t
my_write(int fd, void *buf, size_t count);

#endif /* MYLIB_H */
