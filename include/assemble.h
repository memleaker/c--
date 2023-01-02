#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#include "files.h"
#include "argument.h"

namespace c89 {

class Assembler
{
public:
    static void Assemble(const CcArg& arg, Files& files);
};

}
#endif