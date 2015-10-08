#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <parser/parser.h>

#include "mylib.h"
#include "log.h"
#include "server/cq_server.h"
#include "command/cq_command.h"
#include "table/cq_table.h"

#define MAX_PATH 1024

void dump_server(cq_server_t *cq_server);
void dump_cmd_list(cmd_list_t *my_cmd_list);
/*
 * As table_data, schema, table_schema's structure is not public, 
 * this part of code should be disabled. Before enable it, please 
 * change these structure's definition to public first.
 *
void dump_table_data(const table_data_t *my_table_data);
void dump_table_schema(table_schema_t *my_table_schema);
void dump_table(table_t *my_table);
 */

//#define TEST_XML

cq_server_cluster_t* cluster = NULL;

int main()
{
#ifdef TEST_XML
    char config_file[] = "../../../config/config.xml.example";
#else
    char config_file[] = "../../../config/config.txt.example";
#endif
    char cwd[MAX_PATH];
    char config_path[MAX_PATH * 2];
    
    /* generate config file's path */
    if (NULL == getcwd(cwd, MAX_PATH)) {
        log_error("Cannot get current work directiory");
        exit(1);
    }

    strcpy(config_path, cwd);
    strcat(config_path, "/");
    strcat(config_path, config_file);
    
    /* init data structure */
    cq_cluster_init(&cluster);
    cmd_bucket_init();
#ifdef TEST_XML
    schema_set_init();
#else
    table_set_init();
#endif

    parser_read_file(config_path);
    
    /* test server parse */
    cq_server_t** cq_servers_array;
    size_t read_server_num;

    cq_servers_array = cq_cluster_get_all_read_server(cluster, &read_server_num);

    for (int i = 0; i < read_server_num; i++) {
        dump_server(cq_servers_array[i]);
    }

    my_free(cq_servers_array);

    /* test command parse */
    cmd_list_t *my_cmd_list;

    my_cmd_list = get_random_cmd_list();
    dump_cmd_list(my_cmd_list);

#ifdef TEST_XML
    /*
     * As table_data, schema, table_schema's structure is not public, 
     * this part of code should be disabled. Before enable it, please 
     * change these structure's definition to public first.
     *
    table_schema_t *my_table_schema;
    my_table_schema = schema_set_get_random();
    
    dump_table_schema(my_table_schema);
     */
#else
    /*
     * As table_data, schema, table_schema's structure is not public, 
     * this part of code should be disabled. Before enable it, please 
     * change these structure's definition to public first.
     *
    table_t *my_table;

    my_table = table_set_get_random();
    dump_table(my_table);
     */
#endif

    cq_cluster_destroy(cluster);
    cmd_bucket_destroy();
#ifdef TEST_XML
    schema_set_destroy();
#else
    table_set_destroy();
#endif

    return 0;
}

void dump_server(cq_server_t *cq_server_p) {
    printf("\ncq_server {\n");
    if (NULL != cq_server_p) {
        printf("name\t=>\t%s\n", get_server_name(cq_server_p));
        printf("ip\t=>\t%s\n", get_server_ip(cq_server_p));
        printf("ip\t=>\t%d\n", get_server_port(cq_server_p));
    }
    printf("}\n");
}

void dump_cmd_list(cmd_list_t *my_cmd_list) {
    cmd_t *my_cmd;
    my_cmd = get_first_cmd(my_cmd_list);
    
    printf("\ncq_command_list {\n");
    while (my_cmd != my_cmd_list->tail) {
        printf("%s\n", cmd_toString(my_cmd));
        my_cmd = my_cmd->next;
    }
    printf("}\n");
}

/*
 * As table_data, schema, table_schema's structure is not public, 
 * this part of code should be disabled. Before enable it, please 
 * change these structure's to definition public first.
 *
void dump_table_data(const table_data_t *my_table_data) {
    printf("%s ", my_table_data->name);
    switch (my_table_data->type) {
        case BIGINT:
            printf("bigint ");
            break;
        case VARCHAR:
            printf("vachar ");
            break;
    }
    printf("%d\n", my_table_data->len);
}

void dump_table_schema(table_schema_t *my_table_schema) {
    schema_t *my_schema;

    printf("\ncq_table {\n");
    printf("key:\n");
    my_schema = my_table_schema->key_schema;
    if (NULL == my_schema) {
        log_error("Key schema not exists");
        return ;
    }
    printf("capacity\t=>\t%d\n", my_schema->capacity);
    printf("nschema\t=>\t%d\n", my_schema->nschema);
    for (int i = 0; i < my_schema->nschema; i ++) {
        dump_table_data(my_schema->schemas[i]);
    }
    printf("value:\n");
    my_schema = my_table_schema->val_schema;
    if (NULL == my_schema) {
        log_error("Value schema not exists");
        return ;
    }
    printf("capacity\t=>\t%d\n", my_schema->capacity);
    printf("nschema\t=>\t%d\n", my_schema->nschema);
    for (int i = 0; i < my_schema->nschema; i ++) {
        dump_table_data(my_schema->schemas[i]);
    }
    printf("}\n");
}

void dump_table(table_t *my_table) {
    const table_data_t **my_table_data_a;
    int num;

    printf("\ncq_table {\n");
    printf("name: %s\n", get_table_name(my_table));
    printf("key:\n");
    my_table_data_a = get_table_key_types(my_table, &num);
    for (int i = 0; i < num; i ++) {
        dump_table_data(my_table_data_a[i]);
    }
    printf("val:\n");
    my_table_data_a = get_table_val_types(my_table, &num);
    for (int i = 0; i < num; i++) {
        dump_table_data(my_table_data_a[i]);
    }
    printf("}\n");
}

 */
