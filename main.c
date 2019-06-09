#include <stdio.h>
#include <stdlib.h>

#include "parse_args.h"

int intval = 0;
long longval = 0;
float floatval = 0.0;
double doubleval = 0.0;
char *strval = NULL;
long hexval = 0;
unsigned uintval = 0;
unsigned long ulongval = 0;

int qflag = 10;
int wflag = 18;

float p1 = 0.0;
float p2 = 0.0;

args_option_t options[] = {
    ARGS_POSITIONAL_ARG(ARGTYPE_FLOAT, &p1),
    ARGS_POSITIONAL_ARG(ARGTYPE_FLOAT, &p2),
    ARGS_OPTION("-i", "--int", ARGTYPE_INT, &intval),
    ARGS_OPTION("-l", "--long", ARGTYPE_LONG, &longval),
    ARGS_OPTION("-f", "--float", ARGTYPE_FLOAT, &floatval),
    ARGS_OPTION("-d", "--double", ARGTYPE_DOUBLE, &doubleval),
    ARGS_OPTION("-u", "--unsigned", ARGTYPE_UINT, &uintval),
    ARGS_OPTION("-g", "--ulong", ARGTYPE_ULONG, &ulongval),
    ARGS_OPTION("-x", "--hex", ARGTYPE_HEX, &hexval),
    ARGS_OPTION("-s", "--string", ARGTYPE_STRING, &strval),
    ARGS_FLAG("-q", NULL, &qflag),
    ARGS_FLAG("-w", NULL, &wflag),
    ARGS_END_OF_OPTIONS
};

int main(int argc, char *argv[])
{
    if (parse_arguments(argc, argv, options) < 0)
    {
        printf("Error parsing arguments\n");
        return -1;
    }

    printf("q=%d, w=%d\n", qflag, wflag);
    printf("i=%d, l=%ld, ul=%lu, f=%.2f, d=%.2f, u=%u, x=%ld, s=%s\n",
           intval, longval, ulongval, floatval, doubleval, uintval, hexval, strval);
    printf("1=%.2f, 2=%.2f\n", p1, p2);
    return 0;
}
