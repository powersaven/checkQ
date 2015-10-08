#include "mylib.h"
#include "vector.h"

#define TEST_NUM 1000 

// Test data structure
typedef struct {
    int      num;
} data_t;

void
data_free_func(void *entry) {
    data_t *data = (data_t *)entry;
    log_debug("Freeing %d", data->num);
    free(data);
}

#define test(func, expect, got) \
    do {                                                               \
        if ((expect) != (got)) {                                       \
            log_error(#func " expect:" #expect "=%d got:" #got "=%d",  \
                      expect, got);                                    \
            return RET_FAILED;                                         \
        }                                                              \
    } while (0)

int
main(int argc, char *argv[]) {

    vector_t *vector = vector_alloc(data_free_func);
    if (vector == NULL) {
        log_error("Failed to alloc vector");
        return RET_FAILED;
    }

    for (int i = 0; i < TEST_NUM; i ++) {
        data_t *data = my_malloc(sizeof(data_t));
        data->num = i;

        vector_unshift(vector, data);
        for (int j = 0; j <= i; j ++) {
            data_t *unshift = vector_get(vector, j);
            test(vector_get, i - j, unshift->num);
        }

    }


    for (int i = 0; i < TEST_NUM; i ++) {
        data_t *data = vector_pop(vector);
        test(vector_pop, i, data->num);
        data_free_func(data);
    }

    vector_empty(vector);
    for (int i = 0; i < TEST_NUM; i ++) {
        data_t *data = my_malloc(sizeof(data_t));
        data->num = i;
        if (i & 0x1) {
            vector_unshift(vector, data);
        } else {
            vector_push_back(vector, data);
        }
    }

    for (int i = 0; i < TEST_NUM / 2; i ++) {
        data_t *data = vector_shift(vector);
        test(vector_shift, TEST_NUM - 2 * i - 1, data->num);
        data_free_func(data);
    }
    
    for (int i = 0; i < TEST_NUM / 2; i ++) {
        data_t *data = vector_shift(vector);
        test(vector_shift, 2 * i, data->num);
        data_free_func(data);
    }
    
    vector_empty(vector);

    for (int i = 0; i < TEST_NUM; i ++) {
        data_t *data = my_malloc(sizeof(data_t));
        data->num = i;

        vector_push_back(vector, data);
    }

    for (int i = 0; i < vector_size(vector); i ++) {
        data_t *data = vector_get(vector, i);
        test(vector_get, i, data->num);
    }


    vector_destroy(vector);
}
