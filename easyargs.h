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
}

#define ARGS_FLAG(short_flagstr, long_flagstr, value) { \
    .short_flag = short_flagstr, \
    .long_flag = long_flagstr, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_FLAG, \
    .data = value, \
}

#define ARGS_END_OF_OPTIONS { \
    .short_flag = NULL, \
    .long_flag = NULL, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_NONE, \
    .data = NULL, \
}


typedef int (*arg_decoder_t)(char*, void*);

static int _decode_int(char *input, void *output);
static int _decode_uint(char *input, void *output);
static int _decode_float(char *input, void *output);
static int _decode_string(char *input, void *output);
static int _decode_hex(char *input, void *output);

typedef struct {
    arg_decoder_t decode;
    const char *name;
} decode_params_t;

typedef enum {
    ARGTYPE_INT = 0,
    ARGTYPE_UINT,
    ARGTYPE_FLOAT,
    ARGTYPE_STRING,
    ARGTYPE_HEX,
    ARGTYPE_NONE
} argtype_e;

static decode_params_t _decoders[] = {
    { .decode = _decode_int, .name = "integer" },            // ARGTYPE_INT
    { .decode = _decode_uint, .name = "unsigned integer" },  // ARGTYPE_UINT
    { .decode = _decode_float, .name = "floating point" },   // ARGTYPE_FLOAT
    { .decode = _decode_string, .name = "string" },          // ARGTYPE_STRING
    { .decode = _decode_hex, .name = "hexadecimal" }         // ARGTYPE_HEX
};

typedef enum {
    OPTTYPE_FLAG,
    OPTTYPE_OPTION,
    OPTTYPE_NONE
} opttype_e;

typedef struct {
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

static int _decode_float(char *input, void *output)
{
    char *endptr = NULL;
    float floatval = strtod(input, &endptr);
    if (!endptr || *endptr)
    {
        return -1;
    }

    float *float_output = (float *)output;
    *float_output = (float)floatval;
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
    long intval = strtol(input, &endptr, 16);
    if (!endptr || *endptr)
    {
        return -1;
    }

    int *int_output = (int *)output;
    *int_output = (int)intval;
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


/*
 * Find any arguments that do not follow an option flag requiring an argument,
 * and shift them to the end of the argument list
 */
static void shift_nonopt_args(int argc, char *argv[], args_option_t *options)
{
    for (int i = 1; i < (argc - 1); i++) {
        if (argv[i][0] != '-') {
            if (is_optarg(argv, i, options)) {
                continue;
            }

            char *temp = argv[i];

            for (int j = i + 1; j < argc; j++) {
                argv[j - 1] = argv[j];
            }

            argv[argc - 1] = temp;
        }
    }
}

static void init_options(args_option_t *options)
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

    init_options(options);
    shift_nonopt_args(argc, argv, options);

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

            decode_params_t *params = &_decoders[opt->arg_type];
            if (params->decode(argv[i + 1], opt->data) < 0)
            {
                printf("%s value required for option '%s'\n",
                       params->name, argv[i]);
                return -1;
            }
        }
    }

    return 0;
}
