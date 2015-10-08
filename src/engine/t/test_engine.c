#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "command/cq_command.h"
#include "server/cq_server.h"
#include "table/cq_table.h"
#include "engine/cq_engine.h"
#include "cq_mysql/cq_mysql_cmd.h"

#include "log.h"
#include "mylib.h"
#include "frandom.h"

cq_server_cluster_t* cluster = NULL;

void init_cluster() {
    cq_server_t* pserver;

    char ip_tmp[16];
    int port_tmp;

    cq_cluster_init(&cluster);
    for (int i = 0; i < 100; i++) {
        string_frandom(ip_tmp, 15, i);
        ip_tmp[15] = '\0';
        port_tmp = frandom(i, 0) % 65536;

        pserver = server_init(ip_tmp, port_tmp, WRITE_SERVER);
        cq_cluster_add(cluster, pserver);
    }
}

void destroy_cluster() {
    cq_cluster_destroy(cluster);
}

void init_command_bucket() {
    // init command list
    cmd_t* pcmda0 = cmd_init(INSERT);
    cmd_t* pcmda1 = cmd_init(INSERT);

    cmd_t* pcmdb0 = cmd_init(DELETE);
    cmd_t* pcmdb1 = cmd_init(DELETE);

    cmd_t* pcmdc0 = cmd_init(UPDATE);
    cmd_t* pcmdc1 = cmd_init(UPDATE);

    cmd_t* pcmdd0 = cmd_init(SELECT);
    cmd_t* pcmdd1 = cmd_init(SELECT);

    cmd_list_t* pcmd_lista = cmd_list_init();
    cmd_list_t* pcmd_listb = cmd_list_init();
    cmd_list_t* pcmd_listc = cmd_list_init();
    cmd_list_t* pcmd_listd = cmd_list_init();

    cmd_list_add(pcmd_lista, pcmda0);
    cmd_list_add(pcmd_lista, pcmda1);
    cmd_list_add(pcmd_listb, pcmdb0);
    cmd_list_add(pcmd_listb, pcmdb1);
    cmd_list_add(pcmd_listc, pcmdc0);
    cmd_list_add(pcmd_listc, pcmdc1);
    cmd_list_add(pcmd_listd, pcmdd0);
    cmd_list_add(pcmd_listd, pcmdd1);

    cmd_bucket_init();
    cmd_bucket_add(pcmd_lista);
    cmd_bucket_add(pcmd_listb);
    cmd_bucket_add(pcmd_listc);
    cmd_bucket_add(pcmd_listd);
}

void destroy_command_bucket() {
    cmd_bucket_destroy();
}

void init_table_set() {
    table_data_t*   ptable_data;
    schema_t*       pschema_key;
    schema_t*       pschema_val;
    table_schema_t* ptable_schema;
    table_t*        ptable;

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
}

void destroy_table_set() {
    schema_set_destroy();
    table_set_destroy();
}

int main(int argc, char* argv[]) {
    srand((int) (time(0)));
    init_cluster();
    init_command_bucket();
    init_table_set();

    g_end_time = 30;
    g_engine_thread_num = 10;

    start_engine();

    destroy_cluster();
    destroy_command_bucket();
    destroy_table_set();

    return 0;
}
