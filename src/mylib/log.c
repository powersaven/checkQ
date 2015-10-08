#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "log.h"

#define LOG_FILE stdout

#define color_table \
    color_item(BLACK,   0)   \
    color_item(RED,     1)   \
    color_item(GREEN,   2)   \
    color_item(YELLOW,  3)   \
    color_item(BLUE,    4)   \
    color_item(PINK,    5)   \
    color_item(CYAN,    6)   \
    color_item(WHITE,   7)

typedef enum {
    #define color_item(name, code) name,
    color_table
    #undef color_item
} log_color_t;

static const char *color_strs[] __attribute__((unused)) = {
    #define color_item(name, code) \
        "\e[3" #code "m",
    color_table
    #undef color_item
};

static const char *bold_color_strs[] __attribute__((unused)) = {
    #define color_item(name, code) \
        "\e[1;3" #code "m",
    color_table
    #undef color_item
};

#define reset             "\e[0m"
#define color_str(code)       (color_strs[code])
#define bold_color_str(code)  (bold_color_strs[code])

static const char *log_level_str[] = {
    #define item(name, color, prio, str) str,
    log_level_table
    #undef item
};

static const char *
get_level_str_color(log_level_t level) {
    switch (level) {
    #define item(name, color, prio, str) \
        case name: return color_str(color);
    log_level_table
    #undef item
    }

    return NULL;
}

void
log_print_helper_impl(const log_level_t    level, 
                      const char          *file_name,
                      const int            line,
                      const char          *func,
                      const char          *string) {
    // Check log level
    if (level < MIN_LOG_PRINT_LEVEL)
        return;

    // Get current thread id
    int tid = (int)pthread_self();

    // Get time string
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%F %T", localtime(&now));

    // Decide if use color mode
    int color_flag = COLOR_LOG && isatty(fileno(LOG_FILE));

    // Get the file name
    char *file = (char *)file_name;
    char *tmp;
    while ((tmp = strstr(file, "/")) != NULL)
        file = tmp + 1;

    // Print the log
    const char *level_str = log_level_str[level];

    if (color_flag) {
        fprintf(LOG_FILE, 
                "%s%s " reset
                "%s%s " reset 
                "%s%x %s:%d [%s] " reset 
                "%s\n",
                color_str(GREEN), time_str, 
                get_level_str_color(level), level_str, 
                color_str(GREEN), tid, file, line, func, 
                string);
    } else {
        fprintf(LOG_FILE, "%s %s %x %s:%d [%s] %s\n", 
                time_str, level_str, tid, file, line, func, string);
    }
}
