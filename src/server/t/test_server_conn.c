#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "log.h"
#include "frandom.h"
#include "server/cq_server.h"
#include "lib/libmysql/include/mysql.h"

#define SQL_LEN 200

int main(int argc, char *argv[]) {    
    cq_server_t* pserver;

    srand((int) (time(0)));
    // normal server test
    pserver = server_init("10.1.62.1", 3306, WRITE_SERVER);
    assert(pserver);

    int ret = mysql_select_db(get_server_mysql(pserver), "test");
    assert(ret == 0);

    char sql[SQL_LEN];
    sprintf(sql, "create table test (a bigint primary key, b varchar(512))");

    ret = mysql_real_query(get_server_mysql(pserver), sql, strlen(sql));
    log_info("ret status:%d:%s\n",ret,mysql_error(get_server_mysql(pserver)));
    
    assert(ret == 0);
    server_destroy(pserver);

    return 0;
}
