#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <time.h>

#include "parser.h"
#include "file_operation.h"
#include "verify.h"
#include "log.h"
#include "mylib.h"

#include "server/cq_server.h"
#include "command/cq_command.h"
#include "table/cq_table.h"

void
parser_read_file(const char *file_path) {
    config_type_t my_config_type;
    ret_t         parse_ret;

    if ( RET_FAILED == file_is_exists(file_path) ) {
        log_error("Specific config file \"%s\"not exists", file_path);
        // TODO: Fatal error handling
        exit(2);
    }
    my_config_type = parser_check_type(file_path);
    
    parse_ret = RET_SUCCEED;
    if (XML == my_config_type) {
        parse_ret = parser_read_xml(file_path);
    } else if (TXT == my_config_type) {
        parse_ret = parser_read_txt(file_path);
    } else if (UNRECOGNIZED == my_config_type) {
        log_error("Unrecognized config file type");
        parse_ret = RET_FAILED;
    } else {
        // Some error occured when checking config type
        // Should never reach here
        exit(EXIT_FAILURE);
    }

    if ( RET_FAILED == parse_ret )
        exit(EXIT_FAILURE);
}

config_type_t
parser_check_type(const char *file_path) {
    FILE *config_file;
    int byte_read;
    size_t buff_size;
    char *buff = NULL;
    config_type_t ret = ERROR;

    buff_size = file_get_size(file_path);

    // FIXME, buff_size shall not be judged here 
    if (buff_size <= 0 || buff_size > CONFIG_MAX_LENGTH) {
        log_error("Size of config file is invalid");
        goto error;
    }
    buff = my_malloc(buff_size + 1);

    config_file = fopen(file_path, "r");
    if (NULL == config_file) {
        log_error("Failed to open config file");
        goto error;
    }

    while (-1 != (byte_read = getline(&buff, &buff_size, config_file))) {
        if (NULL != strstr(buff, XML_PATTERN)) {
            ret = XML;
        } else if (NULL != strstr(buff, TXT_PATTERN)) {
            ret = TXT;
        }
    }

error:
    fclose(config_file);
    my_free(buff);
    return ret;
}

ret_t
parser_read_xml(const char *file_path) {
    xmlDocPtr xml_doc;
    xmlNodePtr xml_root;

    xml_doc = xmlReadFile(file_path, "UTF-8", XML_PARSE_NOBLANKS);
    if (NULL == xml_doc) {
        log_error("Failed to read XML file");
        goto error;
    }

    xml_root = xmlDocGetRootElement(xml_doc);
    if (NULL == xml_root) {
        log_error("XML file has no root node");
        goto error;
    }
    
    if (RET_FAILED == parser_read_xml_server(xml_root)) {
        log_error("Failed to read XML's server config");
        goto error;
    }
    if (RET_FAILED == parser_read_xml_cmd(xml_root)) {
        log_error("Failed to read XML's command config");
        goto error;
    }
    if (RET_FAILED == parser_read_xml_table(xml_root)) {
        log_error("Failed to read XML's table config");
        goto error;
    }

    xmlFreeDoc(xml_doc);
    xmlCleanupParser();
    return RET_SUCCEED;

error:
    xmlFreeDoc(xml_doc);
    xmlCleanupParser();
    return RET_FAILED;
}

