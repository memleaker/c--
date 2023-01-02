#ifndef __FILE_H__
#define __FILE_H__

#include <vector>
#include <string>

#include "argument.h"

namespace c89 {

enum FileType 
{
    C_FILE,
    ASM_FILE,
    OBJ_FILE,
    AR_FILE,
    DSO_FILE,
    PREC_FILE,
    NONE_FILE,
};

class Files
{
public:
    static std::string DirName(const std::string& name);
    static std::string BaseName(const std::string& name);
    static void RenameFile(const std::string& oldpath, const std::string& newpath);
    static FileType GetFileType(const std::string& name);
    static std::string ConvertTo(const std::string& name, FileType type);

    void DispatchFiles(const CcArg& ccarg);

public:
    std::vector<std::string> cfiles;    // input c file (.c)
    std::vector<std::string> tmpcfiles; // generated c file (xxx.i)
    std::vector<std::string> asmfiles;     // input asm file
    std::vector<std::string> tmpasmfiles;  // generated asm file
    std::vector<std::string> objfiles;     // input obj file (.o, .so, .a)
    std::vector<std::string> tmpobjfiles;  // generated obj file (.o)
};

}

#endif