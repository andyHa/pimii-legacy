#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include <iostream>

#include "vm/env.h"

class Interceptor
{
public:
    Interceptor() {}

    virtual void println(String std) {
        std::wcout << std << std::endl;
    }

    virtual void reportStatus(EngineStatus status) {}
};

#endif // INTERCEPTOR_H