ret_t
parser_read_xml_server(const xmlNodePtr xml_root) {
    xmlNodePtr cur_node;
    cq_server_t *my_server_p;
    char *ip, *port_str;
    int port;
    server_type_t type;
    ip = port_str = NULL;

    cur_node = xml_root->xmlChildrenNode;
    while (0 != xmlStrcmp(cur_node->name, BAD_CAST "servers")) {
        cur_node = cur_node->next;
        if (NULL == cur_node) {
            log_debug("XML config file has no server mode");
            goto error;
        }
    }

    cur_node = cur_node->xmlChildrenNode;

    while (NULL != cur_node) {

        if (0 == xmlStrcmp(cur_node->name, BAD_CAST "write")) {
            type = WRITE_SERVER;
        } else if (0 == xmlStrcmp(cur_node->name, BAD_CAST "read")) {
            type = READ_SERVER;
        } else {
            log_debug("XML config file has no server");
            log_debug("Current node's name : %s", cur_node->name);
            goto error;
        }

        // Maybe check prop exists or not first safer
        ip       = (char *) xmlGetProp(cur_node, BAD_CAST "ip");
        port_str = (char *) xmlGetProp(cur_node, BAD_CAST "port");
        if (NULL == ip || NULL == port_str) {
            log_debug("XML config file has no ip/port properties");
            goto error;
        }
        port = atoi(port_str);

        // verify if ip and port are valid
        if (RET_FAILED == verify_ip(ip)) {
            log_debug("Invalid ip : %s", ip);
            goto error;
        } else if (RET_FAILED == verify_port(port)) {
            log_debug("Invalid port: %d", port);
            goto error;
        }

        my_server_p = server_init(ip, port, type);
        if (RET_FAILED == cq_cluster_add(cluster_base, my_server_p)) {
            log_debug("Failed to add server into cluster");
            goto error;
        }

        cur_node = cur_node->next;
        xmlFree(ip);
        xmlFree(port_str);
    }

    return RET_SUCCEED;

error:
    xmlFree(ip);
    xmlFree(port_str);
    return RET_FAILED;
} 

ret_t
parser_read_table_data(const xmlNodePtr cur_node, schema_t *my_schema) {
    table_data_t *my_table_data;
    char *name, *type, *length_str;
    int length;

    name   = (char *) xmlGetProp(cur_node, BAD_CAST "name");
    type   = (char *) xmlGetProp(cur_node, BAD_CAST "type");

    if (RET_SUCCEED == verify_string(type, "varchar")) {
        length_str = (char *) xmlGetProp(cur_node, BAD_CAST "length");
        length = atoi(length_str);
        xmlFree(length_str);
        my_table_data = table_data_init(name, VARCHAR, length);
    } else if (RET_SUCCEED == verify_string(type, "bigint")) {
        my_table_data = table_data_init(name, BIGINT, 0);
    } else {
        log_debug("Invalid value's type : %s", type);
        goto error;
    }
    
    if (RET_FAILED == schema_add(my_schema, my_table_data)) {
        log_debug("Failed to add table data into schema");
        goto error;
    }

    xmlFree(name);
    xmlFree(type);
    return RET_SUCCEED;

error:
    xmlFree(name);
    xmlFree(type);
    return RET_FAILED;
}

ret_t
parser_read_table_schema(const xmlNodePtr xml_node, table_schema_t **my_table_schema_p) {
    xmlNodePtr cur_node;
    schema_t *my_key_schema;
    schema_t *my_value_schema;
    table_schema_t *my_table_schema;

    my_key_schema = schema_init();
    my_value_schema = schema_init();

    cur_node = xml_node->xmlChildrenNode;

    while (NULL != cur_node) {

        if (0 == xmlStrcmp(cur_node->name, BAD_CAST "key")) {
            if (RET_FAILED == parser_read_table_data(cur_node, my_key_schema)) {
                log_debug("Failed to add table_data to key_schema");
                return RET_FAILED;
            }
        } else {
            if (RET_FAILED == parser_read_table_data(cur_node, my_value_schema)) {
                log_debug("Failed to add table_data to val_schema");
                return RET_FAILED;
            }
        }

        cur_node = cur_node->next;
    }

    my_table_schema = table_schema_init(my_key_schema, my_value_schema);
    if (NULL == my_table_schema) {
        log_debug("Failed to init a table_schema");
        return RET_FAILED;
    }

    *my_table_schema_p = my_table_schema;

    return RET_SUCCEED;
}

