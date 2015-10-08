#ifndef CQ_SERVER_H
#define CQ_SERVER_H

#include <sys/types.h>

#include "return.h"

typedef struct cq_server cq_server_t;
typedef struct cq_server_cluster cq_server_cluster_t; 

typedef enum {
    WRITE_SERVER,
    READ_SERVER,  
} server_type_t;

cq_server_t*  server_init(char* ip, int port, server_type_t type);
void          server_destroy(cq_server_t* pserver);
char*         get_server_ip(cq_server_t* pserver);
int           get_server_port(cq_server_t* pserver);
char*         get_server_name(cq_server_t* pserver);
void*         get_server_mysql(cq_server_t* pserver);

void          cq_cluster_init(cq_server_cluster_t** cluster);
ret_t         cq_cluster_add(cq_server_cluster_t* cluster, cq_server_t* pserver);
cq_server_t*  cq_cluster_get_read_server(cq_server_cluster_t* cluster, char* name);
cq_server_t*  cq_cluster_get_write_server(cq_server_cluster_t* cluster, char* name);
cq_server_t*  cq_cluster_get_random_read_server(cq_server_cluster_t* cluster);
cq_server_t*  cq_cluster_get_random_write_server(cq_server_cluster_t* cluster);
cq_server_t** cq_cluster_get_all_read_server(cq_server_cluster_t* cluster, size_t *number);
cq_server_t** cq_cluster_get_all_write_server(cq_server_cluster_t* cluster, size_t *number);
void          cq_cluster_destroy(cq_server_cluster_t* cluster);

extern cq_server_cluster_t* cluster_base;
extern char g_username[20];
extern char g_password[20];

#endif
