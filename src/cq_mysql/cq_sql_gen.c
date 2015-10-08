#include <stdio.h>
#include <inttypes.h>

#include "table/cq_table.h"
#include "command/cq_command.h"
#include "engine/cq_engine.h"
#include "return.h"
#include "frandom.h"
#include "log.h"
#include "my_assert.h"
#include "my_bool.h"

int sql_frandom_seed = 0;

static inline char* write_bigint(eng_ctx_t* eng_ctx, char* sql_buf, 
    const table_data_t* table_data, bool is_key) {
    int    tid = eng_ctx->tid;
    int offset = 0;
    int   seed;

    if (is_key) {
        seed = eng_ctx->seed_base;
    } else {
        seed = __sync_add_and_fetch(&sql_frandom_seed, 1);
    }

    offset = sprintf(sql_buf, "%" PRIu64, frandom(seed, tid)>>1);

    return sql_buf+offset;
}

static inline char* write_varchar(eng_ctx_t* eng_ctx, char* sql_buf,
    const table_data_t* table_data, bool is_key) {
    int  tid = eng_ctx->tid;
    int  len; 
    int seed;
    
    if (is_key) {
        seed = eng_ctx->seed_base;
        len = frandom(seed, tid) % get_table_data_len(table_data) + 1;
    } else {
        seed = __sync_add_and_fetch(&sql_frandom_seed, 1);
        len = rand() % get_table_data_len(table_data) + 1;
    }

    string_frandom(sql_buf, len, seed);

    return sql_buf+len;
}