ret_t
parser_read_xml_table(const xmlNodePtr xml_root) {
    xmlNodePtr cur_node;
    xmlNodePtr tables_node;

    cur_node = xml_root->xmlChildrenNode;
    while (0 != xmlStrcmp(cur_node->name, BAD_CAST "tables")) {
        cur_node = cur_node->next;
        if (NULL == cur_node) {
            log_debug("XML config file has no tables node");
            return RET_FAILED;
        }
    }

    tables_node = cur_node;
    cur_node = cur_node->xmlChildrenNode;

    while (NULL != cur_node) {
        table_schema_t *my_table_schema = NULL;

        if (0 == xmlStrcmp(cur_node->name, BAD_CAST "table")) {
            if (RET_FAILED == parser_read_table_schema(cur_node, &my_table_schema)) {
                return RET_FAILED;
            }
            if (RET_FAILED == schema_set_add(my_table_schema)) {
                return RET_FAILED;
            }
        }
        cur_node = cur_node->next;
    }
    
    return RET_SUCCEED;
}

ret_t
parser_read_cmd_list(const xmlNodePtr xml_node, cmd_list_t* my_cmd_list) {
    xmlNodePtr cur_node;
    cmd_t *cmd;
    cmd_type_t cmd_type;

    cur_node = xml_node->xmlChildrenNode;

    while (NULL != cur_node) {

        if (0 == xmlStrcmp(cur_node->name, BAD_CAST "insert")) {
            cmd_type = INSERT;
        } else if (0 == xmlStrcmp(cur_node->name, BAD_CAST "delete")) {
            cmd_type = DELETE;
        } else if (0 == xmlStrcmp(cur_node->name, BAD_CAST "update")) {
            cmd_type = UPDATE;
        } else if (0 == xmlStrcmp(cur_node->name, BAD_CAST "select")) {
            cmd_type = SELECT;
        } else {
            log_debug("Failed to recognize command type \"%s\"", cur_node->name);
            return RET_FAILED;
        }

        cmd = cmd_init(cmd_type);
        if (RET_FAILED == cmd_list_add(my_cmd_list, cmd)) {
            log_debug("Failed to add command to command list");
            return RET_FAILED;
        }
        
        cur_node = cur_node->next;
    }
    
    return RET_SUCCEED;
}
        

ret_t
parser_read_xml_cmd(const xmlNodePtr xml_root) {
    xmlNodePtr cur_node;

    cur_node = xml_root->xmlChildrenNode;

    while (0 != xmlStrcmp(cur_node->name, BAD_CAST "cmds")) {
        cur_node = cur_node->next;

        if(NULL == cur_node) {
            log_debug("XML config file has no cmds node");
            return RET_FAILED;
        }
    }

    cur_node = cur_node->xmlChildrenNode;

    while (NULL != cur_node) {

        if (0 == xmlStrcmp(cur_node->name, BAD_CAST "cmd")) {
            cmd_list_t *my_cmd_list;
            
            my_cmd_list = cmd_list_init();
            if (RET_FAILED ==  parser_read_cmd_list(cur_node, my_cmd_list)) {
                return RET_FAILED;
            }
            if (RET_FAILED == cmd_bucket_add(my_cmd_list)) {
                log_error("Failed to add cmd_list to cmd_bucket");
                return RET_FAILED;
            }
        }
        
        cur_node = cur_node->next;
    }
    return RET_SUCCEED;
}

