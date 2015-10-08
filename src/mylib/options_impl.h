#ifdef options_table
#ifdef PROGRAM_DESCRIPTION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "options.h"
#include "log.h"

#define option_type_bool     int
#define option_type_string   char *
#define option_type_int      int

#define option_value_required_bool     0
#define option_value_required_int      1
#define option_value_required_string   1

options_t *options = NULL;

typedef enum {
    #define item(name, opt, optc, type, required, value, help) \
        OPT_ ## name,
    options_table
    #undef item
    OPT_MAX_OPT_NUM,
} options_enum;

char *strdup(const char *s);

static void
help_info() {
    printf("%s\n", PROGRAM_DESCRIPTION);
    printf("Options: (= means it requires a value, * means it's mandatory)\n");

    const char *opt_mandatory[] = { " ", "*" };
    const char *value_required[] = { " ", "=" };

    #define item(name, opt, optc, type, required, value, help) \
        printf("    %s  %s  %s                  %s   (%s)\n",                  \
               # opt,                                                          \
               opt_mandatory[required],                                        \
               value_required[option_value_required_ ## type],                 \
               help, # value);
    options_table
    #undef item
}

void
options_parse(int argc, char *argv[]) {

    static options_t options_base;

    options = &options_base;

    // Initialize the options with default value
    #define item(name, opt, optc, type, required, value, help) \
        options->name = value;
    options_table
    #undef item

    // For each required option make a flag for it
    #define item(name, opt, optc, type, required, value, help) \
        int opt_required_flag_ ## name = required;
    options_table
    #undef item

    // Create a short option string
    const char *value_required[] = { "", ":" };
    char option_str[OPT_MAX_OPT_NUM * 2 + 1];
    int option_str_offset = 0;

    #define item(name, opt, optc, type, required, value, help) \
        sprintf(option_str + option_str_offset, "%s%s",                        \
                # opt, value_required[option_value_required_ ## type]);        \
        option_str_offset += 1 + option_value_required_ ## type;
    options_table;
    #undef item

    char opt;

    // Get the options
    while ((opt = getopt(argc, argv, option_str)) != -1) {
    #define mark_required(name) opt_required_flag_ ## name = 0;
    #define option_handle_string(name, optarg) options->name = strdup(optarg);
    #define option_handle_bool(name, optarg) options->name = !(options->name);
    #define option_handle_int(name, optarg) options->name = atoi(optarg);

    #define item(name, opt, optc, type, required, value, help) \
        case optc:                                  \
            mark_required(name)                     \
            option_handle_ ## type (name, optarg)   \
            break;

        switch (opt) {
        options_table
        default:
            help_info();
            exit(EXIT_FAILURE);
        }

    #undef item
    }

    if (options->help) {
        help_info();
        exit(EXIT_SUCCESS);
    }

    // Check the required options
    #define item(name, opt, optc, type, required, value, help)   \
        if(opt_required_flag_ ## name) {                         \
            log_error("Option -%c is required.", optc);        \
            help_info();                                         \
            exit(EXIT_FAILURE);                                  \
        }
    options_table
    #undef item

    // Special check for each option
}

#undef option_type_bool
#undef option_type_string
#undef option_type_number

#undef option_value_required_bool
#undef option_value_required_number
#undef option_value_required_string

#undef mark_required
#undef option_handle_string
#undef option_handle_bool
#undef option_handle_number

#endif // PROGRAM_DESCRIPTION
#endif // options_table
