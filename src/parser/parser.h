#ifndef PARSER_H
#define PARSER_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "return.h"

typedef enum config_type {
    XML,
    TXT,
    UNRECOGNIZED,
    ERROR
} config_type_t;

#define CONFIG_MAX_LENGTH 10240 // FIXME, 10240 is only a workaround solution
#define XML_PATTERN "<?xml"
#define TXT_PATTERN "[server]"

void
parser_read_file(const char *file_path);

config_type_t
parser_check_type(const char *file_path);

ret_t
parser_read_xml(const char *file_path);

ret_t
parser_read_txt(const char *file_path);

ret_t
parser_read_xml_server(const xmlNodePtr xml_root);

ret_t
parser_read_xml_table(const xmlNodePtr xml_root);

ret_t
parser_read_xml_cmd(const xmlNodePtr xml_root);

#endif