ret_t
parser_read_txt_server(FILE *file) {
    char *cur;
    char *ip, *port_str;
    char *buf = NULL;
    size_t buf_size = 0;
    int port;
    server_type_t type;
    cq_server_t *my_server_p;

    ip = my_malloc(sizeof(char) * 20);
    port_str = my_malloc(sizeof(char) * 7);
    memset(ip, 0, sizeof(ip));
    memset(port_str, 0, sizeof(port_str));

    for (int i = 0; i < 2; i++) {

        if (-1 == getline(&buf, &buf_size, file)) {
            log_debug("Failed to read a line");
            goto error;
        }
        log_info("Read a line \"%s\"", buf);

        cur = NULL;
        if (NULL != (cur = strstr(buf, "write_servers"))) {
            type = WRITE_SERVER;
        } else if (NULL != (cur = strstr(buf, "read_servers"))) {
            type = READ_SERVER;
        }

        while ('=' != *cur && '\0' != *cur) cur ++;
        if (NULL == cur || '=' != *cur) {
            continue;
        } else {
            int cnt = 0;

            while ('\0' != *cur && (*cur < '0' || *cur > '9')) cur ++;
            while ('.' == *cur || (*cur >= '0' && *cur <= '9')) ip[cnt ++] = *(cur ++);
            ip[cnt] = '\0';
            while ('\0' != *cur && ':' != *(cur ++));
            cnt = 0;
            while (*cur >= '0' && *cur < '9') port_str[cnt ++] = *(cur ++);
            port_str[cnt] = '\0';
            port = atoi(port_str);
        }

        if (RET_FAILED == verify_ip(ip)) {
            log_debug("Invalid ip : %s", ip);
            goto error;
        }
        if (RET_FAILED == verify_port(port)) {
            log_debug("Invalid port : %d", port);
            goto error;
        }

        my_server_p = server_init(ip, port, type);
        if (RET_FAILED == cq_cluster_add(cluster_base, my_server_p)) {
            log_debug("Failed to add server into cluster");
            goto error;
        }
    }
    
    my_free(buf);
    my_free(ip);
    my_free(port_str);
    return RET_SUCCEED;
error:
    my_free(buf);
    my_free(ip);
    my_free(port_str);
    return RET_FAILED;
}

ret_t parser_read_a_table(FILE *file, char **text_p) {
    regex_t reg;
    regmatch_t match;

    int str_size = 8 * CONFIG_MAX_LENGTH;
    int max_size = 1024 * CONFIG_MAX_LENGTH;
    char *text = my_malloc(sizeof(char) * str_size);
    char *buf = NULL;
    char *cur = text;
    size_t buf_size = 0;
    int cnt = 0;

    if (0 != regcomp(&reg, ");", REG_EXTENDED)) {
        log_debug("Failed to compile pattern \"%s\"", ");");
        goto error;
    }

    *cur = '\0';
    do {
        if (-1 == getline(&buf, &buf_size, file)) {
            break;
        }
        log_info("Read a line \"%s\"", buf);
        strcpy(cur, buf);
        cnt += strlen(buf);
        cur += strlen(buf);
        if (cnt + CONFIG_MAX_LENGTH >= str_size) {
            str_size *= 2;
            if (str_size > max_size) {
                log_debug("Not enough memory for read a table");
                goto error;
            }
            cur = text;
            text = my_malloc(sizeof(char) * str_size);
            *text = '\0';
            strcpy(text, cur);
            my_free(cur);
            cur = text + cnt;
        }
    } while (0 != regexec(&reg, buf, 1, &match, 0));
    
    *(text_p) = text;
    my_free(buf);
    regfree(&reg);
    return RET_SUCCEED;

error:
    my_free(buf);
    regfree(&reg);
    my_free(text);
    return RET_FAILED;
}

