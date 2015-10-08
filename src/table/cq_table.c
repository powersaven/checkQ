#include <stdlib.h>
#include <string.h>

#include "cq_table.h"
#include "mylib.h"
#include "return.h"
#include "log.h"
#include "hash.h"
#include "vector.h"

typedef struct schema_set schema_set_t;
typedef struct table_set  table_set_t;

struct table_data {
    char*             name;
    table_data_type_t type;
    int               len;   // only varchar need define len
};

struct schema {
    table_data_t** schemas;
    int            capacity;
    int            nschema;
};

struct table_schema {
    schema_t*  key_schema;
    schema_t*  val_schema;
};

struct schema_set {
    vector_t* schemas;
};

struct table {
    char*           name;
    table_schema_t* schema;
};

struct table_set {
    hash_t* table_set;
};

static schema_set_t* cq_schema_set = NULL;
static table_set_t*  cq_table_set  = NULL;

schema_t* get_table_key_schema(table_schema_t* pschema) {
    return pschema->key_schema;
}

schema_t* get_table_val_schema(table_schema_t * pschema) {
    return pschema->val_schema;
}

table_schema_t* get_table_schema(table_t *ptable) {
    return ptable->schema;
}

int is_table_schema_full(table_schema_t *pschema) {

    schema_t *pschema_val = get_table_val_schema(pschema);

    if (pschema_val->nschema >= pschema_val->capacity) {
        return 1;
    } 

    return 0;
}

void add_table_data(table_schema_t *pschema) {
    
    schema_t     *pschema_val = get_table_val_schema(pschema);
    table_data_t *pdata       = NULL;
    char data_name[10];
    sprintf(data_name, "col%d", pschema_val->nschema);

    if (rand() % 2 == 0) {
        pdata = table_data_init(data_name, BIGINT, 0);
    } else {
        int len = rand() % 503 + 10;
        pdata = table_data_init(data_name, VARCHAR, len);
    }

    schema_add(pschema_val, pdata);
}

void drop_table_data(table_schema_t *pschema) {
    
    schema_t *pschema_val = pschema->val_schema;
    
    table_data_destroy(pschema_val->schemas[pschema_val->nschema - 1]);
    
    --pschema_val->nschema;
}

int get_schema_capacity(schema_t *pschema) {
    return pschema->capacity;
}

table_data_t* table_data_init(char* name, table_data_type_t type, int len) {
    table_data_t* pdata = my_malloc(sizeof(table_data_t));

    pdata->name = strdup(name);
    if (NULL == pdata->name) {
        log_error("Failed to strdup table data name!");
        return NULL;
    }

    pdata->type = type;
    pdata->len  = len;

    return pdata;
}

void table_data_destroy(table_data_t* ptable_data) {
    my_free(ptable_data->name);
    my_free(ptable_data);
}

char* get_table_data_name(const table_data_t* ptable_data) {
    return ptable_data->name;
}

table_data_type_t get_table_data_type(const table_data_t* ptable_data) {
    return ptable_data->type;
}

int get_table_data_len(const table_data_t* ptable_data) {
    return ptable_data->len;
}

schema_t* schema_init() {
    schema_t* pschema = my_malloc(sizeof(schema_t));

    int capacity = 100;     // fix me
    pschema->nschema  = 0;
    pschema->capacity = capacity;
    pschema->schemas  = my_malloc(sizeof(table_data_t *) * capacity);

    return pschema;
}

void schema_destroy(schema_t* pschema) {
    for (int i = 0; i < pschema->nschema; i++) {
        table_data_destroy(pschema->schemas[i]);
    }

    my_free(pschema->schemas);
    my_free(pschema);
}

ret_t schema_add(schema_t* pschema, table_data_t* ptable_data) {
    if (pschema->nschema == pschema->capacity) {
        log_error("Table schema is full!");
        return RET_FAILED;
    }

    pschema->schemas[pschema->nschema] = ptable_data;
    pschema->nschema++;

    return RET_SUCCEED;
}

