#ifndef CQ_TABLE_H
#define CQ_TABLE_H

#include "return.h"

typedef enum {
    BIGINT,
    VARCHAR,
} table_data_type_t;

typedef struct table_data     table_data_t;
typedef struct schema         schema_t;
typedef struct table_schema   table_schema_t;
typedef struct table          table_t;

// table data fuction
table_data_t*     table_data_init(char* name, table_data_type_t type, int len);
void              table_data_destroy(table_data_t* ptable_data);
char*             get_table_data_name(const table_data_t* ptable_data);
table_data_type_t get_table_data_type(const table_data_t* ptable_data);
int               get_table_data_len(const table_data_t* ptable_data);

// schema function
schema_t* schema_init(void);
void      schema_destroy(schema_t* pschema);
ret_t     schema_add(schema_t* pschema, table_data_t* ptable_data);

// table schema function
table_schema_t* table_schema_init(schema_t* key_schema, schema_t* val_schema);
void            table_schema_destroy(table_schema_t* ptable_schema);
const table_data_t** get_schema_key_types(table_schema_t* schema, int* nschema);
const table_data_t** get_schema_val_types(table_schema_t* schema, int* nschema);

// table function
table_t* table_init(char* name, table_schema_t* ptable_schema);
void     table_destroy(table_t* ptable);
char*    get_table_name(table_t* ptable);
const table_data_t** get_table_key_types(table_t* ptable, int* nschema);
const table_data_t** get_table_val_types(table_t* ptable, int* nschema);
table_schema_t*      get_table_schema(table_t *ptable);

// schema set function
void            schema_set_init(void);
void            schema_set_destroy();
ret_t           schema_set_add(table_schema_t* ptable_schema);
table_schema_t* schema_set_get_random();

// table set function
void     table_set_init();
void     table_set_destroy();
ret_t    table_set_add(table_t* ptable);
table_t* table_set_get_random();

// schema get function
schema_t* get_table_key_schema(table_schema_t *pschema);
schema_t* get_table_val_schema(table_schema_t *pschema);
void      drop_table_data(table_schema_t *pschema);
void      add_table_data(table_schema_t *pschema);
int       is_table_schema_full(table_schema_t *pschema);

#endif