ret_t
parser_read_txt_table_data(schema_t *my_schema, char *text, int so) {
    char pattern_bigint[] = "(\\w+)\\s+bigint";
    char pattern_varchar[] = "(\\w+)\\s+varchar\\(([0-9]+)\\)";
    int match_size = 2;
    regex_t reg;
    regmatch_t match[match_size];
    char *name = NULL, *length_str = NULL;
    int length = 0;
    table_data_t *my_table_data = NULL;
    
    if (0 != regcomp(&reg, pattern_bigint, REG_EXTENDED)) {
        log_error("Failed to compile pattern \"%s\"", pattern_bigint);
        goto error;
    }
    
    char *cur = text + so;
    int offset;
    if (0 == regexec(&reg, cur, match_size, match, 0)) {
        offset = match[1].rm_eo - match[1].rm_so;
        name = my_malloc(offset + 1);
        memcpy(name, cur + match[1].rm_so, offset);
        *(name + offset) = '\0';
        my_table_data = table_data_init(name, BIGINT, 0);
    } else {

        if (0 != regcomp(&reg, pattern_varchar, REG_EXTENDED)) {
            log_error("Failed to compile pattern \"%s\"", pattern_varchar);
            goto error;
        }

        cur = text + so;
        if (0 == regexec(&reg, cur, match_size, match, 0)) {
            offset = match[1].rm_eo - match[1].rm_so;
            name = my_malloc(offset + 1);
            memcpy(name, cur + match[1].rm_so, offset);
            *(name + offset) = '\0';

            offset = match[2].rm_eo - match[2].rm_so;
            length_str = my_malloc(offset + 1);
            memcpy(length_str, cur + match[2].rm_so, offset);
            *(length_str + offset) = '\0';
            length = atoi(length_str);

            my_table_data = table_data_init(name, VARCHAR, length);
        } else {
            log_debug("Failed to parse table data");
            goto error;
        }
    }

    if (RET_FAILED == schema_add(my_schema, my_table_data)) {
        log_debug("Failed to add table_data to schema");
        goto error;
    }

    my_free(name);
    my_free(length_str);
    regfree(&reg);
    return RET_SUCCEED;
    
error:
    my_free(name);
    my_free(length_str);
    regfree(&reg);
    return RET_FAILED;
}

ret_t
parser_read_txt_table_schema(table_schema_t **my_table_schema,
        char *text, int so) {
    char pattern_key[] = "\\s*(\\w+)\\s+(\\S+)\\s+primary\\s+key\\s*[,\\)]";
    char pattern_val[] = "\\s*(\\w+)\\s+([a-zA-Z0-9_()]+)\\s*[,\\)]";
    int match_size = 3;
    regex_t reg;
    regmatch_t match[match_size];
    char *cur;
    schema_t *key_schema, *val_schema;

    if (0 != regcomp(&reg, pattern_key, REG_EXTENDED)) {
        log_error("Failed to compile pattern \"%s\"", pattern_key);
        goto error;
    }

    key_schema = schema_init();
    cur = text + so;
    while (0 == regexec(&reg, cur, match_size, match, 0)) {
        parser_read_txt_table_data(key_schema, text, so + match[0].rm_so);
        cur += match[0].rm_eo;
    }

    if (0 != regcomp(&reg, pattern_val, REG_EXTENDED)) {
        log_error("Failed to compile pattern \"%s\"", pattern_val);
        goto error;
    }

    val_schema = schema_init();
    cur = text + so;
    while (0 == regexec(&reg, cur, match_size, match, 0)) {
        parser_read_txt_table_data(val_schema, text, so + match[0].rm_so);
        cur += match[0].rm_eo;
    }

    *my_table_schema = table_schema_init(key_schema, val_schema);

    regfree(&reg);
    return RET_SUCCEED;

error:
    regfree(&reg);
    return RET_FAILED;
}

ret_t
parser_read_txt_table(FILE *file) {
    char pattern_table_name[] = "create\\s+table\\s+(\\w+)\\s*\\((.+)\\);";
    char *text = NULL;
    int match_size = 2;
    regex_t reg;
    regmatch_t match[match_size];

    if(RET_FAILED == parser_read_a_table(file, &text)) {
        log_error("Failed to read a table into buffer");
        goto error;
    }

    if (0 != regcomp(&reg, pattern_table_name, REG_EXTENDED)) {
        log_debug("Failed to compile pattern \"%s\"", pattern_table_name);
        goto error;
    }

    log_info("%s", text);
    if (0 == regexec(&reg, text, match_size, match, 0)) {
        int offset = match[1].rm_eo - match[1].rm_so;
        char *name;
        table_t *my_table;
        table_schema_t *my_table_schema;

        if (RET_FAILED == parser_read_txt_table_schema(
                    &my_table_schema, text, match[1].rm_so)) {
            log_debug("Failed to parse txt config's table_schema");
            goto error;
        }

        name = my_malloc(offset + 1);
        memcpy(name, text + match[1].rm_so, offset);
        *(name + offset) = '\0';

        log_info("Get table \"%s\"", name);
        my_table = table_init(name, my_table_schema);
        if (RET_FAILED == table_set_add(my_table)) {
            log_debug("Failed to add table into table_set");
            goto error;
        }

        my_free(name);
    }

    my_free(text);
    regfree(&reg);
    return RET_SUCCEED;

error:
    my_free(text);
    regfree(&reg);
    return RET_FAILED;
}

