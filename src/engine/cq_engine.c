#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "command/cq_command.h"
#include "server/cq_server.h"
#include "table/cq_table.h"
#include "engine/cq_engine.h"
#include "engine/cq_stats.h"
#include "cq_mysql/cq_mysql_cmd.h"

#include "log.h"
#include "mylib.h"
#include "vector.h"
#include "my_mutex.h"

#define MAX_THREAD_NUM      300
#define SQL_BUFFER_LEN      1048576
#define MAX_ENGINE_DATA_LEN 20
#define TABLE_THREAD_SLEEP  1200

my_lock_t total_request_lock;
long long total_request;

typedef struct engine_thrd_para {
    int tid;
} engine_para_t;

int engine_frandom_seed = 0;

eng_data_t* eng_data_init() {
    eng_data_t* eng_data = my_malloc(sizeof(eng_data_t));    
    return eng_data;
}

void eng_data_destroy(eng_data_t* eng_data) {
    my_free(eng_data);
}

eng_table_t* eng_table_init() {
    eng_table_t* eng_table = my_malloc(sizeof(eng_table_t));
    
//    eng_table->eng_datas = vector_alloc((vector_entry_free_func)eng_data_destroy);
    eng_table->eng_keys = vector_alloc((vector_entry_free_func)eng_data_destroy);
    return eng_table;
}

void eng_table_destroy(eng_table_t* eng_table) {
    my_free(eng_table);
}

time_t g_end_time = 3600;
time_t g_stat_interval;
int g_engine_thread_num = 0;
int g_alter_interval;
struct eng_stats g_stat;

static time_t duration = 0;
static time_t end_time = 0;

static bool is_thread_run = true;
static bool has_first_table = false;

static void* time_thread(void* p) {
    time_t curr_time  = 0;
    time_t start_time = time(NULL);
    end_time = g_end_time;

    while (duration < end_time) {
        sleep(1);

        curr_time = time(NULL);
        duration = curr_time - start_time;
    }

    is_thread_run = false;

    pthread_detach(pthread_self());

    return NULL;
}

static eng_ctx_t* eng_context_init(void* p) {
    eng_ctx_t* eng_ctx = my_malloc(sizeof(eng_ctx_t));
    memset(eng_ctx, 0, sizeof(eng_ctx_t));

    eng_ctx->sql = malloc(SQL_BUFFER_LEN);

    engine_para_t* para = (engine_para_t *)(p);
    eng_ctx->tid = para->tid;

    // dispatch cluster to all thread
    size_t nserver;
    cq_server_t *server;
    char *ip;
    int port;

    cq_cluster_init(&(eng_ctx->thread_cluster));

    cq_server_t** write_servers 
        = cq_cluster_get_all_write_server(cluster_base, &nserver);

    for (int i = 0; i < nserver; i++) {
        ip = get_server_ip(write_servers[i]);
        port = get_server_port(write_servers[i]);

        server = server_init(ip, port, WRITE_SERVER);
        if (RET_FAILED == cq_cluster_add(eng_ctx->thread_cluster, server)) {
            log_error("Failed to add server into thread cluster");
            abort();
        }
    }

    cq_server_t** read_servers 
        = cq_cluster_get_all_read_server(cluster_base, &nserver);

    for (int i = 0; i < nserver; i++) {
        ip = get_server_ip(read_servers[i]);
        port = get_server_port(read_servers[i]);

        server = server_init(ip, port, READ_SERVER);
        if (RET_FAILED == cq_cluster_add(eng_ctx->thread_cluster, server)) {
            log_error("Failed to add server into thread cluster");
            abort();
        }
    }

    memset(&(eng_ctx->stats), 0, sizeof(struct eng_stats));

    my_free(write_servers);
    my_free(read_servers);

    return eng_ctx;
}

