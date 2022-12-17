#include <map>
#include <cstdio>

#include "files.h"

namespace c11 {

static std::map<FileType, const char *> filemap {
    {C_FILE,   ".c"},
    {AR_FILE,  ".a"},
    {ASM_FILE, ".s"},
    {OBJ_FILE, ".o"},
    {DSO_FILE, ".so"},
    {PREC_FILE,".i"},
};

std::string Files::DirName(const std::string& name)
{
    auto pos = name.find_last_of('/');
    if (pos == name.size()) {
        if (pos == 0) {
            return name;
        }

        pos = name.find_last_of('/', pos-1);
    }

    if (pos != name.npos) {
        return name.substr(0, pos);
    }

    return "."; 
}

std::string Files::BaseName(const std::string& name)
{
    auto pos = name.find_last_of('/');
    if (pos == name.size()-1) {
        if (pos == 0) {
            return name;
        }
        pos = name.find_last_of('/', pos-1);
    }

    if (pos != name.npos) {
        return name.substr(pos+1);
    }

    return name;
}

void Files::RenameFile(const std::string& oldpath, const std::string& newpath)
{
    rename(oldpath.c_str(), newpath.c_str());
}

FileType Files::GetFileType(const std::string& name)
{
    for (auto& pair : filemap)
    {
        if (name.substr(name.size() - 2) == pair.second) {
            return pair.first;
        }
    }

    return NONE_FILE;
}

std::string Files::ConvertTo(const std::string& name, FileType type)
{
    auto pos = name.find_last_of('.');
    if (name.npos == pos || NONE_FILE == GetFileType(name)) {
        return name + filemap[type];
    }

    return std::string(name).replace(pos, name.size() - pos, filemap[type]);
}

void Files::DispatchFiles(const CcArg& ccarg)
{
    // dispatch input files
    for (auto & s : ccarg.input)
    {
        if (ccarg.input_type == Languages::C) {
            cfiles.emplace_back(s);
        } else if (ccarg.input_type == Languages::ASM) {
            asmfiles.emplace_back(s);
        } else {
            switch (Files::GetFileType(s))
            {
            case FileType::C_FILE:
            case FileType::PREC_FILE:
            case FileType::NONE_FILE:
                cfiles.emplace_back(s);
                break;
            case FileType::ASM_FILE:
                asmfiles.emplace_back(s);
                break;
            case FileType::AR_FILE:
            case FileType::DSO_FILE:
            case FileType::OBJ_FILE:
                objfiles.emplace_back(s);
                break;
            }
        }
    }
}

}