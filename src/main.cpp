#include <vector>
#include <string>

#include "files.h"
#include "linker.h"
#include "argument.h"
#include "assemble.h"
#include "tokenize.h"

int main(int argc, char **argv)
{
    c89::Files files;
    // cleanup at exit

    // parse arguments
    c89::CcArg ccarg;
    ccarg.ParseArgs(argc, argv);

    // dispatch input files
    files.DispatchFiles(ccarg);

    // preprocess 生成 .i 即 tmpcfiles, 临时c文件
    // 先不写预处理，先写 tokenize和codegen, 最后写预处理

    // lexical
    c89::Tokenizer toks;
    toks.TokenizeFiles(ccarg, files);

    // parse

    // codegen

    c89::Assembler::Assemble(ccarg, files);

    c89::Linker::Link(ccarg, files);

    return 0;
}