ret_t
parser_read_txt_cmd_rate(FILE *file, double *rate) {
    char pattern_rate[] = "(\\w+)\\s*=\\s*([0-9\\.]+)\\s*";
    int match_size = 3;
    char *buf = NULL;
    size_t buf_size = 0;
    regex_t reg;
    regmatch_t match[match_size];
    
    if (0 != regcomp(&reg, pattern_rate, REG_EXTENDED)) {
        log_debug("Failed to compile pattern \"%s\"", pattern_rate);
        goto error;
    }

    for (int i = 0; i < 4; i ++) {
        if (-1 == getline(&buf, &buf_size, file)) {
            log_debug("Failed to read a line");
            return RET_FAILED;
        }
        log_info("Read a line \"%s\"", buf);

        if (0 == regexec(&reg, buf, match_size, match, 0)) {
            switch (*(buf + match[1].rm_so)) {
                case 'i': sscanf(buf + match[2].rm_so, "%lf", &rate[0]);
                    break;
                case 'd': sscanf(buf + match[2].rm_so, "%lf", &rate[1]);
                    break;
                case 'u': sscanf(buf + match[2].rm_so, "%lf", &rate[2]);
                    break;
                case 's': sscanf(buf + match[2].rm_so, "%lf", &rate[3]);
                    break;
                default:
                    *(buf + match[1].rm_eo) = '\0';
                    log_debug("Failed to recognize cmd type \"%s\"", buf + match[1].rm_so);
                    goto error;
            }
        }
    }

    if (RET_FAILED == verify_rate(rate)) {
        log_debug("Invalid command rate \"%lf, %lf, %lf, %lf\"", 
                rate[0], rate[1], rate[2], rate[3]);
        goto error;
    }

    my_free(buf);
    regfree(&reg);
    return RET_SUCCEED;

error:
    my_free(buf);
    regfree(&reg);
    return RET_FAILED;
}

ret_t
parser_read_txt_cmd_window(FILE *file, int *rows, int *cols) {
    char pattern_window[] = "(\\w+)\\s*=\\s*([0-9]+)\\s*";
    int match_size = 3;
    char *buf = NULL;
    size_t buf_size = 0;
    regex_t reg;
    regmatch_t match[match_size];
    
    if (0 != regcomp(&reg, pattern_window, REG_EXTENDED)) {
        log_debug("Failed to compile pattern \"%s\"", pattern_window);
        goto error;
    }

    for (int i = 0; i < 2; i ++) {
        if (-1 == getline(&buf, &buf_size, file)) {
            log_debug("Failed to read a line");
            return RET_FAILED;
        }
        log_info("Read a line \"%s\"", buf);

        if (0 == regexec(&reg, buf, match_size, match, 0)) {
            if (NULL != strstr(buf + match[1].rm_so, "cmd_rows")) {
                sscanf(buf + match[2].rm_so, "%d", rows);
            } else if (NULL != strstr(buf + match[1].rm_so, "cmd_cols")) {
                sscanf(buf + match[2].rm_so, "%d", cols);
            } else {
                *(buf + match[1].rm_eo) = '\0';
                log_debug("Failed to recognize cmd_window \"%s\"", buf + match[1].rm_so);
                goto error;
            }
        }
    }

    if (RET_FAILED == verify_window(*rows, *cols)) {
        log_debug("Invalid window size \"%d, %d\"", *rows, *cols);
        goto error;
    }

    my_free(buf);
    regfree(&reg);
    return RET_SUCCEED;

error:
    my_free(buf);
    regfree(&reg);
    return RET_FAILED;
}

