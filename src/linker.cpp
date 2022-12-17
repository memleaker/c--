#include <vector>
#include <string>

#include <glob.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "log.h"
#include "linker.h"

namespace c11 {

void Linker::Link(const CcArg& arg, Files& files)
{
    std::vector<std::string> cmds;
    std::vector<const char*> ccmds;

    if ((files.objfiles.size() == 0) &&
        (files.tmpobjfiles.size() == 0)) {
        return;
    }

    cmds.emplace_back("ld");
    cmds.emplace_back("-o");
    cmds.emplace_back(arg.output);
    cmds.emplace_back("-m");
    cmds.emplace_back("elf_x86_64");

    auto glibctr = FindGlibCrt();
    auto gcclibctr = FindGccLibCrt();

    if (arg.opt_shared) {
        cmds.emplace_back(glibctr + "/crti.o");
        cmds.emplace_back(gcclibctr + "/crtbeginS.o");
    } else {
        cmds.emplace_back(glibctr + "/crti.o");
        cmds.emplace_back(glibctr + "/crt1.o");
        cmds.emplace_back(gcclibctr + "/crtbegin.o");
    }

    cmds.emplace_back("-L /usr/lib");
    cmds.emplace_back("-L /usr/lib64");
    cmds.emplace_back("-L" + glibctr);
    cmds.emplace_back("-L" + gcclibctr);

    if (!arg.opt_static) {
        cmds.emplace_back("-dynamic-linker");
        cmds.emplace_back("/lib64/ld-linux-x86-64.so.2");
    }

    cmds.insert(cmds.end(), files.objfiles.begin(), files.objfiles.end());
    cmds.insert(cmds.end(), files.tmpobjfiles.begin(), files.tmpobjfiles.end());
    cmds.insert(cmds.end(), arg.ldargs.begin(), arg.ldargs.end());

    if (arg.opt_static) {
        cmds.emplace_back("--start-group");
        cmds.emplace_back("-lgcc");
        cmds.emplace_back("-lgcc_eh");
        cmds.emplace_back("-lc");
        cmds.emplace_back("--end-group");
    } else {
        cmds.emplace_back("-lc");
        cmds.emplace_back("-lgcc");
        cmds.emplace_back("--as-needed");
        cmds.emplace_back("-lgcc_s");
        cmds.emplace_back("--no-as-needed");
    }

    if (arg.opt_shared) {
        cmds.emplace_back(gcclibctr + "/crtendS.o");
    } else {
        cmds.emplace_back(gcclibctr + "/crtend.o");
    }
    cmds.emplace_back(glibctr + "/crtn.o");

    // run ld command
    for (auto& s:cmds)
        ccmds.emplace_back(s.c_str());
    ccmds.emplace_back(nullptr);

    if (fork() == 0) {
        execvp(ccmds[0], (char *const*)ccmds.data());
        exit(1);
    }

    wait(nullptr);
}

std::string Linker::FindGlibCrt()
{
    struct stat st;

    if (!stat("/usr/lib/x86_64-linux-gnu/crti.o", &st)) {
        return "/usr/lib/x86_64-linux-gnu";
    }

    if (!stat("/usr/lib64/crti.o", &st)) {
        return "/usr/lib64";
    }

    Error::Fatal("Glibc Runtime Library not found");
    return "";
}

std::string Linker::FindGccLibCrt()
{
    unsigned int v;
    glob_t buf;
    std::string version, path;

    if ((v = GetGccVersion())) {
        version = std::to_string(v);
    } else {
        version = "*";
    }

    std::vector<std::string> paths {
        "/usr/lib/gcc/x86_64*/" + version + "/crtbegin.o",
        "/usr/lib64/gcc/x86_64*/" + version + "/crtbegin.o",
    };

    for (auto& s : paths)
    {
        glob(s.c_str(), 0, NULL, &buf);
        if (buf.gl_pathc > 0) {
            path = buf.gl_pathv[buf.gl_pathc - 1];
            globfree(&buf);
            return Files::DirName(path);
        }
    }

    Error::Fatal("Gcc Runtime Library not found");
    return "";
}

unsigned int Linker::GetGccVersion()
{
    int n;
    FILE *fp;
    unsigned int version;

    fp = popen("gcc -dumpversion", "r");
    if (!fp) {
        return 0;
    }

    n = fscanf(fp, "%u", &version);
    if (n != 1) {
        return 0;
    }

    fclose(fp);

    return version;
}

}