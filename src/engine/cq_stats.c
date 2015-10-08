#include <sys/time.h>
#include "engine/cq_engine.h"
#include "engine/cq_stats.h"

int set_sql_write_time(eng_ctx_t* eng_ctx) {
    return gettimeofday(&(eng_ctx->stats.tv_start), &(eng_ctx->stats.tz_start));
}

int set_sql_read_time(eng_ctx_t* eng_ctx) {
    return gettimeofday(&(eng_ctx->stats.tv_end), &(eng_ctx->stats.tz_end));
}

long long diff_sql_write_read_time(eng_ctx_t* eng_ctx) {
    return (eng_ctx->stats.tv_end.tv_sec 
        - eng_ctx->stats.tv_start.tv_sec) * 1000000LL 
        + (eng_ctx->stats.tv_end.tv_usec 
        - eng_ctx->stats.tv_start.tv_usec);    
}

