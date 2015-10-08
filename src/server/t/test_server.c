#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "log.h"
#include "frandom.h"
#include "server/cq_server.h"

static cq_server_cluster_t* cluster;

int main(int argc, char *argv[]) {    
    cq_server_t* pserver;

    srand((int) (time(0)));
    // normal server test
    pserver = server_init("10.1.105.1", 3306, WRITE_SERVER);
    assert(pserver);

    log_info("%s", get_server_ip(pserver));
    log_info("%d", get_server_port(pserver));
    log_info("%s", get_server_name(pserver));

    server_destroy(pserver);

    // normal cluster test
    pserver = server_init("255.255.255.255", 65535, WRITE_SERVER);

    cq_cluster_init(&cluster);
    cq_cluster_add(cluster, pserver);
    cq_server_t* tmp_server = 
        cq_cluster_get_write_server(cluster, get_server_name(pserver));

    log_info("%s", get_server_name(tmp_server));

    cq_cluster_destroy(cluster);

    char ip_tmp[16];
    int port_tmp;

    cq_cluster_init(&cluster);
    for (int i = 0; i < 100; i++) {
        string_frandom(ip_tmp, 15, i);
        ip_tmp[15] = '\0';
        port_tmp = frandom(i, 0) % 65536;
        
        pserver = server_init(ip_tmp, port_tmp, WRITE_SERVER);
        cq_cluster_add(cluster, pserver);
    }
    
    for (int i = 0; i < 10; i++) {
        pserver = cq_cluster_get_random_write_server(cluster);
        log_info("Get one write server:%s", get_server_name(pserver));
    }

    for (int i = 0; i < 100; i++) {
        string_frandom(ip_tmp, 15, i);
        port_tmp = frandom(i, 0) % 65536;
        
        pserver = server_init(ip_tmp, port_tmp, READ_SERVER);
        cq_cluster_add(cluster, pserver);
    }

    for (int i = 0; i < 10; i++) {
        pserver = cq_cluster_get_random_read_server(cluster);
        log_info("Get one read server:%s", get_server_name(pserver));
    }

    cq_cluster_destroy(cluster);

    cq_cluster_init(&cluster);
    string_frandom(ip_tmp, 15, 2012);
    port_tmp = frandom(2012, 0) % 65536;
    pserver = server_init(ip_tmp, port_tmp, WRITE_SERVER);
    cq_cluster_add(cluster, pserver);
    log_info("Add one write server:%s", get_server_name(pserver));
    pserver = cq_cluster_get_random_write_server(cluster);
    log_info("Get one write server:%s", get_server_name(pserver));

    cq_cluster_destroy(cluster);

    return 0;
}
