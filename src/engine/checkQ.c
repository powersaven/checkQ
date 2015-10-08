#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "command/cq_command.h"
#include "server/cq_server.h"
#include "table/cq_table.h"
#include "engine/cq_engine.h"
#include "engine/cq_stats.h"
#include "cq_mysql/cq_mysql_cmd.h"
#include "parser/parser.h"

#include "checkQ.h"

#include "options.h"
#include "options_impl.h"
#include "log.h"
#include "mylib.h"
#include "sigsegv.h"

#include "lib/libmysql/include/mysql.h"

void init_env() {

    setup_sigsegv();

    srand((int) (time(0)));

    g_end_time = options->end_time;
    g_engine_thread_num = options->thread_num;
    g_stat_interval = options->stats_intval;
    g_alter_interval = options->alter_intval;
    memset(&g_stat, 0, sizeof(struct eng_stats));

    strncpy(g_username, options->username, strlen(options->username));
    strncpy(g_password, options->password, strlen(options->password));

    if (mysql_library_init(0, NULL, NULL)) {
        log_error("Could not initialize MySQL library");
        exit(EXIT_FAILURE);
    }   

    cq_cluster_init(&cluster_base);
    cmd_bucket_init();
    schema_set_init();
    table_set_init();

    parser_read_file((const char *)options->config);

    return;
}

void summary() {
    log_info("Total request is %lld", total_request);
    log_info("TPS is %lld/sec", total_request / g_end_time);
}

void clean_env() {
    cq_cluster_destroy(cluster_base);
    cmd_bucket_destroy();
    schema_set_destroy();
    table_set_destroy();

    // mysql_library_end();

    return;
}

/*
 * Main function for CheckQ
 */
int main(int argc, char* argv[]) {

    options_parse(argc, argv);

    init_env();

    start_engine();

    summary();

    clean_env();

    return 0;
}
