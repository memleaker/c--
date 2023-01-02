#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "assemble.h"

namespace c89 {

void Assembler::Assemble(const CcArg& arg, Files& files)
{
    std::vector<const char *> cmds {"as", "-c", nullptr, "-o", nullptr, nullptr};

    for (const auto &vec : {files.asmfiles, files.tmpasmfiles})
    {
        for (const auto &f : vec)
        {
            cmds[2] = f.c_str();
            cmds[4] = Files::BaseName(Files::ConvertTo(f, OBJ_FILE)).c_str();

            // run as command
            if (fork() == 0) {
                execvp(cmds[0], (char *const*)cmds.data());
                exit(1);
            }

            wait(nullptr);

            files.tmpobjfiles.emplace_back(cmds[4]);
        }
    }

    // c-- xxx -c
    // c-- xxx -c -o output
    if (arg.opt_c) {
        if (arg.opt_o) {
            Files::RenameFile(files.tmpobjfiles[0], arg.output);
        }

        files.tmpobjfiles.clear();
        exit(0);
    }
}

} 