table_schema_t* table_schema_init(schema_t* key_schema,
                                  schema_t* val_schema) {
    table_schema_t* ptable_schema = my_malloc(sizeof(table_schema_t));

    ptable_schema->key_schema = key_schema;
    ptable_schema->val_schema = val_schema;

    return ptable_schema;
}

void table_schema_destroy(table_schema_t* ptable_schema) {
    schema_destroy(ptable_schema->key_schema);
    schema_destroy(ptable_schema->val_schema);

    my_free(ptable_schema);
}

const table_data_t** get_schema_key_types(table_schema_t* schema, 
    int* nschema) {
    schema_t* key_schema = schema->key_schema;
    *nschema = key_schema->nschema;

    return (const table_data_t **)(key_schema->schemas);
}

const table_data_t** get_schema_val_types(table_schema_t* schema, int* nschema) {
    schema_t* val_schema = schema->val_schema;
    *nschema = val_schema->nschema;

    return (const table_data_t **)(val_schema->schemas);
}

table_t* table_init(char* name, table_schema_t* ptable_schema) {
    table_t* ptable = my_malloc(sizeof(table_t));

    ptable->name = strdup(name);
    if (NULL == ptable->name) {
        log_error("Failed to strdup table name!");
        return NULL;
    }
    ptable->schema = ptable_schema;

    return ptable;
}

void table_destroy(table_t* ptable) {
    my_free(ptable->name);
    my_free(ptable);

    return;
}

char* get_table_name(table_t* ptable) {
    return ptable->name;
}

const table_data_t** get_table_key_types(table_t* ptable, int* nschema) {
    table_schema_t* schema = ptable->schema;
    schema_t* key_schema   = schema->key_schema;

    *nschema = key_schema->nschema;

    return (const table_data_t **)(key_schema->schemas);
}

const table_data_t** get_table_val_types(table_t* ptable, int* nschema) {
    table_schema_t* schema = ptable->schema;
    schema_t* val_schema   = schema->val_schema;

    *nschema = val_schema->nschema;

    return (const table_data_t **)(val_schema->schemas);
}

void schema_set_init() {
    cq_schema_set = my_malloc(sizeof(schema_set_t));

    cq_schema_set->schemas
        = vector_alloc((vector_entry_free_func)table_schema_destroy);

    return;
}

void schema_set_destroy() {
    vector_destroy(cq_schema_set->schemas);
    my_free(cq_schema_set);
}

ret_t schema_set_add(table_schema_t* ptable_schema) {
    return vector_push_back(cq_schema_set->schemas, ptable_schema);
}

table_schema_t* schema_set_get_random() {
    if (vector_isempty(cq_schema_set->schemas)) {
        log_error("schema set is empty!");
        return NULL;
    }

    int schema_index = rand() % vector_size(cq_schema_set->schemas);

    return vector_get(cq_schema_set->schemas, schema_index);
}

static void* table_key_func(void *entry) {
    return get_table_name((table_t *)entry);
}

static size_t table_key_size_func(void *key) {
    return strlen((char *)key);
}

static void table_free_func(void *entry) {
    return table_destroy((table_t *)entry);
}

void table_set_init() {
    cq_table_set = my_malloc(sizeof(table_set_t));

    cq_table_set->table_set =
        hash_alloc(table_key_func,
                   table_key_size_func,
                   table_free_func);
    return;
}

ret_t table_set_add(table_t* ptable) {
    return hash_add(cq_table_set->table_set, ptable);
}

static table_t** table_set_get_all(hash_t* hash, size_t *number) {
    return (table_t **)hash_get_entries(hash, number);
}

table_t* table_set_get_random() {
    size_t table_num;
    table_t*  ptable = NULL;
    table_t** ptables = table_set_get_all(cq_table_set->table_set, &table_num);

    if (table_num > 0) {
        ptable = ptables[rand() % table_num];
    } else {
        log_debug("Table set is NULL!");
    }

    // low perf, will fix it later in hash.c
    my_free(ptables);

    return ptable;
}

void table_set_destroy() {
    hash_destroy(cq_table_set->table_set);
    my_free(cq_table_set);
    cq_table_set = NULL;
}