static void eng_context_config(eng_ctx_t* eng_ctx) {
    eng_ctx->cmd_list = get_random_cmd_list();
    eng_ctx->write_server = cq_cluster_get_random_write_server(
        eng_ctx->thread_cluster);
 
    eng_ctx->write_servers 
        = cq_cluster_get_all_write_server(eng_ctx->thread_cluster, &(eng_ctx->nwrite_server));
    eng_ctx->read_servers 
        = cq_cluster_get_all_read_server(eng_ctx->thread_cluster, &(eng_ctx->nread_server));

    eng_ctx->table = table_set_get_random();
    eng_ctx->table_schema = schema_set_get_random();

    // generate a random table name for table thread
    sprintf(eng_ctx->table_name, "test%d", rand() % 10000); // magic number, uh
}

static void eng_context_destroy(eng_ctx_t* eng_ctx) {

    cq_cluster_destroy(eng_ctx->thread_cluster);
    my_free(eng_ctx->sql);
    my_free(eng_ctx);
}

static void eng_context_free_rwservers(eng_ctx_t* eng_ctx) {
    my_free(eng_ctx->write_servers);
    my_free(eng_ctx->read_servers);
}

static void set_seed(eng_ctx_t* eng_ctx) {
    eng_ctx->seed_base = __sync_add_and_fetch(&engine_frandom_seed, 1);   
    return;
}

static void travel_command_list(eng_ctx_t* eng_ctx) {
    cmd_list_t* cmd_list = eng_ctx->cmd_list;
    cmd_t* cmd = get_first_cmd(cmd_list);
    
    set_seed(eng_ctx);

    while (cmd != cmd_list->tail) {
        switch (cmd->type) {
        case INSERT:
            mysql_insert(eng_ctx);
            break;
        case DELETE:
            mysql_delete(eng_ctx);
            break;
        case UPDATE:
            mysql_update(eng_ctx);
            break;
        case SELECT:
            mysql_select(eng_ctx);
            break;
        default: break;
        }
        cmd = cmd->next;
    }

    return;
}

static void* engine_thread(void* p) {

    while(!has_first_table) {}

    eng_ctx_t* eng_ctx = eng_context_init(p);

    while (is_thread_run) {
        eng_context_config(eng_ctx);
        travel_command_list(eng_ctx);

        eng_context_free_rwservers(eng_ctx);
    }

    my_lock(&total_request_lock);
    total_request += eng_ctx->stats.ninsert;
    total_request += eng_ctx->stats.nupdate;
    total_request += eng_ctx->stats.ndelete;
    total_request += eng_ctx->stats.nselect;
    my_unlock(&total_request_lock);

    eng_context_destroy(eng_ctx);

    return NULL;
}

static void* table_thread(void* p) {
        
    eng_ctx_t* eng_ctx = eng_context_init(p);
    while (is_thread_run) {
        eng_context_config(eng_ctx);
        eng_ctx->table = p;  // FIXME, to avoid abort in mysql command       
     
        table_t* ptable;
        int ret = mysql_create_table(eng_ctx);
        if (0 == ret) {
            ptable = table_init(eng_ctx->table_name, eng_ctx->table_schema);
            table_set_add(ptable);
            has_first_table = true;
        } 
        eng_context_free_rwservers(eng_ctx);

        time_t remaining_time = end_time - duration;
        if (remaining_time < TABLE_THREAD_SLEEP) {
            sleep(remaining_time);
        } else {
            sleep(TABLE_THREAD_SLEEP);
        }
    }
   
    eng_context_destroy(eng_ctx);

    return NULL;
}

static void* alter_thread(void *p) {
    while (!has_first_table) {}
    
    eng_ctx_t* eng_ctx = eng_context_init(p);
    
    while (is_thread_run) {
        eng_context_config(eng_ctx);
        int op = rand() % ALTER_OP_NUM;
        int ret = 0;
    
        if (ALTER_OP_ADD == op) {
            ret = mysql_alter_add_table(eng_ctx);
        } else if (ALTER_OP_DROP == op) {
            ret = mysql_alter_drop_table(eng_ctx);
        }

        if (0 != ret) {
            log_warn("alter table failed!");
        //    exit(EXIT_FAILURE);
        }

        eng_context_free_rwservers(eng_ctx);

        time_t remaining_time = end_time - duration;
        if (remaining_time < g_alter_interval) {
            sleep(remaining_time);
        } else {
            sleep(g_alter_interval);
        }
    }
    eng_context_destroy(eng_ctx);
    
    return NULL;
}

