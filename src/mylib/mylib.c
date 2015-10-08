#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "mylib.h"

void *
cq_malloc(const size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        log_error("Out of memory!");
        abort();
    }
    return ptr;
}

void
my_free(void *p) {
    free(p);
}

void *
my_realloc(void *ptr, size_t size) {
    void *product = realloc(ptr, size);
    if (size != 0 && product == NULL) {
        log_error("Out of memory!");
        abort();
    }

    return ptr;
}

ssize_t
my_read(int fd, void *buf, size_t count) {
    size_t rdsz;
    size_t total = count;

    while ((rdsz = read(fd, buf, count)) >= 0) {
        count -= rdsz;
        if (errno != EAGAIN && errno != 0) break;
        if (count == 0) return total;
    }

    return -1;
}

ssize_t
my_write(int fd, void *buf, size_t count) {
    size_t wrsz;
    size_t total = count;

    while ((wrsz = write(fd, buf, count))) {
        if (wrsz < 0 && errno != EINPROGRESS)
            break;
        count -= wrsz;
        if (errno != EAGAIN && errno != 0 && errno != EINPROGRESS) break;
        if (count == 0) return total;
    }

    log_error("wrsz: write %zd\n", wrsz);
    return -1;
}

