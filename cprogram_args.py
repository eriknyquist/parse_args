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

class UnknownArgType(Exception):
    pass

class TooManyLongFlags(Exception):
    pass

class TooManyShortFlags(Exception):
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
    def __init__(self, argtype, opttype, short_arg, long_arg=None):
        if opttype == COptType.FLAG:
            self.argtype = "_flag"
        elif argtype not in arg_type_data:
            raise UnknownArgType("unknown argument type: %s" % argtype)
        else:
            self.argtype = argtype

        self.short_arg = short_arg
        self.long_arg = long_arg
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
        longarg = "NULL" if not self.long_arg else '"%s"' % self.long_arg

        if self.opttype == COptType.POSITIONAL:
            ret = ("ARGS_POSITIONAL_ARG(%s, &%s)" % (argdata.argtype_name,
                                                     self._varname))

        elif self.opttype == COptType.OPTION:
            ret = ("ARGS_OPTION(\"%s\", %s, %s, &%s)" % (self.short_arg,
                                                         longarg,
                                                         argdata.argtype_name,
                                                         self._varname))

        elif self.opttype == COptType.FLAG:
            ret = ("ARGS_FLAG(\"%s\", %s, &%s)" % (self.short_arg, longarg,
                                                   self._varname))

        else:
            raise ValueError("Unknown option type: %d" % self.opttype)

        return "    " + ret

def parse_args(args):
    i = 1
    opts = []
    flags = []
    pos = []

    def parse_arg(index):
        i = index
        shortflag = None
        longflag = None
        argtype = None

        if args[i].startswith('--'):
            longflag = args[i]
            i += 1
            if i == len(args):
                return i - index, shortflag, longflag, argtype

            if args[i].startswith('--'):
                raise TooManyLongFlags()

            if args[i][0] == '-':
                short_flag = args[i]
                i += 1
        else:
            shortflag = args[i]
            i += 1
            if i == len(args):
                return i - index, shortflag, longflag, argtype

            if args[i][0] == '-':
                if not args[i].startswith('--'):
                    raise TooManyShortFlags()

                longflag = args[i]
                i += 1

        if (i < len(args)) and args[i][0] != '-':
            argtype = args[i]
            i += 1

        return i - index, shortflag, longflag, argtype

    while i < len(args):
        if args[i][0] != '-':
            pos.append(COption(args[i], COptType.POSITIONAL, None, None))
            i += 1
            continue

        inc, shortflag, longflag, argtype = parse_arg(i)
        i += inc

        opttype = None
        if argtype:
            opts.append(COption(argtype, COptType.OPTION, shortflag, longflag))
        else:
            flags.append(COption(None, COptType.FLAG, shortflag, longflag))

    return opts, flags, pos

def main():
    opts, flags, pos = parse_args(sys.argv)
    print generate_c_code(opts, flags, pos)

if __name__ == "__main__":
    main()
