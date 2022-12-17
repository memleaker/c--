#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <cstdlib>

namespace c11 {

class Error {
public:
    static void Fatal(const std::string& str) 
    {
        std::cerr << "c--: fatal rror: " << str << std::endl;
        exit(1);
    }

    static void Warning(const std::string& str) 
    {
        std::cerr << "warning: " << str << std::endl;
    };
};

}

#endif