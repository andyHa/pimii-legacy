#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

#include "vm/env.h"

class Logger
{
public:
    Logger() {}

    virtual void println(String std) {
        std::wcout << std << std::endl;
    }
};

#endif // LOGGER_H
