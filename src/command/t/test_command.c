#include <time.h>

#include "log.h"
#include "command/cq_command.h"

int main(int argc, char* argv[]) {

    srand((int) (time(0)));

    // test cmd
    cmd_t* pcmd0 = cmd_init(INSERT);
    cmd_t* pcmd1 = cmd_init(DELETE);
    cmd_t* pcmd2 = cmd_init(UPDATE);
    cmd_t* pcmd3 = cmd_init(SELECT);
    cmd_t* pcmd4 = cmd_init(INSERT);
    cmd_t* pcmd5 = cmd_init(DELETE);
    cmd_t* pcmd6 = cmd_init(UPDATE);
    cmd_t* pcmd7 = cmd_init(SELECT);

    // test cmd_list
    cmd_list_t* pcmd_list = cmd_list_init();

    cmd_list_add(pcmd_list, pcmd0);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add(pcmd_list, pcmd1);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add(pcmd_list, pcmd2);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add(pcmd_list, pcmd3);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_t* pcmd = get_first_cmd(pcmd_list);

    while (pcmd != pcmd_list->tail) {
        log_info("type:%s", cmd_toString(pcmd));
        pcmd = pcmd->next;
    }

    cmd_list_add_head(pcmd_list, pcmd4);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add_head(pcmd_list, pcmd5);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add_head(pcmd_list, pcmd6);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    cmd_list_add_head(pcmd_list, pcmd7);
    log_info("%d cmd in cmdlist", pcmd_list->ncmd);

    pcmd = get_first_cmd(pcmd_list);

    while (pcmd != pcmd_list->tail) {
        log_info("type:%s", cmd_toString(pcmd));
        pcmd = pcmd->next;
    }

    // cmd_list_destroy(pcmd_list);

    // test cmd_bucket
    cmd_bucket_init();
    cmd_bucket_add(pcmd_list);
    cmd_bucket_destroy();

    // test_get random cmd_list
    cmd_t* pcmda0 = cmd_init(INSERT);
    cmd_t* pcmda1 = cmd_init(INSERT);

    cmd_t* pcmdb0 = cmd_init(DELETE);
    cmd_t* pcmdb1 = cmd_init(DELETE);

    cmd_t* pcmdc0 = cmd_init(UPDATE);
    cmd_t* pcmdc1 = cmd_init(UPDATE);

    cmd_t* pcmdd0 = cmd_init(SELECT);
    cmd_t* pcmdd1 = cmd_init(SELECT);

    cmd_list_t* pcmd_lista = cmd_list_init();
    cmd_list_t* pcmd_listb = cmd_list_init();
    cmd_list_t* pcmd_listc = cmd_list_init();
    cmd_list_t* pcmd_listd = cmd_list_init();

    cmd_list_add(pcmd_lista, pcmda0);
    cmd_list_add(pcmd_lista, pcmda1);
    cmd_list_add(pcmd_listb, pcmdb0);
    cmd_list_add(pcmd_listb, pcmdb1);
    cmd_list_add(pcmd_listc, pcmdc0);
    cmd_list_add(pcmd_listc, pcmdc1);
    cmd_list_add(pcmd_listd, pcmdd0);
    cmd_list_add(pcmd_listd, pcmdd1);

    cmd_bucket_init();
    cmd_bucket_add(pcmd_lista);
    cmd_bucket_add(pcmd_listb);
    cmd_bucket_add(pcmd_listc);
    cmd_bucket_add(pcmd_listd);

    // get 1st
    cmd_list_t* pcmd_list1 = get_random_cmd_list();
    cmd_t* pcmdt = get_first_cmd(pcmd_list1);

    while (pcmdt != pcmd_list1->tail) {
        log_info("type:%s", cmd_toString(pcmdt));
        pcmdt = pcmdt->next;
    }

    // get 2nd
    pcmd_list1 = get_random_cmd_list();
    pcmdt = get_first_cmd(pcmd_list1);

    while (pcmdt != pcmd_list1->tail) {
        log_info("type:%s", cmd_toString(pcmdt));
        pcmdt = pcmdt->next;
    }

    cmd_bucket_destroy();

    return 0;
}
