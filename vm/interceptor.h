#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include "vm/env.h"

#include <QtWebKit>

class Interceptor
{
public:
    Interceptor() {}

    virtual void println(const QString& std) = 0;

    virtual void reportStatus(const EngineStatus& status) = 0;

    virtual QWebFrame* getFrame() = 0;
};

#endif // INTERCEPTOR_H
