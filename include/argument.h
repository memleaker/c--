#ifndef __ARGUMENT_H__
#define __ARGUMENT_H__

#include <string>
#include <vector>

namespace c11 {

enum Languages
{
    C,
    ASM,
    NONE,
};

class CcArg 
{
public:
    CcArg();
    void ParseArgs(int argc, char **argv);
    Languages ParseOptx(const std::string& arg);
    void ParseOptWl(const std::string& str, std::vector<std::string>& ldargs);

public:
    // Option without parameters
    bool opt_c = false; // -c
    bool opt_E = false; // -E
    bool opt_S = false; // -S
    bool opt_fpic = false; // -fPIC
    bool opt_static = false; // -static
    bool opt_shared = false; // -shared

    // Warning Options
    bool opt_Wall = false; // -Wall

    // Debug options -g ?

    // Option with parameters
    bool opt_o = false;
    std::string output = "a.out";  // -o
    Languages input_type = NONE; // -x
    std::vector<std::string> input;
    std::vector<std::string> ldargs; // -l -Wl -L
    std::vector<std::string> includes; // -I
};
}

#endif