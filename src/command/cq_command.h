#ifndef CQ_COMMAND_H
#define CQ_COMMAND_H

#include "return.h"

typedef struct cmd        cmd_t;
typedef struct cmd_list   cmd_list_t;

typedef enum {
    INSERT = 1,
    DELETE,
    UPDATE,
    SELECT,
    CREATE,
    ALTER_ADD,
    ALTER_DROP,
    DROP,
} cmd_type_t;

typedef enum {
    ALTER_OP_ADD = 0,
    ALTER_OP_DROP,
    ALTER_OP_NUM,
} alter_type_t;

struct cmd {
    cmd_type_t type;
    cmd_t*     next;
    cmd_t*     prev;
};

struct cmd_list {
    cmd_t* head;
    cmd_t* tail;
    int    ncmd;
};

cmd_t* cmd_init(cmd_type_t type);
void cmd_destroy(cmd_t* pcmd);
const char *cmd_toString(cmd_t *pcmd);
const char *cmdtype_toString(cmd_type_t type);

cmd_list_t* cmd_list_init();
void cmd_list_destroy(cmd_list_t* pcmd_list);
ret_t cmd_list_add(cmd_list_t* pcmd_list, cmd_t* pcmd);
ret_t cmd_list_add_head(cmd_list_t* pcmd_list, cmd_t* pcmd);
cmd_t* get_first_cmd(cmd_list_t* pcmd_list);

void cmd_bucket_init(void);
void cmd_bucket_destroy();
ret_t cmd_bucket_add(cmd_list_t* pcmd_list);
cmd_list_t* get_random_cmd_list();

#endif
