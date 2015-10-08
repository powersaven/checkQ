#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>

#define COLOR_LOG 1
#define MIN_LOG_PRINT_LEVEL LOG_INFO

#define log_level_table \
    /*   Name       Color    Priority    String    */\
    item(LOG_DBG,   BLUE,    0,          "DEBUG ")   \
    item(LOG_INFO,  GREEN,   1,          "INFO  ")   \
    item(LOG_WARN,  YELLOW,  2,          "WARN  ")   \
    item(LOG_ERR,   RED,     3,          "ERROR ")   \

typedef enum {
    #define item(name, color, prio, str) name = prio,
    log_level_table
    #undef item
} log_level_t;

#define log_info(format, ...) \
    log_print_helper(LOG_INFO, __FILE__, __LINE__, __func__,  \
    format, ## __VA_ARGS__)

#define log_debug(format, ...) \
    log_print_helper(LOG_DBG, __FILE__, __LINE__, __func__,   \
    format, ## __VA_ARGS__)

#define log_error(format, ...) \
    log_print_helper(LOG_ERR, __FILE__, __LINE__, __func__,   \
    format, ## __VA_ARGS__)

#define log_warn(format, ...) \
    log_print_helper(LOG_WARN, __FILE__, __LINE__, __func__,  \
    format, ## __VA_ARGS__)

#define log_print_helper(level, file, line, func, format, ...) \
    do {                                                      \
        char *str;                                            \
        asprintf(&str, format, ## __VA_ARGS__);               \
        log_print_helper_impl(level, file, line, func, str);  \
        free(str);                                            \
    } while (0)

void
log_print_helper_impl(const log_level_t   level,
                      const char         *file, 
                      const int           line, 
                      const char         *func,
                      const char         *string);

#endif
