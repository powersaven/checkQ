#ifndef CQ_SQL_GEN_H
#define CQ_SQL_GEN_H

#include "table/cq_table.h"
#include "command/cq_command.h"
#include "engine/cq_engine.h"
#include "return.h"


ret_t sql_gen(eng_ctx_t* eng_ctx, cmd_type_t cmd_type);

#endif
