#ifndef CQ_ENGINE_H
#define CQ_ENGINE_H

#include <stdint.h>
#include "command/cq_command.h"
#include "server/cq_server.h"
#include "table/cq_table.h"
#include "vector.h"

#define MAX_NAME_LEN 100
#define MAX_SCHEMA   100

#define MAX_ENGINE_DATA_LEN 20
#define MAX_TABLE_NAME_LEN  20
#define MAX_DATA_LOG_LEN    2048


typedef struct engine_context  eng_ctx_t;
typedef struct engine_pre_data pre_data_t;

// scroll array
typedef struct eng_table {
    vector_t* eng_keys;
} eng_table_t;

typedef struct eng_data {
    char              name[MAX_ENGINE_DATA_LEN];
    table_data_type_t type;
    uint64_t          seed;
    int               len;
} eng_data_t;

struct engine_pre_data {
    char              name[MAX_NAME_LEN];
    table_data_type_t type;

    // for bigint
    uint64_t          key;

    // for varchar
    uint64_t          seed;
    int               len;
};

#include "engine/cq_stats.h"

struct engine_context {
    cmd_list_t*          cmd_list;
    cq_server_t*         write_server;

    cq_server_cluster_t* thread_cluster;
    cq_server_t**        write_servers;
    cq_server_t**        read_servers;
    size_t               nread_server;
    size_t               nwrite_server;
    table_t*             table;
    table_schema_t*      table_schema;
    char                 table_name[MAX_TABLE_NAME_LEN];

    int                  tid;
    char*                sql;
    int                  seed_base;
    
    cmd_type_t           prev_cmd;
    struct eng_stats     stats;

#ifdef READABLE_DATA_LOG
    char data_inconsist_log_buf[MAX_DATA_LOG_LEN];
#endif    
};

extern time_t g_end_time;
extern int g_engine_thread_num;
extern long long total_request;
extern time_t g_stat_interval;
extern struct eng_stats g_stat;
extern int g_alter_interval;

void start_engine();

#endif
