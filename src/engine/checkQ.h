#ifndef CHECKQ_H
#define CHECKQ_H

#define PROGRAM_NAME "checkQ"
#define PROGRAM_DESCRIPTION "Mysql data consistency check tool checkQ 1.0"

#define options_table \
    /*   name          opt optc type     required     value                    \
     *   help                                                                */\
    item(help,         h,  'h', bool,    0,           0,                       \
         "Print help information        ")                                     \
    item(config,       c,  'c', string,  1,           NULL,                    \
         "Config file name              ")                                     \
    item(end_time,     e,  'e', int,     1,           600,                     \
         "Program Run Time(sec)         ")                                     \
    item(thread_num,   t,  't', int,     1,           0,                       \
         "Engine thread number          ")                                     \
    item(stats_intval, i,  'i', int,     1,           10,                      \
         "stats print interval(sec)     ")                                     \
    item(alter_intval, a,  'a', int,     0,           600,                     \
         "alter table interval(sec)     ")                                     \
    item(username,     u,  'u', string,  1,           "admin",                 \
         "Mysql username                ")                                     \
    item(password,     p,  'p', string,  1,           "admin",                 \
         "Mysql password                ")                                     \

#include "options_decl.h"

#endif
