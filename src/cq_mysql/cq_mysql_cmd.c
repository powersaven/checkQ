#include <string.h>
#include "table/cq_table.h"
#include "command/cq_command.h"
#include "server/cq_server.h"
#include "engine/cq_engine.h"
#include "engine/cq_stats.h"
#include "cq_mysql/cq_sql_gen.h"
#include "cq_mysql/cq_mysql_cmd.h"
#include "my_assert.h"
#include "lib/libmysql/include/mysql.h"
#include "lib/libmysql/include/errmsg.h"


char database[] = "test";

int mysql_create_table(eng_ctx_t* eng_ctx) {
    int ret = 0;

    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);
  
    ret = mysql_select_db(mysql, database); 
    if (ret != 0) {
        return ret;
    }

    sql_gen(eng_ctx, CREATE);   

    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        return 0;
    } else {
        log_error("%s", mysql_error(mysql)); 
        return ret;
    }
}

int mysql_alter_add_table(eng_ctx_t* eng_ctx) {
    int ret = 0;

    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);

    table_t        *ptable  = eng_ctx->table;
    table_schema_t *pschema = get_table_schema(ptable);

    ret = mysql_select_db(mysql, database); 
    if (ret != 0) {
        return ret;
    }

    if (is_table_schema_full(pschema)) {
        return 0;
    }

    add_table_data(pschema);
    sql_gen(eng_ctx, ALTER_ADD);

    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        return 0;
    } else {
        log_error("%s", mysql_error(mysql));
        
        return ret;
    }
}

int mysql_alter_drop_table(eng_ctx_t* eng_ctx) {    
    int ret = 0;
    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);

    int             ndata   = 0;
    table_t        *ptable  = eng_ctx->table;
    table_schema_t *pschema = get_table_schema(ptable);

    const table_data_t **table_datas = get_table_val_types(ptable, &ndata);
    if (ndata <= 0)
        return 0;
    
    ret = mysql_select_db(mysql, database); 
    if (ret != 0) {
        return ret;
    }

    char *data_name = get_table_data_name(table_datas[ndata - 1]);
    if (strncmp(data_name, "col", 3) != 0) {
        return 0;
    }

    sql_gen(eng_ctx, ALTER_DROP);   
    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        drop_table_data(pschema);
 
        return 0;
    } else {
        log_error("%s", mysql_error(mysql));
        return ret;
    }
}

int mysql_insert(eng_ctx_t* eng_ctx) {
    int ret = 0;
    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);

    ret = mysql_select_db(mysql, database);
    if (ret != 0) {
        return ret;
    }

    sql_gen(eng_ctx, INSERT);
    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        set_sql_write_time(eng_ctx);
        eng_ctx->prev_cmd = INSERT;
        eng_ctx->stats.ninsert++;
        g_stat.ninsert = __sync_add_and_fetch(&g_stat.ninsert, 1);

        return 0;
    } else {
        log_debug("%s", mysql_error(mysql));

        return ret;
    }
}

int mysql_delete(eng_ctx_t* eng_ctx) {
    int ret = 0;
    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);

    ret = mysql_select_db(mysql, database);
    if (ret != 0) {
        return ret;
    }

    sql_gen(eng_ctx, DELETE);
    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        set_sql_write_time(eng_ctx);
        eng_ctx->prev_cmd = DELETE;
        eng_ctx->stats.ndelete++;
        g_stat.ndelete = __sync_add_and_fetch(&g_stat.ndelete, 1);

        return 0;
    } else {
        log_debug("%s", mysql_error(mysql));

        return ret;
    }
}

int mysql_update(eng_ctx_t* eng_ctx) {
    int ret = 0;
    MYSQL* mysql = get_server_mysql(eng_ctx->write_server);

    ret = mysql_select_db(mysql, database);
    if (ret != 0) {
        return ret;
    }

    sql_gen(eng_ctx, UPDATE);
    my_assert(eng_ctx->sql);

    const char* sql = eng_ctx->sql;
    ret = mysql_real_query(mysql, sql, strlen(sql));

    if (0 == ret) {
        set_sql_write_time(eng_ctx);
        eng_ctx->prev_cmd = UPDATE;
        eng_ctx->stats.nupdate++;
        g_stat.nupdate = __sync_add_and_fetch(&g_stat.nupdate, 1);

        return 0;
    } else {
        log_debug("%s", mysql_error(mysql));

        return ret;
    }
}

