#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "table/cq_table.h"
#include "command/cq_command.h"
#include "cq_mysql/cq_sql_gen.h"
#include "log.h"
#include "mylib.h"

int main(int argc, char* argv[]) {

    table_data_t*   ptable_data;
    schema_t*       pschema_key;
    schema_t*       pschema_val;
    table_schema_t* ptable_schema;
    table_t*        ptable;

    srand((int)(time(0)));
    schema_set_init();
    table_set_init();

    // loop begin
    pschema_key = schema_init();
    pschema_val = schema_init();

    ptable_data = table_data_init("col1", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("col2", VARCHAR, 10);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col3", VARCHAR, 11);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col4", VARCHAR, 12);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col5", VARCHAR, 13);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col6", VARCHAR, 14);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col7", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_schema = table_schema_init(pschema_key, pschema_val);

    schema_set_add(ptable_schema);
    // loop end

    // loop begin
    pschema_key = schema_init();
    pschema_val = schema_init();

    ptable_data = table_data_init("t2col1", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("t2col2", VARCHAR, 10);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col3", VARCHAR, 11);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col4", VARCHAR, 12);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col5", VARCHAR, 13);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col6", VARCHAR, 14);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col7", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("t2col8", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_schema = table_schema_init(pschema_key, pschema_val);

    schema_set_add(ptable_schema);
    // loop end

    // loop begin
    pschema_key = schema_init();
    pschema_val = schema_init();

    ptable_data = table_data_init("col1", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("col2", VARCHAR, 10);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col3", VARCHAR, 11);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col4", VARCHAR, 12);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col5", VARCHAR, 13);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col6", VARCHAR, 14);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col7", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_schema = table_schema_init(pschema_key, pschema_val);

    schema_set_add(ptable_schema);
    // loop end

    // loop begin
    pschema_key = schema_init();
    pschema_val = schema_init();

    ptable_data = table_data_init("col1", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("col2", VARCHAR, 10);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col3", VARCHAR, 11);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col4", VARCHAR, 12);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col5", VARCHAR, 13);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col6", VARCHAR, 14);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col7", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_schema = table_schema_init(pschema_key, pschema_val);

    schema_set_add(ptable_schema);
    // loop end

    // loop begin
    pschema_key = schema_init();
    pschema_val = schema_init();

    ptable_data = table_data_init("col1", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("col2", BIGINT, 0);
    schema_add(pschema_key, ptable_data);

    ptable_data = table_data_init("col3", VARCHAR, 11);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col4", VARCHAR, 12);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col5", VARCHAR, 13);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col6", VARCHAR, 14);
    schema_add(pschema_val, ptable_data);

    ptable_data = table_data_init("col7", VARCHAR, 15);
    schema_add(pschema_val, ptable_data);

    ptable_schema = table_schema_init(pschema_key, pschema_val);

    schema_set_add(ptable_schema);
    // loop end

    ptable = table_init("test_mysql_cmd", ptable_schema);
    table_set_add(ptable);

    char* sql = my_malloc((size_t)1024000);
    memset(sql, 0, sizeof(sql));

    eng_ctx_t* eng_ctx = my_malloc(sizeof(eng_ctx_t));
    memset(eng_ctx, 0, sizeof(eng_ctx_t));
    eng_ctx->sql = sql;
    eng_ctx->table = ptable;
    eng_ctx->table_schema = ptable_schema;

    sql_gen(eng_ctx, INSERT);
    sql_gen(eng_ctx, UPDATE);
    sql_gen(eng_ctx, DELETE);
    sql_gen(eng_ctx, SELECT);
    sql_gen(eng_ctx, CREATE);

    my_free(sql);
    my_free(eng_ctx);

    schema_set_destroy();
    table_set_destroy();

    return 0;
}
