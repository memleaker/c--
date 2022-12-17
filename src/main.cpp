#include <vector>
#include <string>

#include "files.h"
#include "linker.h"
#include "argument.h"
#include "assemble.h"

int main(int argc, char **argv)
{
    c11::Files files;
    // cleanup at exit

    // parse arguments
    c11::CcArg ccarg;
    ccarg.ParseArgs(argc, argv);

    // dispatch input files
    files.DispatchFiles(ccarg);

    // preprocess 生成 .i 即 tmpcfiles, 临时c文件

    // tokenize 生成token

    // codegen 生成 .s

    // assemble 处理asm 文件(用户输入的或生成的)
    // 通过files结构。每个函数只处理自己的files，并生成files
    c11::Assembler::Assemble(ccarg, files);

    // link 处理库以及二进制文件 (用户输入的和生成的)
    c11::Linker::Link(ccarg, files);

    return 0;
}