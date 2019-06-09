#include <stdlib.h>
#include <string.h>


#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))
#define MAX_OPTION_LEN (64)

#define ARGS_OPTION(short_flagstr, long_flagstr, argtype, argdata) { \
    .short_flag = short_flagstr, \
    .long_flag = long_flagstr, \
    .arg_type = argtype, \
    .opt_type = OPTTYPE_OPTION, \
    .data = argdata, \
    .seen = 0 \
}

#define ARGS_POSITIONAL_ARG(argtype, argdata) { \
    .short_flag = NULL, \
    .long_flag = NULL, \
    .arg_type = argtype, \
    .opt_type = OPTTYPE_POSITIONAL, \
    .data = argdata, \
    .seen = 0 \
}

#define ARGS_FLAG(short_flagstr, long_flagstr, value) { \
    .short_flag = short_flagstr, \
    .long_flag = long_flagstr, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_FLAG, \
    .data = value, \
    .seen = 0 \
}

#define ARGS_END_OF_OPTIONS { \
    .short_flag = NULL, \
    .long_flag = NULL, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_NONE, \
    .data = NULL, \
    .seen = 0 \
}


typedef int (*arg_decoder_t)(char*, void*);

static int _decode_int(char *input, void *output);
static int _decode_long(char *input, void *output);
static int _decode_uint(char *input, void *output);
static int _decode_ulong(char *input, void *output);
static int _decode_float(char *input, void *output);
static int _decode_double(char *input, void *output);
static int _decode_string(char *input, void *output);
static int _decode_hex(char *input, void *output);

typedef struct {
    arg_decoder_t decode;
    const char *name;
} decode_params_t;

typedef enum {
    ARGTYPE_INT = 0,
    ARGTYPE_LONG,
    ARGTYPE_UINT,
    ARGTYPE_ULONG,
    ARGTYPE_FLOAT,
    ARGTYPE_DOUBLE,
    ARGTYPE_STRING,
    ARGTYPE_HEX,
    ARGTYPE_NONE
} argtype_e;

static decode_params_t _decoders[] = {
    { .decode=_decode_int, .name="integer" },                 // ARGTYPE_INT
    { .decode=_decode_long, .name="long integer" },           // ARGTYPE_LONG
    { .decode=_decode_uint, .name="unsigned integer" },       // ARGTYPE_UINT
    { .decode=_decode_ulong, .name="unsigned long integer" }, // ARGTYPE_ULONG
    { .decode=_decode_float, .name="floating point" },        // ARGTYPE_FLOAT
    { .decode=_decode_double, .name="floating point" },       // ARGTYPE_DOUBLE
    { .decode=_decode_string, .name="string" },               // ARGTYPE_STRING
    { .decode=_decode_hex, .name="hexadecimal" }              // ARGTYPE_HEX
};

typedef enum {
    OPTTYPE_FLAG,
    OPTTYPE_OPTION,
    OPTTYPE_POSITIONAL,
    OPTTYPE_NONE
} opttype_e;

typedef struct {
    int seen;
    const char *short_flag;
    const char *long_flag;
    argtype_e arg_type;
    opttype_e opt_type;
    void *data;
} args_option_t;

