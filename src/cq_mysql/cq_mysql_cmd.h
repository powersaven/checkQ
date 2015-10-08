#ifndef CQ_MYSQL_CMD_H
#define CQ_MYSQL_CMD_H

#include "server/cq_server.h"
#include "table/cq_table.h"
#include "command/cq_command.h"
#include "engine/cq_engine.h"

int mysql_create_table(eng_ctx_t* eng_ctx);
int mysql_alter_table(eng_ctx_t* eng_ctx);
int mysql_alter_add_table(eng_ctx_t* eng_ctx);
int mysql_alter_drop_table(eng_ctx_t* eng_ctx);

int mysql_insert(eng_ctx_t* eng_ctx);
int mysql_delete(eng_ctx_t* eng_ctx);
int mysql_update(eng_ctx_t* eng_ctx);
int mysql_select(eng_ctx_t* eng_ctx);

#endif
