Portable command-line argument parser for C programs
----------------------------------------------------

* Handles optional arguments with data (e.g. ``--file out.txt``)
* Converts optional argument data to desired data type
* Handles optional arguments without data, AKA flags (e.g. ``--verbose``)
* Handles positional arguments

Does argument *parsing* only-- no docs generation.

Code example
------------

.. code:: c

   #include <stdio.h>
   #include <stdlib.h>

   #include "parse_args.h"

   // Data for positional arguments
   char *positional_1 = NULL;
   int positional_2 = 0;

   // Data for optional arguments
   long long_value = 0;
   long hex_value = 0;
   double double_value = 0.0;
   char *str_value = NULL;

   // Data for flags
   int flag1;
   int flag2;

   // Option definitions
   args_option_t options[] = {
       ARGS_POSITIONAL_ARG(ARGTYPE_STRING, &positional_1),
       ARGS_POSITIONAL_ARG(ARGTYPE_INT, &positional_2),
       ARGS_OPTION("-l", "--long", ARGTYPE_LONG, &long_value),
       ARGS_OPTION("-x", "--hex", ARGTYPE_HEX, &hex_value),
       ARGS_OPTION("-d", "--double", ARGTYPE_DOUBLE, &double_value),
       ARGS_OPTION("-s", "--string", ARGTYPE_STRING, &str_value),
       ARGS_FLAG("-q", NULL, &flag1),
       ARGS_FLAG("-w", NULL, &flag2),
       ARGS_END_OF_OPTIONS
   };

   int main(int argc, char *argv[])
   {
       if (parse_arguments(argc, argv, options) < 0)
       {
           printf("Error parsing arguments\n");
           return -1;
       }

       printf("Flags:\n");
       printf("q=%d, w=%d\n\n", flag1, flag2);

       printf("Optional arguments:\n");
       printf("l=%ld, x=%ld, d=%.4f, s=%s\n\n", long_value, hex_value, double_value, str_value);

       printf("Positional arguments:\n");
       printf("1=%s, 2=%d\n\n", positional_1, positional_2);

       return 0;
   }
