#include <stdlib.h>

#include "mylib.h"
#include "log.h"
#include "cq_command.h"
#include "vector.h"

struct cmd_bucket {
    vector_t*    cmd_lists;
};

typedef struct cmd_bucket cmd_bucket_t;

static cmd_bucket_t* cq_cmd_bucket = NULL;

cmd_t* cmd_init(cmd_type_t type) {
    cmd_t* pcmd = my_malloc(sizeof(cmd_t));

    pcmd->type = type;

    return pcmd;
}

void cmd_destroy(cmd_t* pcmd) {
    my_free(pcmd);
}

cmd_list_t* cmd_list_init() {
    cmd_list_t* pcmd_list = my_malloc(sizeof(cmd_list_t));

    pcmd_list->head = my_malloc(sizeof(cmd_t));
    pcmd_list->tail = my_malloc(sizeof(cmd_t));
    pcmd_list->ncmd = 0;

    pcmd_list->head->prev = NULL;
    pcmd_list->head->next = pcmd_list->tail;
    pcmd_list->tail->prev = pcmd_list->head;
    pcmd_list->tail->next = NULL;

    return pcmd_list;
}

void cmd_list_destroy(cmd_list_t* pcmd_list) {

    cmd_t* pcmd = pcmd_list->head;
    cmd_t* pcmd_tmp;

    while (pcmd->next) {
        pcmd_tmp = pcmd;
        pcmd = pcmd->next;

        cmd_destroy(pcmd_tmp);
    }
    cmd_destroy(pcmd);

    my_free(pcmd_list);
    pcmd_list = NULL;
}

ret_t cmd_list_add(cmd_list_t* pcmd_list, cmd_t* pcmd) {
    cmd_t* ptail = pcmd_list->tail;

    pcmd->prev = ptail->prev;
    pcmd->next = ptail;

    ptail->prev->next = pcmd;
    ptail->prev = pcmd;

    pcmd_list->ncmd++;

    return RET_SUCCEED;
}

ret_t cmd_list_add_head(cmd_list_t* pcmd_list, cmd_t* pcmd) {
    cmd_t* phead = pcmd_list->head;

    pcmd->prev = phead;
    pcmd->next = phead->next;

    phead->next->prev = pcmd;
    phead->next = pcmd;

    pcmd_list->ncmd++;

    return RET_SUCCEED;
}

cmd_t* get_first_cmd(cmd_list_t* pcmd_list) {
    return pcmd_list->head->next;
}

const char *cmd_toString(cmd_t *pcmd) {
    switch (pcmd->type) {
        case INSERT: return "INSERT";
        case DELETE: return "DELETE";
        case UPDATE: return "UPDATE";
        case SELECT: return "SELECT";
        case ALTER_ADD:  return "ALTER_ADD";
        case ALTER_DROP: return "ALTER_DROP";
        default: return "command not defined!";
    };
}

const char *cmdtype_toString(cmd_type_t type) {
    switch (type) {
        case INSERT: return "INSERT";
        case DELETE: return "DELETE";
        case UPDATE: return "UPDATE";
        case SELECT: return "SELECT";
        case ALTER_ADD:  return "ALTER_ADD";
        case ALTER_DROP: return "ALTER_DROP";
        default: return "command not defined!";
    };
}

void cmd_bucket_init() {
    cq_cmd_bucket = my_malloc(sizeof(cmd_bucket_t));
    cq_cmd_bucket->cmd_lists =
        vector_alloc((vector_entry_free_func)cmd_list_destroy);

    if (NULL == cq_cmd_bucket->cmd_lists) {
        log_error("init cmd vector failed!");
        abort();
    }

    return;
}

void cmd_bucket_destroy() {
    vector_destroy(cq_cmd_bucket->cmd_lists);
    my_free(cq_cmd_bucket);

    return;
}

ret_t cmd_bucket_add(cmd_list_t* pcmd_list) {
    return vector_push_back(cq_cmd_bucket->cmd_lists, pcmd_list);
}

cmd_list_t* get_random_cmd_list() {
    if (vector_isempty(cq_cmd_bucket->cmd_lists)) {
        log_error("command list is empty!");
        return NULL;
    }

    int cmd_index = rand() % vector_size(cq_cmd_bucket->cmd_lists);

    return vector_get(cq_cmd_bucket->cmd_lists, cmd_index);
}


