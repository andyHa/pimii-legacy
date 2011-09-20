#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include "vm/env.h"

class Interceptor
{
public:
    Interceptor() {}

    virtual void println(const QString& std) {}

    virtual void reportStatus(const EngineStatus& status) {}
};

#endif // INTERCEPTOR_H
