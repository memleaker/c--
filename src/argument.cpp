#include <cstring>
#include <cstdlib>
#include <iostream>

#include "log.h"
#include "argument.h"

namespace c89 {

CcArg::CcArg()
{
    // add default include path
    includes.push_back("/usr/include");
    includes.push_back("/usr/local/include");
    includes.push_back("/usr/include/x86_64-linux-gnu");
}

void CcArg::ParseArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c")) {
            opt_c = true;
            continue;
        }

        if (!strcmp(argv[i], "-E")) {
            opt_E = true;
            continue;
        }

        if (!strcmp(argv[i], "-S")) {
            opt_S = true;
            continue;
        }

        if (!strcmp(argv[i], "-fPIC")) {
            opt_fpic = true;
            continue;
        }

        if (!strcmp(argv[i], "-static")) {
            opt_static = true;
            continue;
        }

        if (!strcmp(argv[i], "-shared")) {
            opt_shared = true;
            continue;
        }

        if (!strcmp(argv[i], "-x")) {
            if (i+1 < argc) {
                input_type = ParseOptx(argv[++i]);
                continue;
            }

            Error::Fatal("option '-x' need argument");
        }

        if (!strncmp(argv[i], "-x", 2)) {
            input_type = ParseOptx(argv[i]+2);
            continue;
        }

        if (!strcmp(argv[i], "-o")) {
            if (i+1 < argc) {
                output = argv[++i];
                continue;
            }

            Error::Fatal("option '-o' need argument");
        }

        if (!strncmp(argv[i], "-o", 2)) {
            opt_o = true;
            output = argv[i]+2;
            continue;
        }

        if (!strcmp(argv[i], "-l")) {
            if (i+1 < argc) {
                ldargs.emplace_back(std::string("-l ")+ argv[++i]);
                continue;
            }
            Error::Fatal("option '-l' need argument");
        }

        if (!strncmp(argv[i], "-l", 2)) {
            ldargs.emplace_back(argv[i]);
            continue;
        }

        if (!strncmp(argv[i], "-Wl,", 4)) {
            ParseOptWl(argv[i]+4, ldargs);
            continue;
        }

        if (!strcmp(argv[i], "-I")) {
            if (i+1 < argc) {
                includes.emplace_back(argv[++i]);
                continue;
            }
            Error::Fatal("option '-I' need argument");
        }

        if (!strncmp(argv[i], "-I", 2)) {
            includes.emplace_back(argv[i]+2);
            continue;
        }

        if (!strcmp(argv[i], "-L")) {
            if (i+1 < argc) {
                ldargs.emplace_back(std::string("-L ")+argv[++i]);
                continue;
            }
            Error::Fatal("option '-L' need argument");
        }

        if (!strncmp(argv[i], "-L", 2)) {
            ldargs.emplace_back(argv[i]);
            continue;
        }

        input.emplace_back(argv[i]);
    }

    // check arguments
    if (input.size() == 0) {
        Error::Fatal("no input files");
    }

    // "c-- x.c xx.c -c -o xxx"
    if (input.size() > 1 && (opt_c || opt_E || opt_S) && opt_o) {
        Error::Fatal("can not spiecify output for multiple files with"
                        " '-c', '-E', '-S' option");
    }
}

Languages CcArg::ParseOptx(const std::string& arg)
{
    if (arg == "c") {
        return C;
    } else if (arg == "assembler") {
        return ASM;
    } else {
        return NONE;
    }
}

void CcArg::ParseOptWl(const std::string& str, std::vector<std::string>& ldargs)
{
    size_t last{0}, pos{0};

    while ((str.npos != (pos = str.find(',', last))) || (last != str.size()))
    {
        // not found, push last token
        if (pos == str.npos) {
            ldargs.emplace_back(str.substr(last, str.size() - last));
            last = str.size();
        } else {
            ldargs.emplace_back(str.substr(last, pos-last));
            last = pos+1;
        }
    }
}

}