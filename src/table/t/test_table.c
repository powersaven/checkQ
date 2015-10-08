#include <time.h>

#include "table/cq_table.h"
#include "log.h"

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

    char table_name[20];
    for (int i = 0; i < 5; i++) {
        ptable_schema = schema_set_get_random();
        for(int j = 0; j < 5; j++) {
            sprintf(table_name, "schema_%d_table_%d", i, j);
            ptable = table_init(table_name, ptable_schema);
            table_set_add(ptable);
        }
    }

    const table_data_t** pdata;
    int ndata;
    for (int i = 0; i < 100; i++) {
        ptable = table_set_get_random();

        log_info("Get table %s from table set", get_table_name(ptable));
        pdata = get_table_val_types(ptable, &ndata);

        for (int j = 0; j < ndata; j++) {
            log_info("From %s, data name is %s", get_table_name(ptable), get_table_data_name(pdata[j]));
            log_info("From %s, data type is %d", get_table_name(ptable), get_table_data_type(pdata[j]));
            log_info("From %s, data len is %d", get_table_name(ptable), get_table_data_len(pdata[j]));
        }
    }

    schema_set_destroy();
    table_set_destroy();

    return 0;
}
