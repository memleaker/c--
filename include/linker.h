#ifndef __LINKER_H__
#define __LINKER_H__

#include <string>

#include "files.h"
#include "argument.h"

namespace c11 {

class Linker
{
public:
    static void Link(const CcArg& arg, Files& files);

    // crt1.o, crti.o, crtn.o
    static std::string FindGlibCrt();

    // crtbegin.o  crtend.o
    static std::string FindGccLibCrt();

    static unsigned int GetGccVersion();
};

}

#endif