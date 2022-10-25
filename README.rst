Portable command-line argument parser for C programs
----------------------------------------------------

* Handles optional arguments with data (e.g. ``--file out.txt``)
* Converts optional argument data to desired data type
* Handles optional arguments without data, AKA flags (e.g. ``--verbose``)
* Handles positional arguments

Does argument *parsing* only-- no docs generation.

How to use
----------

Just add the file "parse_args.h" to your project and include it. The whole implementation
is inside this file.

Example main.c
---------------

.. code:: c

    #include <stdio.h>
    #include <stdlib.h>

    #include "parse_args.h"

    int int_value = 0;
    long long_value = 0;
    float float_value = 0.0;
    double double_value = 0.0;
    char *str_value = NULL;
    long hex_value = 0;
    unsigned uint_value = 0;
    unsigned long ulong_value = 0;
    int flag1;
    int flag2;
    char *positional_1 = NULL;
    int positional_2 = 0;

    args_option_t options[] = {
        ARGS_POSITIONAL_ARG(ARGTYPE_STRING, &positional_1),
        ARGS_POSITIONAL_ARG(ARGTYPE_INT, &positional_2),
        ARGS_OPTION("-i", "--int", ARGTYPE_INT, &int_value),
        ARGS_OPTION("-l", "--long", ARGTYPE_LONG, &long_value),
        ARGS_OPTION("-f", "--float", ARGTYPE_FLOAT, &float_value),
        ARGS_OPTION("-d", "--double", ARGTYPE_DOUBLE, &double_value),
        ARGS_OPTION("-u", "--unsigned", ARGTYPE_UINT, &uint_value),
        ARGS_OPTION("-g", "--unsigned-long", ARGTYPE_ULONG, &ulong_value),
        ARGS_OPTION("-x", "--hex", ARGTYPE_HEX, &hex_value),
        ARGS_OPTION("-s", "--string", ARGTYPE_STRING, &str_value),
        ARGS_FLAG("-q", NULL, &flag1),
        ARGS_FLAG("-w", "--whatever", &flag2),
        ARGS_END_OF_OPTIONS
    };

    int main(int argc, char *argv[])
    {
        // Parse all arguments and convert to target types
        if (parse_arguments(argc, argv, options) < 0)
        {
            printf("Error parsing arguments:\n%s\n", parse_arguments_error_string());
            return -1;
        }

        // Print all values received by command-line arguments
        printf("Flags:\nq=%d, w=%d\n\n", flag1, flag2);

        printf("Options:\ni=%d, l=%ld, g=%lu, f=%.2f, d=%.2f, u=%u, x=%ld, s=%s\n\n",
               int_value, long_value, ulong_value, float_value, double_value,
               uint_value, hex_value, str_value);

        printf("Positional arguments:\n1=%s, 2=%d\n\n", positional_1, positional_2);

        return 0;
    }