int mysql_select(eng_ctx_t* eng_ctx) {
    int ret = 0;

    MYSQL* wmysql;
    MYSQL* rmysql;       

    MYSQL_RES* wmysql_result;  
    MYSQL_RES* rmysql_result;
   
    MYSQL_ROW wrow;
    MYSQL_ROW rrow;
    int wnum_fields;
    int rnum_fields;
    int wnum_rows;
    int rnum_rows;

    size_t* wlengths;
    size_t* rlengths;

    wmysql = get_server_mysql(eng_ctx->write_server); 
    ret = mysql_select_db(wmysql, database);
    if (ret != 0) {
        return ret;
    }

    sql_gen(eng_ctx, SELECT);
    my_assert(eng_ctx->sql);
    
    const char* sql = eng_ctx->sql;
    
    // get pre data value from write server
    ret = mysql_real_query(wmysql, sql, strlen(sql));
    if (ret != 0) {
        log_debug("%s", mysql_error(wmysql));
        return ret;
    }

    eng_ctx->stats.nselect++;       
    g_stat.nselect = __sync_add_and_fetch(&g_stat.nselect, 1);

    wmysql_result = mysql_store_result(wmysql);
    wnum_fields   = mysql_num_fields(wmysql_result);
    wnum_rows     = mysql_num_rows(wmysql_result);
    wrow          = mysql_fetch_row(wmysql_result);
    wlengths      = mysql_fetch_lengths(wmysql_result);

    if (0 == wnum_rows) {
        mysql_free_result(wmysql_result);
        return 1;       // this obj do not exist in mysql
    }

    // get data from other nodes    
    for (int i = 0; i < eng_ctx->nread_server; i++) {
        rmysql = get_server_mysql(eng_ctx->read_servers[i]);       
        ret = mysql_select_db(rmysql, database);

        if (ret != 0) {
            log_warn("can not connect to server:%s, db:%s", 
                get_server_name(eng_ctx->read_servers[i]), database);
            continue;
        }

        ret = mysql_real_query(rmysql, sql, strlen(sql));
        if (ret != 0) {
            log_debug("%s", mysql_error(rmysql));
            continue;
        }

        set_sql_read_time(eng_ctx);
        eng_ctx->stats.nselect++;       
        g_stat.nselect = __sync_add_and_fetch(&g_stat.nselect, 1);

        rmysql_result = mysql_store_result(rmysql);
        rnum_fields   = mysql_num_fields(rmysql_result);
        rnum_rows     = mysql_num_rows(rmysql_result);
        rrow          = mysql_fetch_row(rmysql_result);
        rlengths      = mysql_fetch_lengths(rmysql_result);

        if (rnum_rows > 0) {
            for (int j = 0; j < wnum_fields; j++) {
                if (strncmp(wrow[j], rrow[j], wlengths[j]) != 0) {

#ifdef READABLE_DATA_LOG
                    char* logbuf_offset = eng_ctx->data_inconsist_log_buf; 
                    
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Check ERROR !!!\n");
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Command: %s ", cmdtype_toString(eng_ctx->prev_cmd));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Host: %15s ", get_server_name(eng_ctx->write_server));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Table: %s ", get_table_name(eng_ctx->table));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Key: %.*s ", 20, wrow[0]);
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Value: %.*s ...\n", 8, wrow[1]);

                    logbuf_offset += sprintf(logbuf_offset, "Command: SELECT ");
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Host: %15s ", get_server_name(eng_ctx->read_servers[i]));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Table: %s ", get_table_name(eng_ctx->table));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Key: %.*s ", 20, rrow[0]);
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Value: %.*s ...\n\n", 8, rrow[1]);

                    *(logbuf_offset) = '\0';

                    fprintf(stderr, "%s", eng_ctx->data_inconsist_log_buf);
#else
                    /*
                    log_error("Data inconsistent detected!");
                    log_error("Write server is %s, read server is %s", 
                            get_server_name(eng_ctx->write_server), 
                            get_server_name(eng_ctx->read_servers[i]));
                    log_error("Last command is %s", 
                            cmdtype_toString(eng_ctx->prev_cmd));
                    log_error("There are %lld microseconds since last successful" 
                            "sql request", diff_sql_write_read_time(eng_ctx));

                    abort();
                    */
#endif                    
                    g_stat.ncheck_fail = __sync_add_and_fetch(&g_stat.ncheck_fail, 1);
                    abort();
                    
                }

            }
        } 
        else {
#ifdef READABLE_DATA_LOG
                    char* logbuf_offset = eng_ctx->data_inconsist_log_buf; 
                    
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Check ERROR !!!\n");
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Command: %s ", cmdtype_toString(eng_ctx->prev_cmd));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Host: %15s ", get_server_name(eng_ctx->write_server));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Table: %s ", get_table_name(eng_ctx->table));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Key: %.*s ", 20, wrow[0]);
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Value: %.*s ...\n", 8, wrow[1]);

                    logbuf_offset += sprintf(logbuf_offset, "Command: SELECT ");
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Host: %15s ", get_server_name(eng_ctx->read_servers[i]));
                    logbuf_offset += sprintf(logbuf_offset, 
                        "Table: %s ", get_table_name(eng_ctx->table));
                    logbuf_offset += sprintf(logbuf_offset, "Key: NULL ");
                    logbuf_offset += sprintf(logbuf_offset, "Value: NULL \n\n"); 

                    *(logbuf_offset) = '\0';

                    fprintf(stderr, "%s", eng_ctx->data_inconsist_log_buf);
#else                 
            /*       
            log_error("Data inconsistent detected!");
            log_error("Write server is %s, read server is %s", 
                    get_server_name(eng_ctx->write_server), 
                    get_server_name(eng_ctx->read_servers[i]));
            log_error("Last command is %s", 
                    cmdtype_toString(eng_ctx->prev_cmd));
            log_error("There are %lld microseconds since last successful" 
                    "sql request", diff_sql_write_read_time(eng_ctx));

            abort();
            */
#endif   
            g_stat.ncheck_fail = __sync_add_and_fetch(&g_stat.ncheck_fail, 1);         
            abort();
        }

        mysql_free_result(rmysql_result);
    }   
    
    mysql_free_result(wmysql_result);
    return 0;
}


