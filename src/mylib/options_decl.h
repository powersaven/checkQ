#ifdef options_table
#ifdef PROGRAM_DESCRIPTION

#define option_type_bool     int
#define option_type_string   char *
#define option_type_int      int

typedef struct {
#define item(name, opt, optc, type, required, value, help) \
        option_type_ ## type name;
    options_table
#undef item
} options_t;

extern options_t *options;

#undef option_type_bool
#undef option_type_string
#undef option_type_number

#endif // PROGRAM_DESCRIPTION
#endif // options_table
