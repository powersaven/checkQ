#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "mylib.h"
#include "cq_server.h"
#include "hash.h"
#include "lib/libmysql/include/mysql.h"
#include "lib/libmysql/include/errmsg.h"

#define MAX_SERVER_NAME_LEN 21

char g_username[20];
char g_password[20];

struct cq_server {
    char*         ip;
    int           port;
    char*         name;
    int           name_len;
    server_type_t type;

    unsigned long client_flag;
    char*         unix_socket;
    MYSQL*        mysql; // MYSQL *mysql,
};

struct cq_server_cluster {
    hash_t* write_servers;
    hash_t* read_servers;
};

cq_server_cluster_t* cluster_base = NULL;

static MYSQL* open_mysql(MYSQL *mysql, cq_server_t *server) {

    MYSQL* my_sql = NULL;
    MYSQL* my_sql_ret = NULL;
    my_sql = mysql_init(NULL);

    my_sql_ret = mysql_real_connect(my_sql, server->ip, g_username, g_password, "test",
        server->port, NULL, 0);

    if (NULL == my_sql_ret) {
        log_error(mysql_error(my_sql));
        mysql_close(my_sql);
    }
    
    return my_sql_ret; 
}

static void close_mysql(MYSQL *mysql) {
    my_assert(mysql != NULL);
    mysql_close(mysql);
}

cq_server_t* server_init(char* ip, int port, server_type_t type) {
    int ret;

    cq_server_t* pserver = my_malloc(sizeof(cq_server_t));

    pserver->ip = strdup(ip);
    if (NULL == pserver->ip) {
        log_error("Failed to strdup ip!");
        return NULL;
    }

    pserver->port = port;

    ret = asprintf(&(pserver->name), "%s:%d", pserver->ip, pserver->port);
    if (-1 == ret) {
        log_error("Failed to asprintf server name!");
        return NULL;
    }

    pserver->name_len = strlen(pserver->name);
    if (pserver->name_len > MAX_SERVER_NAME_LEN) {
        log_error("Invalid server name:%s", pserver->name);
        server_destroy(pserver);
        return NULL;
    }

    pserver->type = type;

    pserver->mysql = open_mysql(pserver->mysql, pserver);
    if (NULL == pserver->mysql) {
        log_error("Connect to mysql server %s:%d failed:%s",
                ip, port, mysql_error(pserver->mysql));
        abort();
    }

    // auto reconnect
    my_bool reconnect = true;
    if (mysql_options(pserver->mysql, MYSQL_OPT_RECONNECT, &reconnect) != 0) {
        log_error("Set auto reconnect failed");
        abort();
    }

    return pserver;
}

void server_destroy(cq_server_t* pserver)  {

    close_mysql(pserver->mysql);

    my_free(pserver->ip);
    my_free(pserver->name);
    my_free(pserver);

    return;
}

char* get_server_ip(cq_server_t* pserver) {
    return pserver->ip;
}

int get_server_port(cq_server_t* pserver) {
    return pserver->port;
}

char* get_server_name(cq_server_t* pserver) {
    return pserver->name;
}

// shall be MYSQL*
void* get_server_mysql(cq_server_t* pserver) {
    return pserver->mysql;
}

static void* cq_server_key_func(void *entry) {
    return get_server_name((cq_server_t*)entry);
}

static size_t cq_server_key_size_func(void *key) {
    return strlen((char *)key);
}

static void cq_server_free_func(void *entry) {
    return server_destroy((cq_server_t*)entry);
}

void cq_cluster_init(cq_server_cluster_t** cluster) {
    *cluster = my_malloc(sizeof(cq_server_cluster_t));

    (*cluster)->write_servers =
          hash_alloc(cq_server_key_func,
                     cq_server_key_size_func,
                     cq_server_free_func);

    (*cluster)->read_servers =
          hash_alloc(cq_server_key_func,
                     cq_server_key_size_func,
                     cq_server_free_func);

    return;
}

static ret_t add_write_server(cq_server_cluster_t* cluster, 
    cq_server_t* pserver) {
    return hash_add(cluster->write_servers, pserver);
}

static ret_t add_read_server(cq_server_cluster_t* cluster, 
    cq_server_t* pserver) {
    return hash_add(cluster->read_servers, pserver);
}

ret_t cq_cluster_add(cq_server_cluster_t* cluster, cq_server_t* pserver) {
    if (WRITE_SERVER == pserver->type) {
        return add_write_server(cluster, pserver);
    } else {
        return add_read_server(cluster, pserver);
    }
}

cq_server_t* cq_cluster_get_read_server(cq_server_cluster_t* cluster, 
    char* name) {
    return hash_get(cluster->read_servers, name);
}

cq_server_t* cq_cluster_get_write_server(cq_server_cluster_t* cluster, 
    char* name) {
    return hash_get(cluster->write_servers, name);
}

static cq_server_t** cq_cluster_get_all(hash_t *hash, size_t *number) {
    return (cq_server_t **)hash_get_entries(hash, number);
}

cq_server_t** cq_cluster_get_all_read_server(cq_server_cluster_t* cluster, 
    size_t *number) {
    return (cq_server_t **)cq_cluster_get_all(
        cluster->read_servers, number);
}

cq_server_t** cq_cluster_get_all_write_server(cq_server_cluster_t* cluster, 
    size_t *number) {
    return (cq_server_t **)cq_cluster_get_all(
        cluster->write_servers, number);
}

cq_server_t* cq_cluster_get_random_write_server(cq_server_cluster_t* cluster) {
    size_t server_num;
    cq_server_t*  pserver = NULL;
    cq_server_t** pservers = cq_cluster_get_all_write_server(cluster, &server_num);

    if (server_num > 0) {
        pserver = pservers[rand() % server_num];
    } else {
        log_debug("read server list is NULL!");
    }

    // low perf, will fix it later in hash.c
    my_free(pservers);

    return pserver;
}

cq_server_t* cq_cluster_get_random_read_server(cq_server_cluster_t* cluster) {
    size_t server_num;
    cq_server_t*  pserver = NULL;
    cq_server_t** pservers = cq_cluster_get_all_read_server(cluster, &server_num);

    if (server_num > 0) {
        pserver = pservers[rand() % server_num];
    } else {
        log_debug("read server list is NULL!");
    }

    // low perf, will fix it later in hash.c
    my_free(pservers);

    return pserver;
}

void cq_cluster_destroy(cq_server_cluster_t* cluster) {
    hash_destroy(cluster->write_servers);
    hash_destroy(cluster->read_servers);

    my_free(cluster);
    cluster = NULL;
}
