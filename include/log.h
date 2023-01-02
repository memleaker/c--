#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <string>
#include <cstdlib>

namespace c89 {

class Error {
public:
    static void Fatal(const std::string& str) 
    {
        std::cerr << "\e[1;31mfatal error: \e[0m" << str << std::endl;
        exit(1);
    }

    static void Warning(const std::string& str) 
    {
        std::cerr << "warning: " << str << std::endl;
    };
};

}

#endif