static int _decode_int(char *input, void *output)
{
    char *endptr = NULL;
    long intval = strtol(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    int *int_output = (int *)output;
    *int_output = (int)intval;
    return 0;
}

static int _decode_long(char *input, void *output)
{
    char *endptr = NULL;
    long longval = strtol(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    long *long_output = (long *)output;
    *long_output = longval;
    return 0;
}

static int _decode_uint(char *input, void *output)
{
    char *endptr = NULL;
    unsigned long intval = strtoul(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    unsigned *int_output = (unsigned *)output;
    *int_output = (unsigned)intval;
    return 0;
}

static int _decode_ulong(char *input, void *output)
{
    char *endptr = NULL;
    unsigned long longval = strtoul(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    unsigned long *long_output = (unsigned long *)output;
    *long_output = longval;
    return 0;
}

static int _decode_float(char *input, void *output)
{
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (!endptr || *endptr)
    {
        return -1;
    }

    float *float_output = (float *)output;
    *float_output = (float)val;
    return 0;
}

static int _decode_double(char *input, void *output)
{
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (!endptr || *endptr)
    {
        return -1;
    }

    double *double_output = (double *)output;
    *double_output = val;
    return 0;
}

static int _decode_string(char *input, void *output)
{
    char **outptr = (char **)output;
    *outptr = input;
    return 0;
}

static int _decode_hex(char *input, void *output)
{
    char *endptr = NULL;
    long val = strtol(input, &endptr, 16);
    if (!endptr || *endptr)
    {
        return -1;
    }

    long *long_output = (long *)output;
    *long_output = val;
    return 0;
}

static args_option_t *_get_option(args_option_t *options, const char *opt)
{
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        const char *shortf = options[i].short_flag;
        const char *longf = options[i].long_flag;

        if (shortf && (0 == strncmp(shortf, opt, MAX_OPTION_LEN)))
        {
            return &options[i];
        }

        if (longf && (0 == strncmp(options[i].long_flag, opt, MAX_OPTION_LEN)))
        {
            return &options[i];
        }
    }

    return NULL;
}


/*
 * Returns 1 if the string at argv[index] follows a valid option flag that
 * requires an argument, 0 otherwise
 */
static int is_optarg(char *argv[], int index, args_option_t *options)
{
    if (index == 1)
    {
        return 0;
    }

    if (argv[index - 1][0] != '-')
    {
        return 0;
    }

    args_option_t *opt = _get_option(options, argv[index - 1]);
    if (NULL == opt)
    {
        return 0;
    }

    return opt->opt_type == OPTTYPE_OPTION;
}


static int _optargs_ahead(int argc, char *argv[], int index)
{
    for (int i = index; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            return 1;
        }
    }

    return 0;
}

/*
 * Find any arguments that do not follow an option flag requiring an argument,
 * and shift them to the end of the argument list. Returns the index of the
 * first positional arg after shifting is done.
 */
static void _shift_nonopt_args(int argc, char *argv[], args_option_t *options)
{
    int moved = 0;

    for (int i = 1; i < (argc - moved); i++) {
        if (argv[i][0] != '-') {
            if (is_optarg(argv, i, options)) {
                continue;
            }

            if (!_optargs_ahead(argc, argv, i + 1) && (0 == moved))
            {
                return;
            }

            char *temp = argv[i];

            for (int j = i + 1; j < argc; j++) {
                argv[j - 1] = argv[j];
            }

            moved += 1;
            argv[argc - 1] = temp;
        }
    }
}

/*
 * Set all flag data to zero
 */
static void _init_options(args_option_t *options)
{
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        // Set all flags to 0
        if (options[i].data && (OPTTYPE_FLAG == options[i].opt_type))
        {
            int *intptr = (int *)options[i].data;
            *intptr = 0;
        }
    }
}

/*
 * Decode the argument data for a named option
 */
int _decode_value(args_option_t *opt, char *flag, char *input)
{
    decode_params_t *params = &_decoders[opt->arg_type];
    if (params->decode(input, opt->data) < 0)
    {
        printf("%s value required for %s\n", params->name, flag);
        return -1;
    }

    return 0;
}

/*
 * Parse a single positional arg
 */
int _parse_positional(char *arg, args_option_t *options)
{

    // Find the first unseen positional arg entry
    args_option_t *opt = NULL;
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        if ((options[i].opt_type == OPTTYPE_POSITIONAL) && !options[i].seen)
        {
            opt = &options[i];
            break;
        }
    }

    // If no unseen positional args, we're done here
    if (NULL == opt)
    {
        return 0;
    }

    opt->seen = 1;
    if (_decode_value(opt, "positional argument", arg) < 0)
    {
        return -1;
    }

    return 0;
}

/*
 * Calculate max. number of positional arguments allowed
 */
int _calculate_max_positionals(args_option_t *options)
{
    int ret = 0;
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        if (OPTTYPE_POSITIONAL == options[i].opt_type)
        {
            ret += 1;
        }
    }

    return ret;
}

/*
 * Parse all named options and flags
 */
int _parse_options(int argc, char *argv[], args_option_t *options)
{
    int max_positionals = _calculate_max_positionals(options);
    int positionals = 0;

    for (int i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') && argv[i][1])
        {
            // Check if we have a matching option
            args_option_t *opt = _get_option(options, argv[i]);
            if (NULL == opt)
            {
                printf("unknown option '%s'\n", argv[i]);
                return -1;
            }

            if (opt->seen)
            {
                printf("option '%s' is set more than once\n", argv[i]);
                return -1;
            }

            opt->seen = 1;

            // If this is a flag, just set it and we're done
            if (opt->opt_type == OPTTYPE_FLAG)
            {
                int *intptr = (int *)opt->data;
               *intptr = 1;
               continue;
            }

            // Sanity check on data ptr
            if (NULL == opt->data)
            {
                return -1;
            }

            // Sanity check on arg. type
            if (ARRAY_LEN(_decoders) <= opt->arg_type) {
                return -1;
            }

            // Option requires an argument
            if (i == (argc - 1))
            {
                printf("option '%s' requires an argument\n", argv[i]);
                return -1;
            }

            if (_decode_value(opt, argv[i], argv[i + 1]) < 0)
            {
                return -1;
            }

            i += 1;
        }

        else
        {
            if (positionals >= max_positionals)
            {
                printf("too many positional arguments\n");
                return -1;
            }

            if (_parse_positional(argv[i], options) < 0)
            {
                return -1;
            }

            positionals += 1;
        }
    }

    return 0;
}

int parse_arguments(int argc, char *argv[], args_option_t *options)
{
    if ((NULL == argv) || (NULL == options))
    {
        return -1;
    }

    if (1 >= argc)
    {
        return 0;
    }

    _init_options(options);
    _shift_nonopt_args(argc, argv, options);

    if (_parse_options(argc, argv, options) < 0) {
        return -1;
    }

    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        if (OPTTYPE_POSITIONAL == options[i].opt_type)
        {
            if (!options[i].seen)
            {
                printf("missing required positional arguments\n");
                return -1;
            }
        }
    }

    return 0;
}