static ret_t insert_gen(eng_ctx_t* eng_ctx) {
    table_t* table      = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "insert into %s values(", get_table_name(table));

    // generate key
    table_datas = get_table_key_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], true);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], true);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }

    // generate val
    table_datas = get_table_val_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have val schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], false);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], false);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }

    sql_offset--;
    *sql_offset++ = ')';
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t delete_gen(eng_ctx_t* eng_ctx) {
    // delete from %s where %s='%d$%.*s'
    table_t* table      = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "delete from %s where ", get_table_name(table));

    table_datas = get_table_key_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < 1; i++) {
        sql_offset += sprintf(sql_offset, "%s=",
                get_table_data_name(table_datas[i]));
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], true);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], true);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }
    sql_offset--;
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t update_gen(eng_ctx_t* eng_ctx) {
    table_t* table      = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "update %s set ", get_table_name(table));

    // generate val
    table_datas = get_table_val_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have val schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        sql_offset += sprintf(sql_offset, "%s=",
                get_table_data_name(table_datas[i]));
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], false);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], false);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }
    sql_offset--;
    sql_offset += sprintf(sql_offset, " where ");

    // generate key
    table_datas = get_table_key_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < 1; i++) {
        sql_offset += sprintf(sql_offset, "%s=",
                get_table_data_name(table_datas[i]));
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], true);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], true);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }
    sql_offset--;
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t select_gen(eng_ctx_t* eng_ctx) {
//    select %s from %s where %s='%d$%.*s'
    table_t* table      = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_keys;
    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "select ");

    // generate key in sql
    table_keys = get_table_key_types(table, &ndata);
    if (NULL == table_keys) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        sql_offset += sprintf(sql_offset, "%s,",
            get_table_data_name(table_keys[i]));
    }

    // generate val
    table_datas = get_table_val_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have val schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        sql_offset += sprintf(sql_offset, "%s,",
                get_table_data_name(table_datas[i]));
    }
    sql_offset--;
    sql_offset += sprintf(sql_offset, " from %s where ", get_table_name(table));

    // generate key
    table_datas = get_table_key_types(table, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < 1; i++) {
        sql_offset += sprintf(sql_offset, "%s=",
                get_table_data_name(table_datas[i]));
        *sql_offset++ = '\'';
        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset = write_bigint(eng_ctx, sql_offset, table_datas[i], true);
        } else {
            sql_offset = write_varchar(eng_ctx, sql_offset, table_datas[i], true);
        }
        *sql_offset++ = '\'';
        *sql_offset++ = ',';
    }
    sql_offset--;
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t create_gen(eng_ctx_t* eng_ctx) {
    table_schema_t* schema = eng_ctx->table_schema;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    // FIXME, table name shall be random
    sql_offset += sprintf(sql, "create table %s (", eng_ctx->table_name);

    // generate key
    table_datas = get_schema_key_types(schema, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have key schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        sql_offset += sprintf(sql_offset, "%s ",
                        get_table_data_name(table_datas[i]));

        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset += sprintf(sql_offset, "bigint primary key");
        } else {
            sql_offset += sprintf(sql_offset, "varchar(%d) primary key", get_table_data_len(table_datas[i]));
        }
        *sql_offset++ = ',';
    }

    // generate val
    table_datas = get_schema_val_types(schema, &ndata);
    if (NULL == table_datas) {
        log_error("Table do not have val schema!");
        return RET_FAILED;
    }

    for (int i = 0; i < ndata; i++) {
        sql_offset += sprintf(sql_offset, "%s ",
                        get_table_data_name(table_datas[i]));

        if (BIGINT == get_table_data_type(table_datas[i])) {
            sql_offset += sprintf(sql_offset, "bigint");
        } else {
            sql_offset += sprintf(sql_offset, "varchar(%d)", get_table_data_len(table_datas[i]));
        }
        *sql_offset++ = ',';
    }

    sql_offset--;
    *sql_offset++ = ')';
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t alter_add_gen(eng_ctx_t* eng_ctx) {
    table_t* ptable     = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "alter table %s ", get_table_name(ptable));

    // generate val
    table_datas = get_table_val_types(ptable, &ndata);
            
    const table_data_t* ptable_data = table_datas[ndata - 1]; 
    
    if (BIGINT == get_table_data_type(ptable_data)) {
        sql_offset += sprintf(sql_offset, "add %s bigint",
            get_table_data_name(ptable_data));    
    } else {
        sql_offset += sprintf(sql_offset, "add %s varchar(%d)",
            get_table_data_name(ptable_data), get_table_data_len(ptable_data));
    }
       
    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t alter_drop_gen(eng_ctx_t* eng_ctx) {
    table_t* ptable     = eng_ctx->table;
    char*    sql        = eng_ctx->sql;
    int      ndata      = 0;
    char*    sql_offset = sql;

    const table_data_t** table_datas;

    sql_offset += sprintf(sql, "alter table %s ", get_table_name(ptable));

    // generate val
    table_datas = get_table_val_types(ptable, &ndata);
            
    if (ndata > 0) {
        int idx = ndata - 1;
        sql_offset += sprintf(sql_offset, "DROP COLUMN %s",
             get_table_data_name(table_datas[idx]));
    }

    *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

static ret_t drop_gen(eng_ctx_t* eng_ctx) { 
    char*    sql        = eng_ctx->sql;
    char*    sql_offset = sql;

    sql_offset += sprintf(sql, "drop table %s", eng_ctx->table_name);
   *sql_offset++ = '\0';

    log_debug("%s", sql);

    return RET_SUCCEED;
}

ret_t sql_gen(eng_ctx_t* eng_ctx, cmd_type_t cmd_type) {
    
    my_assert(eng_ctx->table != NULL);
    my_assert(eng_ctx->sql != NULL);

    switch (cmd_type) {
    case INSERT: return insert_gen(eng_ctx);
    case DELETE: return delete_gen(eng_ctx);
    case UPDATE: return update_gen(eng_ctx);
    case SELECT: return select_gen(eng_ctx);
    case CREATE: return create_gen(eng_ctx);
    case ALTER_ADD:  return alter_add_gen(eng_ctx);
    case ALTER_DROP: return alter_drop_gen(eng_ctx);
    case DROP:   return drop_gen(eng_ctx);
    default: return RET_FAILED;
    }
}