ret_t
parser_shuffle_cmds(cmd_type_t* cmd_array, int size) {
    int seed = time(NULL);
    int shuffle_time = 200;
    int a, b;

    srand(seed);
    log_info("Use seed \"%d\" to shuffle a commond list", seed);
    for (int i = 0; i < shuffle_time; i ++) {
        a = rand() % (size - 1);
        b = rand() % (size - 1);
        if (a == b) continue;
        cmd_array[a] ^= cmd_array[b];
        cmd_array[b] ^= cmd_array[a];
        cmd_array[a] ^= cmd_array[b];
    }

    return RET_SUCCEED;
} 

ret_t
parser_gen_cmd_bucket(int rows, int cols, double *rate) {
    cmd_list_t *my_cmd_list;
    cmd_t *my_cmd;
    cmd_type_t *cmd_array = my_malloc(sizeof(cmd_type_t) * cols);
    
    /* Generate a cmd_type_list */
    double cur_rate = 0.0;
    int cur_cmd_num = 0;
    cmd_type_t cmd_pattern[4] = {INSERT, DELETE, UPDATE, SELECT};

    log_info("Try to generate a cmd_type_list");
    for (int i = 0; i < 4; i ++) {
        int tmp = (int) (rate[i] * cols);
        for (int j = 0; j < tmp; j ++) {
            cmd_array[cur_cmd_num ++] = cmd_pattern[i];
        }
        cur_rate += rate[i];
        while ( (double) cur_cmd_num / (double) cols < cur_rate) {
            cmd_array[cur_cmd_num ++] = cmd_pattern[i];
        }
    }

    /* TODO: Swap elements in cmd_array randomly for (rows) times. As the last 
     * cmd must be SELECT, so the last element will be left alone. */
    for (int i = 0; i < rows; i ++) {
        if (RET_FAILED == parser_shuffle_cmds(cmd_array, cols)) {
            log_debug("Failed to shuffle cmds' array");
            goto error;
        }
        
        my_cmd_list = cmd_list_init();
        for (int j = 0; j < cols; j ++) {
            my_cmd = cmd_init(cmd_array[j]);
            cmd_list_add(my_cmd_list, my_cmd);
        }
        cmd_bucket_add(my_cmd_list);
    }

    my_free(cmd_array);
    return RET_SUCCEED;

error:
    my_free(cmd_array);
    return RET_FAILED;
}

ret_t
parser_read_txt(const char *file_path) {
    char *buf = NULL;
    size_t buf_size = 0;
    int rows, cols;
    double rate[4];
    FILE *config_file;

    config_file = fopen(file_path, "r");
    if (NULL == config_file) {
        log_error("Failed to open config file \"%s\"", file_path);
        goto error;
    }

    while (-1 != getline(&buf, &buf_size, config_file)) {

        if (NULL != strstr(buf, "[server]")) {
            if (RET_FAILED == parser_read_txt_server(config_file)) {
                log_debug("Failed to parse TXT's server config");
                goto error;
            }
        } else if (NULL != strstr(buf, "[table]")) {
            if (RET_FAILED == parser_read_txt_table(config_file)) {
                log_debug("Failed to parse TXT's table config");
                goto error;
            }
        } else if (NULL != strstr(buf, "[cmd_rate]")) {
            if (RET_FAILED == parser_read_txt_cmd_rate(config_file, rate)) {
                log_debug("Failed to parse TXT's cmd_rate config");
                goto error;
            }
        } else if (NULL != strstr(buf, "[cmd_window]")) {
            if (RET_FAILED == parser_read_txt_cmd_window(config_file, &rows, &cols)) {
                log_debug("Failed to parse TXT's cmd_window config");
                goto error;
            }
        }
    }

    if (RET_FAILED == parser_gen_cmd_bucket(rows, cols, rate)) {
        log_debug("Failed to generate a cmd_bucket containers randowm cmd_list");
        goto error;
    }

    fclose(config_file);
    my_free(buf);
    return RET_SUCCEED;

error:
    fclose(config_file);
    my_free(buf);
    return RET_FAILED;
}
