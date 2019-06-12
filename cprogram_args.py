import sys

c_main_text="""
int main(int argc, char *argv[])
{
    if (parse_arguments(argc, argv, options) < 0)
    {
        printf("Error parsing arguments\\n");
        return -1;
    }

    // Put stuff here

    return 0;
}
"""

class ArgParseError(Exception):
    pass

class CArgData(object):
    def __init__(self, argtype_name, ctype_name, default_value):
        self.argtype_name = argtype_name
        self.ctype_name = ctype_name
        self.default_value = default_value
        self.name_count = 0

arg_type_data = {
    "int": CArgData("ARGTYPE_INT", "int", "0"),
    "long": CArgData("ARGTYPE_LONG", "long", "0"),
    "uint": CArgData("ARGTYPE_UINT", "unsigned", "0u"),
    "ulong": CArgData("ARGTYPE_ULONG", "unsigned long", "0u"),
    "float": CArgData("ARGTYPE_FLOAT", "float", "0.0f"),
    "double": CArgData("ARGTYPE_DOUBLE", "double", "0.0"),
    "string": CArgData("ARGTYPE_STRING", "char", "NULL"),
    "hex": CArgData("ARGTYPE_HEX", "long", "0"),
    "_flag": CArgData("ARGTYPE_INT", "int", "0")
}

class COptType(object):
    OPTION = 0
    POSITIONAL = 1
    FLAG = 3

def generate_c_code(opts, flags, pos):
    ret = ("#include <stdio.h>\n"
           "#include <stdlib.h>\n"
           "#include \"parse_args.h\"\n\n")

    options = opts + flags + pos
    ret += "\n".join([x.data_declaration() for x in options]) + "\n\n"

    ret += "args_option_t options[] =\n{\n"

    last = "    ARGS_END_OF_OPTIONS"
    ret += ",\n".join([x.option_definition() for x in options] + [last])
    ret += "\n};\n"

    return ret + c_main_text

class COption(object):
    def __init__(self, argtype, opttype, short_flag, long_flag=None):
        if opttype == COptType.FLAG:
            self.argtype = "_flag"
        elif argtype not in arg_type_data:
            raise ArgParseError("unknown argument type: %s" % argtype)
        else:
            self.argtype = argtype

        self.short_flag = short_flag
        self.long_flag = long_flag
        self.opttype = opttype
        self._varname = None

    def data_declaration(self):
        vartype = ""
        argdata = arg_type_data[self.argtype]

        if self.opttype == COptType.FLAG:
            vartype = "flag"
        elif self.argtype:
            vartype = argdata.ctype_name
        else:
            return ""

        varname = "%s_val" % vartype
        self._varname = varname
        if (argdata.name_count > 0):
            varname += str(argdata.name_count)
            self._varname = varname
        if self.argtype == "string":
            self._varname = varname
            varname = "*" + varname

        argdata.name_count += 1
        return "%s %s = %s;" % (argdata.ctype_name, varname,
                                argdata.default_value)

    def option_definition(self):
        if not self._varname:
            raise RuntimeError("Must call data_declaration() first")

        if (not self.argtype) or (self.opttype is None):
            return ""

        ret = ""
        argdata = arg_type_data[self.argtype]
        longarg = "NULL" if not self.long_flag else '"%s"' % self.long_flag

        if self.opttype == COptType.POSITIONAL:
            ret = ("ARGS_POSITIONAL_ARG(%s, &%s)" % (argdata.argtype_name,
                                                     self._varname))

        elif self.opttype == COptType.OPTION:
            ret = ("ARGS_OPTION(\"%s\", %s, %s, &%s)" % (self.short_flag,
                                                         longarg,
                                                         argdata.argtype_name,
                                                         self._varname))

        elif self.opttype == COptType.FLAG:
            ret = ("ARGS_FLAG(\"%s\", %s, &%s)" % (self.short_flag, longarg,
                                                   self._varname))

        else:
            raise ValueError("Unknown option type: %d" % self.opttype)

        return "    " + ret

def count_items(items, testfunc):
    return sum(testfunc(x) for x in items)

def verify_option_definitions(opts, flags):
    options = opts + flags

    for option in options:
        if option.long_flag is not None:
            check = lambda x: ((x.long_flag is not None) and
                               (x.long_flag == option.long_flag))

            if count_items(options, check) > 1:
                raise ArgParseError("flag in use for multiple options: %s"
                                    % option.long_flag)

        if option.short_flag is not None:
            check = lambda x: ((x.short_flag is not None) and
                              (x.short_flag == option.short_flag))

            if count_items(options, check) > 1:
                raise ArgParseError("flag in use for multiple options: %s"
                                    % option.short_flag)

        if option.opttype in [COptType.FLAG, COptType.OPTION]:
            if option.short_flag is None:
                raise ArgParseError("long flag without short flag: %s"
                                    % option.long_flag)

def parse_arg(arg):
    shortflag = None
    longflag = None
    argtype = None
    fields = arg.split(',')

    for field in fields:
        if (field[0] == '-') and (not field.startswith('--')):
            if shortflag is not None:
                raise ArgParseError("only one short flag allowed: %s" % arg)

            shortflag = field

        elif field.startswith('--'):
            if longflag is not None:
                raise ArgParseError("only one long flag allowed: %s" % arg)

            longflag = field

        else:
            if argtype is not None:
                raise ArgParseError("only one data type allowed: %s" % arg)

            argtype = field

    return shortflag, longflag, argtype

def parse_args(args):
    opts = []
    flags = []
    pos = []

    for arg in args[1:]:
        shortflag, longflag, argtype = parse_arg(arg)

        opttype = None
        if (shortflag is None) and (longflag is None):
            pos.append(COption(argtype, COptType.POSITIONAL, None, None))
        elif argtype is None:
            flags.append(COption(None, COptType.FLAG, shortflag, longflag))
        else:
            opts.append(COption(argtype, COptType.OPTION, shortflag, longflag))

    verify_option_definitions(opts, flags)
    return opts, flags, pos

def main():
    try:
        opts, flags, pos = parse_args(sys.argv)
    except ArgParseError as e:
        print("\nError: %s\n" % e)
        return -1

    print generate_c_code(opts, flags, pos)

    return 0

if __name__ == "__main__":
    sys.exit(main())
