#ifndef CQ_STATS_H
#define CQ_STATS_H

#include <sys/time.h>

struct eng_stats {
    struct timeval tv_start;
    struct timeval tv_end;
    struct timezone tz_start;
    struct timezone tz_end;

    int ninsert;
    int nupdate;
    int ndelete;
    int nselect;
    
    int ncheck_fail;
};

int set_sql_write_time(eng_ctx_t* eng_ctx);
int set_sql_read_time(eng_ctx_t* eng_ctx);
long long diff_sql_write_read_time(eng_ctx_t* eng_ctx);

#endif