static void* stat_thread(void* p) {

    struct timeval tv_start;
    struct timeval tv_end;
    struct timezone tz_start;
    struct timezone tz_end;
    
    gettimeofday(&tv_start, &tz_start);

    char stat_log[MAX_DATA_LOG_LEN];
    long long time_passed, total_reqs;

    while (is_thread_run) {

        gettimeofday(&tv_end, &tz_end);
        
        time_passed = (tv_end.tv_sec - tv_start.tv_sec) * 1LL;
        total_reqs = g_stat.ninsert + g_stat.nupdate + g_stat.ndelete + g_stat.nselect;

        char *log_offset = stat_log;
        log_offset += sprintf(log_offset, "%-8s %-8s %-8s\n",   "Type", "Ops", "TPS(ops/s)");
        log_offset += sprintf(log_offset, "%-8s %-8d %-8lld\n", "Insert", g_stat.ninsert, time_passed == 0 ? 0 : 1LL * g_stat.ninsert / time_passed);
        log_offset += sprintf(log_offset, "%-8s %-8d %-8lld\n", "Update", g_stat.nupdate, time_passed == 0 ? 0 : 1LL * g_stat.nupdate / time_passed);
        log_offset += sprintf(log_offset, "%-8s %-8d %-8lld\n", "Delete", g_stat.ndelete, time_passed == 0 ? 0 : 1LL * g_stat.ndelete / time_passed);
        log_offset += sprintf(log_offset, "%-8s %-8d %-8lld\n", "Select", g_stat.nselect, time_passed == 0 ? 0 : 1LL * g_stat.nselect / time_passed);
        log_offset += sprintf(log_offset, "%-8s %-8lld %-8lld\n", "Total", total_reqs, time_passed == 0 ? 0 : 1LL * total_reqs / time_passed);
        log_offset += sprintf(log_offset, "Data verify errors: %d\n", g_stat.ncheck_fail);

        *log_offset = '\0';

        fprintf(stderr, "%s\n", stat_log);

        sleep(g_stat_interval);
    }
   
    return NULL;
}

void start_engine() {
    pthread_t     alter_thread_t;
    pthread_t     time_thread_t;
    pthread_t     engine_thread_t[MAX_THREAD_NUM];
    pthread_t     table_thread_t;
    pthread_t     stat_thread_t;
    engine_para_t engine_para[MAX_THREAD_NUM];
    engine_para_t table_para;
    engine_para_t alter_para;
    int           engine_thread_num;

    if (pthread_create(&table_thread_t, NULL, table_thread, &table_para) != 0) {
        log_error("can not create table thread!");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&alter_thread_t, NULL, alter_thread, &alter_para) != 0) {
        log_error("can not create alter thread!");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_create(&time_thread_t, NULL, time_thread, NULL) != 0) {
        log_error("can not create time thread!");
        exit(EXIT_FAILURE);
    }

    engine_thread_num = g_engine_thread_num > MAX_THREAD_NUM ? MAX_THREAD_NUM
            : g_engine_thread_num;

    for (int i = 0; i < engine_thread_num; i++) {
        engine_para[i].tid = i;
        if(pthread_create(&engine_thread_t[i], NULL, engine_thread,
                &engine_para[i]) != 0) {
            log_error("can not create engine thread!");
            exit(EXIT_FAILURE);
        }
    }

    if (pthread_create(&stat_thread_t, NULL, stat_thread, NULL) != 0) {
        log_error("can not create stat thread!");
        exit(EXIT_FAILURE);
    }

    pthread_join(alter_thread_t, NULL);
    pthread_join(table_thread_t, NULL);
    pthread_join(stat_thread_t, NULL);
    for (int i = 0; i < engine_thread_num; i++) {
        pthread_join(engine_thread_t[i], NULL);
    }

}


